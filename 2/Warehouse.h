#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include "Product.h"

#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Inventory
{
public:
    struct StockItem
    {
        std::shared_ptr<const Product> product;
        int quantity;
    };

    virtual ~Inventory();

    void add(const std::shared_ptr<const Product>& product, int quantity);

    void remove(const std::string& sku, int quantity);

    int quantityOf(const std::string& sku) const;

    bool contains(const std::string& sku) const;

    std::shared_ptr<const Product> find(const std::string& sku) const;

    std::size_t size() const;

    const std::map<std::string, StockItem>& items() const;

private:
    std::map<std::string, StockItem> stock;
};

class Warehouse : protected Inventory
{
private:
    std::string name;
    double temperature;

protected:
    void setTemperature(double value);

public:
    using Inventory::StockItem;
    using Inventory::quantityOf;
    using Inventory::contains;
    using Inventory::find;
    using Inventory::size;

    Warehouse(const std::string& name, double temperature);

    const std::string& getName() const;

    double getTemperature() const;

    virtual std::string type() const;

    virtual bool canStore(const Product& product) const;

    void receive(const std::shared_ptr<const Product>& product, int quantity);

    std::shared_ptr<const Product> ship(const std::string& sku, int quantity);

    void printStock(std::ostream& os) const;
};

class ColdStorageWarehouse : public Warehouse
{
public:
    ColdStorageWarehouse(const std::string& name, double temperature);

    std::string type() const override;

    std::vector<std::string> changeTemperature(double value);
};

#endif
