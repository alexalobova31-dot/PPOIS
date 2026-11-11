#include "Organization.h"
#include "Product.h"
#include "SalesDepartment.h"
#include "Warehouse.h"

#include <UnitTest++/UnitTest++.h>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace
{
    constexpr double EPSILON = 1e-9;

    std::shared_ptr<const Product> makeOtc()
    {
        return std::make_shared<OverTheCounterDrug>("OTC-1", "Парацетамол", "БЗМП", 10.0, "парацетамол", 500.0, 0.1);
    }

    std::shared_ptr<const Product> makeRx()
    {
        return std::make_shared<PrescriptionDrug>("RX-1", "Амоксициллин", "Лекфарм", 20.0, "амоксициллин", 500.0);
    }

    std::shared_ptr<const Product> makeVaccine()
    {
        return std::make_shared<Vaccine>("VAC-1", "Гриппол", "Петровакс", 30.0, "антиген", 0.5, 2.0, 8.0, 1);
    }

    std::string toString(const Product& product)
    {
        std::ostringstream stream;
        stream << product;
        return stream.str();
    }
}

SUITE(Products)
{
    TEST(MedicinePriceIncludesVat)
    {
        const PrescriptionDrug drug("RX-1", "Амоксициллин", "Лекфарм", 20.0, "амоксициллин", 500.0);

        CHECK_CLOSE(22.0, drug.price(), EPSILON);
    }

    TEST(OverTheCounterPriceAppliesPromoDiscount)
    {
        CHECK_CLOSE(10.0 * 1.1 * 0.9, makeOtc()->price(), EPSILON);
    }

    TEST(ColdChainPriceIncludesFee)
    {
        const ColdChainProduct insulin("CC-1", "Инсулин", "БМП", 10.0, "инсулин", 3.5, 2.0, 8.0);

        CHECK_CLOSE(11.0 + ColdChainProduct::COLD_CHAIN_FEE, insulin.price(), EPSILON);
    }

    TEST(VaccinePriceIsResolvedThroughVirtualCall)
    {
        const std::shared_ptr<const Product> vaccine = makeVaccine();

        CHECK_CLOSE(33.0 + ColdChainProduct::COLD_CHAIN_FEE, vaccine->price(), EPSILON);
        CHECK_EQUAL("Вакцина", vaccine->category());
    }

    TEST(VaccineHasSingleMedicineSubobject)
    {
        const Vaccine vaccine("VAC-1", "Гриппол", "Петровакс", 30.0, "антиген", 0.5, 2.0, 8.0, 1);

        const Medicine& viaPrescription = static_cast<const PrescriptionDrug&>(vaccine);
        const Medicine& viaColdChain = static_cast<const ColdChainProduct&>(vaccine);

        CHECK(&viaPrescription == &viaColdChain);
        CHECK_EQUAL("VAC-1", vaccine.getSku());
    }

    TEST(VaccineUsingDeclarationsExposeBothOverloads)
    {
        const Vaccine vaccine("VAC-1", "Гриппол", "Петровакс", 30.0, "антиген", 0.5, 2.0, 8.0, 1);

        CHECK(vaccine.isAcceptable(Prescription{"N1", "антиген"}));
        CHECK(!vaccine.isAcceptable(Prescription{"N1", "ибупрофен"}));
        CHECK(!vaccine.isAcceptable(Prescription{"", "антиген"}));
        CHECK(vaccine.isAcceptable(5.0));
        CHECK(!vaccine.isAcceptable(20.0));
    }

    TEST(VaccineCombinesBehaviourOfBothBases)
    {
        const std::shared_ptr<const Product> vaccine = makeVaccine();
        const Prescription prescription{"N1", "антиген"};

        CHECK(vaccine->requiresPrescription());
        CHECK(!vaccine->canBeDispensed(nullptr));
        CHECK(vaccine->canBeDispensed(&prescription));
        CHECK(vaccine->canBeStoredAt(4.0));
        CHECK(!vaccine->canBeStoredAt(20.0));
    }

    TEST(OrdinaryMedicineIsStoredAtRoomTemperature)
    {
        const std::shared_ptr<const Product> drug = makeOtc();

        CHECK(drug->canBeStoredAt(20.0));
        CHECK(!drug->canBeStoredAt(5.0));
        CHECK(!drug->requiresPrescription());
        CHECK(drug->canBeDispensed(nullptr));
    }

    TEST(PrintUsesMostDerivedOverride)
    {
        const std::string text = toString(*makeVaccine());

        CHECK(text.find("Вакцина") != std::string::npos);
        CHECK(text.find("по рецепту") != std::string::npos);
        CHECK(text.find("доз в упаковке: 1") != std::string::npos);
    }

    TEST(ConstructorsValidateArguments)
    {
        CHECK_THROW(PrescriptionDrug("", "A", "B", 1.0, "c", 1.0), std::invalid_argument);
        CHECK_THROW(PrescriptionDrug("S", "A", "B", 0.0, "c", 1.0), std::invalid_argument);
        CHECK_THROW(PrescriptionDrug("S", "A", "B", 1.0, "", 1.0), std::invalid_argument);
        CHECK_THROW(OverTheCounterDrug("S", "A", "B", 1.0, "c", 1.0, 0.9), std::invalid_argument);
        CHECK_THROW(ColdChainProduct("S", "A", "B", 1.0, "c", 1.0, 8.0, 2.0), std::invalid_argument);
        CHECK_THROW(Vaccine("S", "A", "B", 1.0, "c", 1.0, 2.0, 8.0, 0), std::invalid_argument);
    }
}

SUITE(Organizations)
{
    TEST(OrganizationValidatesTaxId)
    {
        CHECK_THROW(Supplier("A", "12345", "BY"), std::invalid_argument);
        CHECK_THROW(Supplier("A", "12345678X", "BY"), std::invalid_argument);
        CHECK_EQUAL("123456789", Supplier("A", "123456789", "BY").getTaxId());
    }

    TEST(PharmacyClientValidatesDiscountAndLicense)
    {
        CHECK_THROW(PharmacyClient("A", "123456789", "L", 0.5), std::invalid_argument);
        CHECK_THROW(PharmacyClient("A", "123456789", "", 0.1), std::invalid_argument);
    }

    TEST(RoleIsVirtual)
    {
        const Supplier supplier("A", "123456789", "BY");
        const PharmacyClient client("B", "987654321", "L-1", 0.1);
        const Organization& first = supplier;
        const Organization& second = client;

        CHECK_EQUAL("Поставщик", first.role());
        CHECK_EQUAL("Аптека", second.role());
    }
}

SUITE(Warehouses)
{
    TEST(WarehouseIsNotConvertibleToInventory)
    {
        CHECK((!std::is_convertible<Warehouse*, Inventory*>::value));
        CHECK((!std::is_convertible<SalesDepartment*, Ledger*>::value));
        CHECK((std::is_convertible<Vaccine*, Product*>::value));
    }

    TEST(ReceiveAndShipUpdateStock)
    {
        Warehouse warehouse("Main", 20.0);
        const std::shared_ptr<const Product> drug = makeOtc();

        warehouse.receive(drug, 10);
        warehouse.receive(drug, 5);
        CHECK_EQUAL(15, warehouse.quantityOf("OTC-1"));

        warehouse.ship("OTC-1", 15);
        CHECK(!warehouse.contains("OTC-1"));
        CHECK_EQUAL(0u, warehouse.size());
    }

    TEST(ShipThrowsWhenNotEnoughStock)
    {
        Warehouse warehouse("Main", 20.0);
        warehouse.receive(makeOtc(), 3);

        CHECK_THROW(warehouse.ship("OTC-1", 4), std::runtime_error);
        CHECK_THROW(warehouse.ship("NONE", 1), std::out_of_range);
        CHECK_EQUAL(3, warehouse.quantityOf("OTC-1"));
    }

    TEST(ReceiveRejectsInvalidQuantityAndDuplicateSku)
    {
        Warehouse warehouse("Main", 20.0);
        warehouse.receive(makeOtc(), 1);

        CHECK_THROW(warehouse.receive(makeOtc(), 1), std::invalid_argument);
        CHECK_THROW(warehouse.receive(makeRx(), 0), std::invalid_argument);
    }

    TEST(RoomWarehouseRejectsColdChainProduct)
    {
        Warehouse warehouse("Main", 20.0);

        CHECK_THROW(warehouse.receive(makeVaccine(), 1), std::runtime_error);
    }

    TEST(ColdWarehouseWritesOffOnTemperatureBreach)
    {
        ColdStorageWarehouse warehouse("Cold", 5.0);
        warehouse.receive(makeVaccine(), 10);

        CHECK(warehouse.changeTemperature(7.0).empty());
        CHECK_EQUAL(10, warehouse.quantityOf("VAC-1"));

        const std::vector<std::string> writtenOff = warehouse.changeTemperature(12.0);
        CHECK_EQUAL(1u, writtenOff.size());
        CHECK_EQUAL("Гриппол", writtenOff[0]);
        CHECK(!warehouse.contains("VAC-1"));
        CHECK_EQUAL("Холодильный склад", warehouse.type());
    }
}

SUITE(Sales)
{
    struct SalesFixture
    {
        Warehouse mainWarehouse{"Main", 20.0};
        ColdStorageWarehouse coldWarehouse{"Cold", 5.0};
        SalesDepartment sales;
        Supplier supplier{"Supplier", "123456789", "BY"};
        PharmacyClient client{"Pharmacy", "987654321", "L-1", 0.1};

        SalesFixture()
        {
            sales.attachWarehouse(mainWarehouse);
            sales.attachWarehouse(coldWarehouse);
        }
    };

    TEST_FIXTURE(SalesFixture, PurchaseRoutesProductToSuitableWarehouse)
    {
        CHECK_EQUAL("Main", sales.purchase(supplier, makeOtc(), 10).getName());
        CHECK_EQUAL("Cold", sales.purchase(supplier, makeVaccine(), 5).getName());
        CHECK_EQUAL(10, mainWarehouse.quantityOf("OTC-1"));
        CHECK_EQUAL(5, coldWarehouse.quantityOf("VAC-1"));
        CHECK_CLOSE(-(10.0 * 10 + 30.0 * 5), sales.balance(), EPSILON);
    }

    TEST_FIXTURE(SalesFixture, SellAppliesClientDiscountAndRecordsIncome)
    {
        sales.purchase(supplier, makeOtc(), 10);

        const double amount = sales.sell(client, "OTC-1", 4);

        CHECK_CLOSE(10.0 * 1.1 * 0.9 * 4 * 0.9, amount, EPSILON);
        CHECK_EQUAL(6, mainWarehouse.quantityOf("OTC-1"));
        CHECK_EQUAL(2u, sales.entryCount());
        CHECK_CLOSE(amount - 100.0, sales.balance(), EPSILON);
    }

    TEST_FIXTURE(SalesFixture, SellRequiresMatchingPrescription)
    {
        sales.purchase(supplier, makeRx(), 5);
        const Prescription wrong{"N1", "ибупрофен"};
        const Prescription valid{"N2", "амоксициллин"};

        CHECK_THROW(sales.sell(client, "RX-1", 1), std::runtime_error);
        CHECK_THROW(sales.sell(client, "RX-1", 1, &wrong), std::runtime_error);
        CHECK_EQUAL(5, mainWarehouse.quantityOf("RX-1"));

        sales.sell(client, "RX-1", 1, &valid);
        CHECK_EQUAL(4, mainWarehouse.quantityOf("RX-1"));
    }

    TEST_FIXTURE(SalesFixture, SellFailsWithoutStock)
    {
        CHECK_THROW(sales.sell(client, "OTC-1", 1), std::runtime_error);
        CHECK_THROW(sales.sell(client, "OTC-1", 0), std::invalid_argument);
        CHECK_EQUAL(0u, sales.entryCount());
    }

    TEST_FIXTURE(SalesFixture, AttachingSameWarehouseTwiceThrows)
    {
        CHECK_THROW(sales.attachWarehouse(mainWarehouse), std::invalid_argument);
    }
}

int main()
{
    return UnitTest::RunAllTests();
}
