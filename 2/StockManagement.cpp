#include "StockManagement.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

StorageZone::StorageZone(const std::string& name, const StorageCondition& condition, int capacity) : name(name), condition(condition), capacity(capacity)
{
    if (name.empty() || capacity <= 0)
        throw ValidationException("Invalid storage zone");
}

const std::vector<ProductBatch*>& StorageZone::getBatches() const
{
    return batches;
}

int StorageZone::occupied() const
{
    int result = 0;
    for (const ProductBatch* batch : batches) result += batch->getQuantity();
    return result;
}

void StorageZone::receive(ProductBatch& batch, const Date& date, double temperature, double humidity)
{
    condition.requireAcceptable(temperature, humidity, false);
    if (!batch.getProduct().canBeStoredAt(temperature))
        throw TemperatureException("Zone temperature is unsuitable for the product");
    if (batch.isExpired(date))
        throw ExpiredProductException("Cannot receive an expired batch");
    for (const ProductBatch* existing : batches)
        if (existing == &batch || (existing->getProduct().getSku() == batch.getProduct().getSku() && existing->getNumber() == batch.getNumber()))
            throw DuplicateException("Batch is already in the zone");
    if (batch.getQuantity() > capacity - occupied())
        throw StockException("Storage zone capacity exceeded");
    batches.push_back(&batch);
}

void StorageZone::remove(ProductBatch& batch)
{
    const auto it = std::find(batches.begin(), batches.end(), &batch);
    if (it == batches.end() || batch.available() != batch.getQuantity())
        throw StockException("Batch is absent or has active reservations");
    batches.erase(it);
}

ProductBatch& StorageZone::selectFirstExpiring(const std::string& sku, const Date& date) const
{
    ProductBatch* selected = nullptr;
    for (ProductBatch* batch : batches)
        if (batch->getProduct().getSku() == sku && batch->canRelease(date) && batch->available() > 0 && (selected == nullptr || batch->getExpiry().isBefore(selected->getExpiry()))) selected = batch;
    if (selected == nullptr)
        throw StockException("No releasable batch for this SKU");
    return *selected;
}

int StorageZone::quantityOf(const std::string& sku) const
{
    int result = 0;
    for (const ProductBatch* batch : batches)
        if (batch->getProduct().getSku() == sku) result += batch->getQuantity();
    return result;
}

StorageRack::StorageRack(const std::string& code, const StorageZone& zone, int shelves, int unitsPerShelf) : code(code), zone(&zone), shelves(shelves), unitsPerShelf(unitsPerShelf), occupied(0)
{
    if (code.empty() || shelves <= 0 || unitsPerShelf <= 0 || shelves > std::numeric_limits<int>::max() / unitsPerShelf)
        throw ValidationException("Invalid rack dimensions");
}

void StorageRack::allocate(int quantity)
{
    if (quantity <= 0 || quantity > freeCapacity() || occupied + quantity > zone->occupied())
        throw StockException("Rack capacity or zone stock exceeded");
    occupied += quantity;
}

void StorageRack::release(int quantity)
{
    if (quantity <= 0 || quantity > occupied)
        throw StockException("Invalid rack release");
    occupied -= quantity;
}

int StorageRack::freeCapacity() const
{
    return shelves * unitsPerShelf - occupied;
}

Reservation::Reservation(const std::string& number, ProductBatch& batch, int quantity, const Date& validUntil) : number(number), batch(&batch), quantity(quantity), validUntil(validUntil), state(State::Pending)
{
    if (number.empty() || quantity <= 0)
        throw StockException("Invalid reservation");
}

Reservation::State Reservation::getState() const
{
    return state;
}

const ProductBatch& Reservation::getBatch() const
{
    return *batch;
}

int Reservation::getQuantity() const
{
    return quantity;
}

void Reservation::activate(const Date& date)
{
    if (state != State::Pending || validUntil.isBefore(date))
        throw StockException("Reservation cannot be activated");
    batch->reserve(quantity, date);
    state = State::Active;
}

bool Reservation::canShip(const Date& date) const
{
    return state == State::Active && !validUntil.isBefore(date) && batch->canRelease(date);
}

void Reservation::ship(const Date& date)
{
    if (!canShip(date))
        throw StockException("Reservation is not available for shipment");
    batch->shipReserved(quantity, date);
    state = State::Shipped;
}

void Reservation::cancel()
{
    if (state != State::Active && state != State::Pending)
        throw StockException("Reservation is already resolved");
    if (state == State::Active) batch->releaseReservation(quantity);
    state = State::Cancelled;
}

TemperatureLog::TemperatureLog(const StorageZone& zone, const Date& date, double minimum, double maximum) : zone(&zone), date(date), minimum(minimum), maximum(maximum)
{
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || minimum >= maximum)
        throw TemperatureException("Invalid temperature log limits");
}

void TemperatureLog::record(double temperature)
{
    if (!std::isfinite(temperature))
        throw TemperatureException("Temperature reading must be finite");
    readings.push_back(temperature);
}

int TemperatureLog::excursionCount() const
{
    return static_cast<int>(std::count_if(readings.begin(), readings.end(), [this](double value) { return value < minimum || value > maximum; }));
}

double TemperatureLog::average() const
{
    if (readings.empty())
        throw TemperatureException("Temperature log is empty");
    double result = 0.0;
    const double count = static_cast<double>(readings.size());
    for (double value : readings) result += value / count;
    return result;
}

void TemperatureLog::requireSafeFor(const Product& product) const
{
    if (readings.empty() || zone->getBatches().empty())
        throw TemperatureException("Temperature log or storage zone is empty");
    for (double value : readings)
        if (!product.canBeStoredAt(value))
            throw TemperatureException("Product storage temperature was violated on " + date.format());
}

StockAudit::StockAudit(const StorageZone& zone, const Date& date) : zone(&zone), date(date), closed(false)
{
}

void StockAudit::record(const std::string& sku, int quantity)
{
    if (closed || sku.empty() || quantity < 0)
        throw StockException("Invalid audit entry or audit is closed");
    if (!counted.emplace(sku, quantity).second)
        throw DuplicateException("SKU has already been counted");
}

std::map<std::string, int> StockAudit::discrepancies() const
{
    std::map<std::string, int> result;
    for (const ProductBatch* batch : zone->getBatches()) result[batch->getProduct().getSku()] = -zone->quantityOf(batch->getProduct().getSku());
    for (const auto& [sku, quantity] : counted) result[sku] += quantity;
    for (auto it = result.begin(); it != result.end();)
    {
        if (it->second == 0) it = result.erase(it);
        else ++it;
    }
    return result;
}

void StockAudit::close()
{
    if (closed || counted.empty())
        throw StockException("Audit is empty or already closed on " + date.format());
    closed = true;
}

ReorderPolicy::ReorderPolicy(const Product& product, int minimum, int target, int packSize) : product(&product), minimum(minimum), target(target), packSize(packSize)
{
    if (minimum < 0 || target <= minimum || packSize <= 0)
        throw ValidationException("Invalid reorder policy");
}

bool ReorderPolicy::needsOrder(const StorageZone& zone) const
{
    return zone.quantityOf(product->getSku()) <= minimum;
}

int ReorderPolicy::recommendedQuantity(const StorageZone& zone) const
{
    if (!needsOrder(zone)) return 0;
    const std::int64_t missing = target - zone.quantityOf(product->getSku());
    const std::int64_t result = (missing + packSize - 1) / packSize * packSize;
    if (result > std::numeric_limits<int>::max())
        throw StockException("Reorder quantity overflow");
    return static_cast<int>(result);
}

Money ReorderPolicy::estimateCost(const StorageZone& zone, const Money& unitPrice) const
{
    return unitPrice.multiply(recommendedQuantity(zone));
}
