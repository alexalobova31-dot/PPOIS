#ifndef SALES_DEPARTMENT_H
#define SALES_DEPARTMENT_H

#include "Organization.h"
#include "Product.h"
#include "Warehouse.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class Ledger
{
public:
    struct Entry
    {
        std::string description;
        double amount;
    };

    void record(const std::string& description, double amount);

    double balance() const;

    const std::vector<Entry>& entries() const;

    std::size_t entryCount() const;

private:
    std::vector<Entry> journal;
};

class SalesDepartment : private Ledger
{
private:
    std::vector<Warehouse*> warehouses;

    Warehouse* findWarehouseWithStock(const std::string& sku, int quantity) const;

    Warehouse* findWarehouseFor(const Product& product) const;

public:
    using Ledger::Entry;
    using Ledger::balance;
    using Ledger::entries;
    using Ledger::entryCount;

    void attachWarehouse(Warehouse& warehouse);

    const std::vector<Warehouse*>& getWarehouses() const;

    const Warehouse& purchase(const Supplier& supplier, const std::shared_ptr<const Product>& product, int quantity);

    double sell(const PharmacyClient& client, const std::string& sku, int quantity, const Prescription* prescription = nullptr);
};

#endif
