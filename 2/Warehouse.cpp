#include "Warehouse.h"

#include <stdexcept>
#include <cmath>
#include <limits>

Inventory::~Inventory() = default;

void Inventory::add(const std::shared_ptr<const Product>& product, int quantity)
{
    if (!product)
        throw std::invalid_argument("Product cannot be null");
    if (quantity <= 0)
        throw std::invalid_argument("Quantity must be positive");

    const auto it = stock.find(product->getSku());
    if (it == stock.end())
    {
        stock.emplace(product->getSku(), StockItem{product, quantity});
        return;
    }
    if (it->second.product != product)
        throw std::invalid_argument("Another product with the same SKU is already in stock");
    if (quantity > std::numeric_limits<int>::max() - it->second.quantity)
        throw std::overflow_error("Stock quantity overflow");
    it->second.quantity += quantity;
}

void Inventory::remove(const std::string& sku, int quantity)
{
    if (quantity <= 0)
        throw std::invalid_argument("Quantity must be positive");

    const auto it = stock.find(sku);
    if (it == stock.end())
        throw std::out_of_range("Product not found in stock: " + sku);
    if (it->second.quantity < quantity)
        throw std::runtime_error("Not enough stock for: " + sku);

    it->second.quantity -= quantity;
    if (it->second.quantity == 0) stock.erase(it);
}

int Inventory::quantityOf(const std::string& sku) const
{
    const auto it = stock.find(sku);
    return it == stock.end() ? 0 : it->second.quantity;
}

bool Inventory::contains(const std::string& sku) const
{
    return stock.count(sku) != 0;
}

std::shared_ptr<const Product> Inventory::find(const std::string& sku) const
{
    const auto it = stock.find(sku);
    return it == stock.end() ? nullptr : it->second.product;
}

std::size_t Inventory::size() const
{
    return stock.size();
}

const std::map<std::string, Inventory::StockItem>& Inventory::items() const
{
    return stock;
}

Warehouse::Warehouse(const std::string& name, double temperature) : name(name), temperature(temperature)
{
    if (name.empty())
        throw std::invalid_argument("Warehouse name cannot be empty");
    if (!std::isfinite(temperature))
        throw std::invalid_argument("Warehouse temperature must be finite");
}

void Warehouse::setTemperature(double value)
{
    if (!std::isfinite(value))
        throw std::invalid_argument("Warehouse temperature must be finite");
    temperature = value;
}

const std::string& Warehouse::getName() const
{
    return name;
}

double Warehouse::getTemperature() const
{
    return temperature;
}

std::string Warehouse::type() const
{
    return "Склад общего хранения";
}

bool Warehouse::canStore(const Product& product) const
{
    return product.canBeStoredAt(temperature);
}

void Warehouse::receive(const std::shared_ptr<const Product>& product, int quantity)
{
    if (!product)
        throw std::invalid_argument("Product cannot be null");
    if (!canStore(*product))
        throw std::runtime_error("Product " + product->getSku() + " cannot be stored at warehouse " + name);
    add(product, quantity);
}

std::shared_ptr<const Product> Warehouse::ship(const std::string& sku, int quantity)
{
    std::shared_ptr<const Product> product = find(sku);
    remove(sku, quantity);
    return product;
}

void Warehouse::printStock(std::ostream& os) const
{
    os << type() << " \"" << name << "\", " << temperature << " °C\n";
    if (items().empty())
    {
        os << "  (пусто)\n";
        return;
    }
    for (const auto& [sku, item] : items())
        os << "  " << *item.product << " — " << item.quantity << " шт.\n";
}

ColdStorageWarehouse::ColdStorageWarehouse(const std::string& name, double temperature) : Warehouse(name, temperature)
{
}

std::string ColdStorageWarehouse::type() const
{
    return "Холодильный склад";
}

std::vector<std::string> ColdStorageWarehouse::changeTemperature(double value)
{
    setTemperature(value);

    std::vector<StockItem> spoiled;
    for (const auto& [sku, item] : Inventory::items())
        if (!canStore(*item.product)) spoiled.push_back(item);

    std::vector<std::string> writtenOff;
    for (const StockItem& item : spoiled)
    {
        Inventory::remove(item.product->getSku(), item.quantity);
        writtenOff.push_back(item.product->getName());
    }
    return writtenOff;
}
