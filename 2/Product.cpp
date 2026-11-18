#include "Product.h"

#include <iomanip>
#include <cmath>
#include <stdexcept>

namespace
{
    constexpr double ROOM_MIN_TEMPERATURE = 15.0;
    constexpr double ROOM_MAX_TEMPERATURE = 25.0;
    constexpr double MAX_PROMO_DISCOUNT = 0.5;

    void requireNotEmpty(const std::string& value, const char* message)
    {
        if (value.empty())
            throw std::invalid_argument(message);
    }
}

Product::Product(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice) : sku(sku), name(name), manufacturer(manufacturer), basePrice(basePrice)
{
    requireNotEmpty(sku, "SKU cannot be empty");
    requireNotEmpty(name, "Product name cannot be empty");
    requireNotEmpty(manufacturer, "Manufacturer cannot be empty");
    if (!std::isfinite(basePrice) || basePrice <= 0.0)
        throw std::invalid_argument("Base price must be positive");
}

Product::~Product() = default;

const std::string& Product::getSku() const
{
    return sku;
}

const std::string& Product::getName() const
{
    return name;
}

const std::string& Product::getManufacturer() const
{
    return manufacturer;
}

double Product::getBasePrice() const
{
    return basePrice;
}

bool Product::requiresPrescription() const
{
    return false;
}

bool Product::canBeDispensed(const Prescription*) const
{
    return true;
}

bool Product::canBeStoredAt(double temperature) const
{
    return temperature >= ROOM_MIN_TEMPERATURE && temperature <= ROOM_MAX_TEMPERATURE;
}

void Product::print(std::ostream& os) const
{
    const std::ios::fmtflags flags = os.flags();
    const std::streamsize precision = os.precision();

    os << "[" << sku << "] " << name << " (" << manufacturer << "), " << category() << ", цена " << std::fixed << std::setprecision(2) << price();
    if (requiresPrescription()) os << ", по рецепту";

    os.flags(flags);
    os.precision(precision);
}

std::ostream& operator<<(std::ostream& os, const Product& product)
{
    product.print(os);
    return os;
}

Medicine::Medicine(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg) : Product(sku, name, manufacturer, basePrice), activeSubstance(activeSubstance), dosageMg(dosageMg)
{
    requireNotEmpty(activeSubstance, "Active substance cannot be empty");
    if (!std::isfinite(dosageMg) || dosageMg <= 0.0)
        throw std::invalid_argument("Dosage must be positive");
}

const std::string& Medicine::getActiveSubstance() const
{
    return activeSubstance;
}

double Medicine::getDosageMg() const
{
    return dosageMg;
}

double Medicine::price() const
{
    return getBasePrice() * (1.0 + VAT_RATE);
}

std::string Medicine::category() const
{
    return "Лекарственный препарат";
}

void Medicine::print(std::ostream& os) const
{
    Product::print(os);
    os << ", " << activeSubstance << " " << dosageMg << " мг";
}

OverTheCounterDrug::OverTheCounterDrug(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg, double promoDiscount) : Product(sku, name, manufacturer, basePrice), Medicine(sku, name, manufacturer, basePrice, activeSubstance, dosageMg), promoDiscount(promoDiscount)
{
    if (!std::isfinite(promoDiscount) || promoDiscount < 0.0 || promoDiscount > MAX_PROMO_DISCOUNT)
        throw std::invalid_argument("Promo discount must be in range [0, 0.5]");
}

double OverTheCounterDrug::getPromoDiscount() const
{
    return promoDiscount;
}

double OverTheCounterDrug::price() const
{
    return Medicine::price() * (1.0 - promoDiscount);
}

std::string OverTheCounterDrug::category() const
{
    return "Безрецептурный препарат";
}

PrescriptionDrug::PrescriptionDrug(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg) : Product(sku, name, manufacturer, basePrice), Medicine(sku, name, manufacturer, basePrice, activeSubstance, dosageMg)
{
}

bool PrescriptionDrug::isAcceptable(const Prescription& prescription) const
{
    return !prescription.number.empty() && prescription.activeSubstance == getActiveSubstance();
}

std::string PrescriptionDrug::category() const
{
    return "Рецептурный препарат";
}

bool PrescriptionDrug::requiresPrescription() const
{
    return true;
}

bool PrescriptionDrug::canBeDispensed(const Prescription* prescription) const
{
    return prescription != nullptr && isAcceptable(*prescription);
}

ColdChainProduct::ColdChainProduct(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg, double minTemperature, double maxTemperature) : Product(sku, name, manufacturer, basePrice), Medicine(sku, name, manufacturer, basePrice, activeSubstance, dosageMg), minTemperature(minTemperature), maxTemperature(maxTemperature)
{
    if (!std::isfinite(minTemperature) || !std::isfinite(maxTemperature) || minTemperature >= maxTemperature)
        throw std::invalid_argument("Min temperature must be less than max temperature");
}

bool ColdChainProduct::isAcceptable(double temperature) const
{
    return temperature >= minTemperature && temperature <= maxTemperature;
}

double ColdChainProduct::getMinTemperature() const
{
    return minTemperature;
}

double ColdChainProduct::getMaxTemperature() const
{
    return maxTemperature;
}

double ColdChainProduct::price() const
{
    return Medicine::price() + COLD_CHAIN_FEE;
}

std::string ColdChainProduct::category() const
{
    return "Препарат холодовой цепи";
}

bool ColdChainProduct::canBeStoredAt(double temperature) const
{
    return isAcceptable(temperature);
}

void ColdChainProduct::print(std::ostream& os) const
{
    Medicine::print(os);
    os << ", хранение " << minTemperature << ".." << maxTemperature << " °C";
}

Vaccine::Vaccine(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg, double minTemperature, double maxTemperature, int dosesPerPackage) : Product(sku, name, manufacturer, basePrice), Medicine(sku, name, manufacturer, basePrice, activeSubstance, dosageMg), PrescriptionDrug(sku, name, manufacturer, basePrice, activeSubstance, dosageMg), ColdChainProduct(sku, name, manufacturer, basePrice, activeSubstance, dosageMg, minTemperature, maxTemperature), dosesPerPackage(dosesPerPackage)
{
    if (dosesPerPackage <= 0)
        throw std::invalid_argument("Doses per package must be positive");
}

int Vaccine::getDosesPerPackage() const
{
    return dosesPerPackage;
}

double Vaccine::price() const
{
    return ColdChainProduct::price();
}

std::string Vaccine::category() const
{
    return "Вакцина";
}

void Vaccine::print(std::ostream& os) const
{
    ColdChainProduct::print(os);
    os << ", доз в упаковке: " << dosesPerPackage;
}
