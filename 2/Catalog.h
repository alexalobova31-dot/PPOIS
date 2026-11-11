#ifndef CATALOG_H
#define CATALOG_H

#include "Common.h"
#include "Product.h"

#include <memory>
#include <vector>

class StorageCondition
{
private:
    double minTemperature;
    double maxTemperature;
    double maxHumidity;
    bool protectFromLight;

public:
    StorageCondition(double minTemperature, double maxTemperature, double maxHumidity, bool protectFromLight);

    bool accepts(double temperature, double humidity, bool exposedToLight) const;
    void requireAcceptable(double temperature, double humidity, bool exposedToLight) const;
    bool overlaps(const StorageCondition& other) const;
};

class ProductBatch
{
private:
    std::shared_ptr<const Product> product;
    std::string number;
    Date expiry;
    int quantity;
    int reserved;
    bool quarantined;

public:
    ProductBatch(const std::shared_ptr<const Product>& product, const std::string& number, const Date& expiry, int quantity);

    const Product& getProduct() const;
    const std::string& getNumber() const;
    const Date& getExpiry() const;
    int getQuantity() const;
    int available() const;

    bool isExpired(const Date& date) const;
    bool canRelease(const Date& date) const;
    void reserve(int count, const Date& date);
    void releaseReservation(int count);
    void shipReserved(int count, const Date& date);
    void quarantine();
    void releaseQuarantine();
};

class QualityCertificate
{
private:
    const ProductBatch* batch;
    const License* issuer;
    Date issued;
    Date validUntil;
    bool approved;

public:
    QualityCertificate(const ProductBatch& batch, const License& issuer, const Date& issued, const Date& validUntil);

    void approve();
    void withdraw();
    bool covers(const ProductBatch& candidate, const Date& date) const;
};

class ProductCatalog
{
private:
    std::string name;
    std::vector<std::shared_ptr<const Product>> products;
    bool published;

public:
    explicit ProductCatalog(const std::string& name);

    void add(const std::shared_ptr<const Product>& product);
    void remove(const std::string& sku);
    std::shared_ptr<const Product> find(const std::string& sku) const;
    std::vector<std::shared_ptr<const Product>> suitableFor(double temperature) const;
    void publish();
};

class RecallNotice
{
private:
    std::string number;
    std::string sku;
    std::string batchNumber;
    std::string reason;
    Date issued;

public:
    RecallNotice(const std::string& number, const std::string& sku, const std::string& batchNumber, const std::string& reason, const Date& issued);

    bool appliesTo(const ProductBatch& batch) const;
    void execute(ProductBatch& batch, const Date& date) const;
    std::string describe() const;
};

class Packaging
{
private:
    std::string material;
    int capacity;
    bool insulated;
    int packed;

public:
    Packaging(const std::string& material, int capacity, bool insulated);

    void pack(const Product& product, int quantity);
    void unpack(int quantity);
    int remainingCapacity() const;
    bool protects(const Product& product) const;
};

#endif
