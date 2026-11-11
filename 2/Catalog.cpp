#include "Catalog.h"

#include <algorithm>
#include <cmath>

StorageCondition::StorageCondition(double minTemperature, double maxTemperature, double maxHumidity, bool protectFromLight) : minTemperature(minTemperature), maxTemperature(maxTemperature), maxHumidity(maxHumidity), protectFromLight(protectFromLight)
{
    if (!std::isfinite(minTemperature) || !std::isfinite(maxTemperature) || minTemperature >= maxTemperature || !std::isfinite(maxHumidity) || maxHumidity < 0.0 || maxHumidity > 100.0)
        throw ValidationException("Invalid storage conditions");
}

bool StorageCondition::accepts(double temperature, double humidity, bool exposedToLight) const
{
    return std::isfinite(temperature) && std::isfinite(humidity) && temperature >= minTemperature && temperature <= maxTemperature && humidity >= 0.0 && humidity <= maxHumidity && !(protectFromLight && exposedToLight);
}

void StorageCondition::requireAcceptable(double temperature, double humidity, bool exposedToLight) const
{
    if (!accepts(temperature, humidity, exposedToLight))
        throw TemperatureException("Storage conditions were violated");
}

bool StorageCondition::overlaps(const StorageCondition& other) const
{
    return minTemperature <= other.maxTemperature && other.minTemperature <= maxTemperature;
}

ProductBatch::ProductBatch(const std::shared_ptr<const Product>& product, const std::string& number, const Date& expiry, int quantity) : product(product), number(number), expiry(expiry), quantity(quantity), reserved(0), quarantined(false)
{
    if (!product || number.empty() || quantity <= 0)
        throw ValidationException("Batch requires a product, a number and a positive quantity");
}

const Product& ProductBatch::getProduct() const
{
    return *product;
}

const std::string& ProductBatch::getNumber() const
{
    return number;
}

const Date& ProductBatch::getExpiry() const
{
    return expiry;
}

int ProductBatch::getQuantity() const
{
    return quantity;
}

int ProductBatch::available() const
{
    return quantity - reserved;
}

bool ProductBatch::isExpired(const Date& date) const
{
    return expiry.isBefore(date);
}

bool ProductBatch::canRelease(const Date& date) const
{
    return !quarantined && !isExpired(date);
}

void ProductBatch::reserve(int count, const Date& date)
{
    if (isExpired(date))
        throw ExpiredProductException("Cannot reserve an expired batch");
    if (quarantined)
        throw QualityException("Batch is quarantined");
    if (count <= 0 || count > available())
        throw StockException("Not enough available units in the batch");
    reserved += count;
}

void ProductBatch::releaseReservation(int count)
{
    if (count <= 0 || count > reserved)
        throw StockException("Invalid reservation release");
    reserved -= count;
}

void ProductBatch::shipReserved(int count, const Date& date)
{
    if (isExpired(date))
        throw ExpiredProductException("Cannot ship an expired batch");
    if (quarantined)
        throw QualityException("Batch is quarantined");
    if (count <= 0 || count > reserved)
        throw StockException("Shipment exceeds reservation");
    reserved -= count;
    quantity -= count;
}

void ProductBatch::quarantine()
{
    quarantined = true;
}

void ProductBatch::releaseQuarantine()
{
    if (!quarantined)
        throw QualityException("Batch is not quarantined");
    quarantined = false;
}

QualityCertificate::QualityCertificate(const ProductBatch& batch, const License& issuer, const Date& issued, const Date& validUntil) : batch(&batch), issuer(&issuer), issued(issued), validUntil(validUntil), approved(false)
{
    if (validUntil.isBefore(issued) || batch.getExpiry().isBefore(validUntil))
        throw QualityException("Invalid certificate validity period");
}

void QualityCertificate::approve()
{
    issuer->requireValid("quality", issued);
    if (!batch->canRelease(issued))
        throw QualityException("Batch cannot be certified");
    approved = true;
}

void QualityCertificate::withdraw()
{
    approved = false;
}

bool QualityCertificate::covers(const ProductBatch& candidate, const Date& date) const
{
    return approved && batch == &candidate && !date.isBefore(issued) && !validUntil.isBefore(date) && issuer->permits("quality", date) && candidate.canRelease(date);
}

ProductCatalog::ProductCatalog(const std::string& name) : name(name), published(false)
{
    if (name.empty())
        throw ValidationException("Catalog name is required");
}

void ProductCatalog::add(const std::shared_ptr<const Product>& product)
{
    if (!product)
        throw ValidationException("Product cannot be null");
    if (find(product->getSku()))
        throw DuplicateException("SKU is already in the catalog");
    products.push_back(product);
    published = false;
}

void ProductCatalog::remove(const std::string& sku)
{
    const auto it = std::find_if(products.begin(), products.end(), [&sku](const auto& product) { return product->getSku() == sku; });
    if (it == products.end())
        throw StockException("Product is not in the catalog");
    products.erase(it);
    published = false;
}

std::shared_ptr<const Product> ProductCatalog::find(const std::string& sku) const
{
    for (const auto& product : products)
        if (product->getSku() == sku) return product;
    return nullptr;
}

std::vector<std::shared_ptr<const Product>> ProductCatalog::suitableFor(double temperature) const
{
    std::vector<std::shared_ptr<const Product>> result;
    for (const auto& product : products)
        if (product->canBeStoredAt(temperature)) result.push_back(product);
    return result;
}

void ProductCatalog::publish()
{
    if (products.empty() || published)
        throw OrderException("Catalog is empty or already published");
    published = true;
}

RecallNotice::RecallNotice(const std::string& number, const std::string& sku, const std::string& batchNumber, const std::string& reason, const Date& issued) : number(number), sku(sku), batchNumber(batchNumber), reason(reason), issued(issued)
{
    if (number.empty() || sku.empty() || batchNumber.empty() || reason.empty())
        throw QualityException("Recall notice must be complete");
}

bool RecallNotice::appliesTo(const ProductBatch& batch) const
{
    return batch.getProduct().getSku() == sku && batch.getNumber() == batchNumber;
}

void RecallNotice::execute(ProductBatch& batch, const Date& date) const
{
    if (!appliesTo(batch) || date.isBefore(issued))
        throw QualityException("Recall does not apply to this batch or date");
    batch.quarantine();
}

std::string RecallNotice::describe() const
{
    return number + ": " + sku + "/" + batchNumber + " — " + reason;
}

Packaging::Packaging(const std::string& material, int capacity, bool insulated) : material(material), capacity(capacity), insulated(insulated), packed(0)
{
    if (material.empty() || capacity <= 0)
        throw ValidationException("Invalid packaging");
}

void Packaging::pack(const Product& product, int quantity)
{
    if (!protects(product))
        throw TemperatureException("Cold-chain product requires insulated packaging");
    if (quantity <= 0 || quantity > remainingCapacity())
        throw StockException("Packaging capacity exceeded");
    packed += quantity;
}

void Packaging::unpack(int quantity)
{
    if (quantity <= 0 || quantity > packed)
        throw StockException("Invalid unpack quantity");
    packed -= quantity;
}

int Packaging::remainingCapacity() const
{
    return capacity - packed;
}

bool Packaging::protects(const Product& product) const
{
    return insulated || product.canBeStoredAt(20.0);
}
