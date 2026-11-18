#ifndef STOCK_MANAGEMENT_H
#define STOCK_MANAGEMENT_H

#include "Catalog.h"

#include <map>

class StorageZone
{
private:
    std::string name;
    StorageCondition condition;
    int capacity;
    std::vector<ProductBatch*> batches;

public:
    StorageZone(const std::string& name, const StorageCondition& condition, int capacity);

    const std::vector<ProductBatch*>& getBatches() const;

    int occupied() const;
    void receive(ProductBatch& batch, const Date& date, double temperature, double humidity);
    void remove(ProductBatch& batch);
    ProductBatch& selectFirstExpiring(const std::string& sku, const Date& date) const;
    int quantityOf(const std::string& sku) const;
};

class StorageRack
{
private:
    std::string code;
    const StorageZone* zone;
    int shelves;
    int unitsPerShelf;
    int occupied;

public:
    StorageRack(const std::string& code, const StorageZone& zone, int shelves, int unitsPerShelf);

    void allocate(int quantity);
    void release(int quantity);
    int freeCapacity() const;
};

class Reservation
{
public:
    enum class State { Pending, Active, Shipped, Cancelled };

private:
    std::string number;
    ProductBatch* batch;
    int quantity;
    Date validUntil;
    State state;

public:
    Reservation(const std::string& number, ProductBatch& batch, int quantity, const Date& validUntil);
    Reservation(const Reservation&) = delete;
    Reservation& operator=(const Reservation&) = delete;

    State getState() const;
    const ProductBatch& getBatch() const;
    int getQuantity() const;

    void activate(const Date& date);
    bool canShip(const Date& date) const;
    void ship(const Date& date);
    void cancel();
};

class TemperatureLog
{
private:
    const StorageZone* zone;
    Date date;
    double minimum;
    double maximum;
    std::vector<double> readings;

public:
    TemperatureLog(const StorageZone& zone, const Date& date, double minimum, double maximum);

    void record(double temperature);
    int excursionCount() const;
    double average() const;
    void requireSafeFor(const Product& product) const;
};

class StockAudit
{
private:
    const StorageZone* zone;
    Date date;
    std::map<std::string, int> counted;
    bool closed;

public:
    StockAudit(const StorageZone& zone, const Date& date);

    void record(const std::string& sku, int quantity);
    std::map<std::string, int> discrepancies() const;
    void close();
};

class ReorderPolicy
{
private:
    const Product* product;
    int minimum;
    int target;
    int packSize;

public:
    ReorderPolicy(const Product& product, int minimum, int target, int packSize);

    bool needsOrder(const StorageZone& zone) const;
    int recommendedQuantity(const StorageZone& zone) const;
    Money estimateCost(const StorageZone& zone, const Money& unitPrice) const;
};

#endif
