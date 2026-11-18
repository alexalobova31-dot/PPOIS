#include "Organization.h"
#include "Product.h"
#include "SalesDepartment.h"
#include "Warehouse.h"
#include "Company.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    using Catalog = std::vector<std::shared_ptr<const Product>>;

    void clearInput()
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    template <typename T>
    T readValue(const std::string& prompt)
    {
        std::cout << prompt;
        T value;
        std::cin >> value;
        if (!std::cin)
        {
            clearInput();
            throw std::invalid_argument("Invalid input");
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }

    std::string formatMoney(double amount)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << amount;
        return stream.str();
    }

    std::string readLine(const std::string& prompt)
    {
        std::cout << prompt;
        std::string value;
        std::getline(std::cin, value);
        return value;
    }

    template <typename T>
    const T& selectFrom(const std::vector<T>& items, const std::string& prompt)
    {
        const int index = readValue<int>(prompt);
        if (index < 1 || index > static_cast<int>(items.size()))
            throw std::out_of_range("Index out of range");
        return items[index - 1];
    }

    Catalog createCatalog()
    {
        return {
            std::make_shared<OverTheCounterDrug>("OTC-001", "Парацетамол", "Борисовский ЗМП", 1.20, "парацетамол", 500.0, 0.05),
            std::make_shared<OverTheCounterDrug>("OTC-002", "Ибупрофен", "Белмедпрепараты", 2.40, "ибупрофен", 200.0, 0.0),
            std::make_shared<PrescriptionDrug>("RX-001", "Амоксициллин", "Лекфарм", 6.80, "амоксициллин", 500.0),
            std::make_shared<PrescriptionDrug>("RX-002", "Бисопролол", "Фармтехнология", 4.10, "бисопролол", 5.0),
            std::make_shared<ColdChainProduct>("CC-001", "Инсулин Протамин", "Белмедпрепараты", 18.50, "инсулин", 3.5, 2.0, 8.0),
            std::make_shared<Vaccine>("VAC-001", "Гриппол плюс", "Петровакс", 12.00, "антиген вируса гриппа", 0.5, 2.0, 8.0, 1),
        };
    }

    void printMenu()
    {
        std::cout << "\n===== КОМПАНИЯ ПО СБЫТУ ЛЕКАРСТВЕННЫХ ПРЕПАРАТОВ =====\n";
        std::cout << "1. Каталог товаров\n";
        std::cout << "2. Контрагенты\n";
        std::cout << "3. Остатки на складах\n";
        std::cout << "4. Закупить товар у поставщика\n";
        std::cout << "5. Продать товар аптеке\n";
        std::cout << "6. Изменить температуру холодильного склада\n";
        std::cout << "7. Журнал операций и баланс\n";
        std::cout << "8. Демонстрация наследования\n";
        std::cout << "0. Выход\n";
        std::cout << "Выберите пункт: ";
    }

    void printCatalog(const Catalog& catalog)
    {
        for (std::size_t i = 0; i < catalog.size(); i++)
            std::cout << i + 1 << ". " << *catalog[i] << "\n";
    }

    template <typename T>
    void printOrganizations(const std::vector<T>& organizations)
    {
        for (std::size_t i = 0; i < organizations.size(); i++)
            std::cout << "  " << i + 1 << ". " << organizations[i] << "\n";
    }

    void printJournal(const SalesDepartment& sales)
    {
        for (const SalesDepartment::Entry& entry : sales.entries())
            std::cout << std::setw(10) << formatMoney(entry.amount) << "  " << entry.description << "\n";
        std::cout << "Баланс: " << formatMoney(sales.balance()) << "\n";
    }

    void purchase(SalesDepartment& sales, const Catalog& catalog, const std::vector<Supplier>& suppliers)
    {
        printCatalog(catalog);
        const std::shared_ptr<const Product>& product = selectFrom(catalog, "Номер товара: ");
        printOrganizations(suppliers);
        const Supplier& supplier = selectFrom(suppliers, "Номер поставщика: ");
        const int quantity = readValue<int>("Количество: ");

        const Warehouse& warehouse = sales.purchase(supplier, product, quantity);
        std::cout << "Партия принята на склад \"" << warehouse.getName() << "\"\n";
    }

    void sell(SalesDepartment& sales, const std::vector<PharmacyClient>& clients)
    {
        printOrganizations(clients);
        const PharmacyClient& client = selectFrom(clients, "Номер аптеки: ");
        const std::string sku = readLine("Артикул: ");
        const int quantity = readValue<int>("Количество: ");
        const std::string answer = readLine("Есть рецепт? (y/n): ");

        Prescription prescription;
        const Prescription* prescriptionPtr = nullptr;
        if (answer == "y" || answer == "Y")
        {
            prescription.number = readLine("Номер рецепта: ");
            prescription.activeSubstance = readLine("Действующее вещество: ");
            prescriptionPtr = &prescription;
        }

        const double amount = sales.sell(client, sku, quantity, prescriptionPtr);
        std::cout << "Продано на сумму " << formatMoney(amount) << "\n";
    }

    void changeColdTemperature(ColdStorageWarehouse& warehouse)
    {
        const double temperature = readValue<double>("Новая температура, °C: ");
        const std::vector<std::string> writtenOff = warehouse.changeTemperature(temperature);
        if (writtenOff.empty())
        {
            std::cout << "Условия хранения не нарушены.\n";
            return;
        }
        std::cout << "Списано из-за нарушения температурного режима:\n";
        for (const std::string& name : writtenOff) std::cout << "  " << name << "\n";
    }

    void demonstrateInheritance()
    {
        const Vaccine vaccine("VAC-DEMO", "Демо-вакцина", "Петровакс", 10.0, "антиген", 0.5, 2.0, 8.0, 10);

        std::cout << "Vaccine: PrescriptionDrug + ColdChainProduct, общий виртуальный базовый Medicine.\n";
        std::cout << vaccine << "\n";

        const Product& asProduct = vaccine;
        const Medicine& viaPrescription = static_cast<const PrescriptionDrug&>(vaccine);
        const Medicine& viaColdChain = static_cast<const ColdChainProduct&>(vaccine);
        std::cout << "Один подобъект Medicine: " << (&viaPrescription == &viaColdChain ? "да" : "нет") << "\n";
        std::cout << "Виртуальный вызов через Product&: " << asProduct.category() << ", цена " << formatMoney(asProduct.price()) << "\n";

        const Prescription valid{"RX-777", "антиген"};
        const Prescription wrong{"RX-778", "ибупрофен"};
        std::cout << "using PrescriptionDrug::isAcceptable(рецепт на антиген): " << (vaccine.isAcceptable(valid) ? "да" : "нет") << "\n";
        std::cout << "using PrescriptionDrug::isAcceptable(рецепт на ибупрофен): " << (vaccine.isAcceptable(wrong) ? "да" : "нет") << "\n";
        std::cout << "using ColdChainProduct::isAcceptable(5 °C): " << (vaccine.isAcceptable(5.0) ? "да" : "нет") << "\n";
        std::cout << "using ColdChainProduct::isAcceptable(20 °C): " << (vaccine.isAcceptable(20.0) ? "да" : "нет") << "\n";
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        if (argc == 2 && std::string(argv[1]) == "--example")
        {
            try
            {
                runCompanyExample(std::cout);
                return 0;
            }
            catch (const std::exception& error)
            {
                std::cerr << "Ошибка: " << error.what() << '\n';
                return 1;
            }
        }
        std::cerr << "Использование: pharma [--example]\n";
        return 1;
    }

    const Catalog catalog = createCatalog();
    const std::vector<Supplier> suppliers{
        Supplier("Белмедпрепараты", "100125012", "Беларусь"),
        Supplier("Петровакс Фарм", "190554321", "Россия"),
    };
    const std::vector<PharmacyClient> clients{
        PharmacyClient("Белфармация №1", "100000001", "02040/0123", 0.05),
        PharmacyClient("Аптека Здоровье", "191234567", "02040/0456", 0.0),
    };

    Warehouse mainWarehouse("Минск-Центральный", 20.0);
    ColdStorageWarehouse coldWarehouse("Минск-Холод", 5.0);

    SalesDepartment sales;
    sales.attachWarehouse(mainWarehouse);
    sales.attachWarehouse(coldWarehouse);

    for (const std::shared_ptr<const Product>& product : catalog)
        sales.purchase(suppliers[0], product, 50);

    int choice = -1;

    do
    {
        printMenu();
        std::cin >> choice;

        if (!std::cin)
        {
            if (std::cin.eof()) break;
            clearInput();
            choice = -1;
            std::cout << "Некорректный ввод.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        try
        {
            switch (choice)
            {
                case 1:
                    printCatalog(catalog);
                    break;

                case 2:
                    std::cout << "Поставщики:\n";
                    printOrganizations(suppliers);
                    std::cout << "Клиенты:\n";
                    printOrganizations(clients);
                    break;

                case 3:
                    for (const Warehouse* warehouse : sales.getWarehouses()) warehouse->printStock(std::cout);
                    break;

                case 4:
                    purchase(sales, catalog, suppliers);
                    break;

                case 5:
                    sell(sales, clients);
                    break;

                case 6:
                    changeColdTemperature(coldWarehouse);
                    break;

                case 7:
                    printJournal(sales);
                    break;

                case 8:
                    demonstrateInheritance();
                    break;

                case 0:
                    std::cout << "Выход.\n";
                    break;

                default:
                    std::cout << "Нет такого пункта.\n";
                    break;
            }
        }
        catch (const std::exception& error)
        {
            std::cout << "Ошибка: " << error.what() << "\n";
        }

    } while (choice != 0);

    return 0;
}
