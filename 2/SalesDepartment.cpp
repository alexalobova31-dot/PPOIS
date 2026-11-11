#include "SalesDepartment.h"

#include <stdexcept>

void Ledger::record(const std::string& description, double amount)
{
    journal.push_back(Entry{description, amount});
}

double Ledger::balance() const
{
    double total = 0.0;
    for (const Entry& entry : journal) total += entry.amount;
    return total;
}

const std::vector<Ledger::Entry>& Ledger::entries() const
{
    return journal;
}

std::size_t Ledger::entryCount() const
{
    return journal.size();
}

Warehouse* SalesDepartment::findWarehouseWithStock(const std::string& sku, int quantity) const
{
    for (Warehouse* warehouse : warehouses)
        if (warehouse->quantityOf(sku) >= quantity) return warehouse;
    return nullptr;
}

Warehouse* SalesDepartment::findWarehouseFor(const Product& product) const
{
    for (Warehouse* warehouse : warehouses)
        if (warehouse->canStore(product)) return warehouse;
    return nullptr;
}

void SalesDepartment::attachWarehouse(Warehouse& warehouse)
{
    for (const Warehouse* attached : warehouses)
        if (attached == &warehouse)
            throw std::invalid_argument("Warehouse is already attached");
    warehouses.push_back(&warehouse);
}

const std::vector<Warehouse*>& SalesDepartment::getWarehouses() const
{
    return warehouses;
}

const Warehouse& SalesDepartment::purchase(const Supplier& supplier, const std::shared_ptr<const Product>& product, int quantity)
{
    if (!product)
        throw std::invalid_argument("Product cannot be null");
    if (quantity <= 0)
        throw std::invalid_argument("Quantity must be positive");

    Warehouse* warehouse = findWarehouseFor(*product);
    if (warehouse == nullptr)
        throw std::runtime_error("No warehouse with suitable storage conditions for " + product->getSku());

    warehouse->receive(product, quantity);
    record("Закупка " + product->getName() + " x" + std::to_string(quantity) + " у " + supplier.getName(), -product->getBasePrice() * quantity);
    return *warehouse;
}

double SalesDepartment::sell(const PharmacyClient& client, const std::string& sku, int quantity, const Prescription* prescription)
{
    if (quantity <= 0)
        throw std::invalid_argument("Quantity must be positive");

    Warehouse* warehouse = findWarehouseWithStock(sku, quantity);
    if (warehouse == nullptr)
        throw std::runtime_error("Not enough stock for: " + sku);

    const std::shared_ptr<const Product> product = warehouse->find(sku);
    if (!product->canBeDispensed(prescription))
        throw std::runtime_error("Valid prescription is required for: " + sku);

    const double amount = product->price() * quantity * (1.0 - client.getDiscount());
    warehouse->ship(sku, quantity);
    record("Продажа " + product->getName() + " x" + std::to_string(quantity) + " аптеке " + client.getName(), amount);
    return amount;
}
