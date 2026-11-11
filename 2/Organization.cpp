#include "Organization.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

Organization::Organization(const std::string& name, const std::string& taxId) : name(name), taxId(taxId)
{
    if (name.empty())
        throw std::invalid_argument("Organization name cannot be empty");

    const bool isDigitsOnly = std::all_of(taxId.begin(), taxId.end(), [](unsigned char symbol) { return std::isdigit(symbol) != 0; });
    if (taxId.size() != TAX_ID_LENGTH || !isDigitsOnly)
        throw std::invalid_argument("Tax ID must consist of 9 digits");
}

Organization::~Organization() = default;

const std::string& Organization::getName() const
{
    return name;
}

const std::string& Organization::getTaxId() const
{
    return taxId;
}

void Organization::print(std::ostream& os) const
{
    os << role() << " \"" << name << "\", УНП " << taxId;
}

std::ostream& operator<<(std::ostream& os, const Organization& organization)
{
    organization.print(os);
    return os;
}

Supplier::Supplier(const std::string& name, const std::string& taxId, const std::string& country) : Organization(name, taxId), country(country)
{
    if (country.empty())
        throw std::invalid_argument("Country cannot be empty");
}

const std::string& Supplier::getCountry() const
{
    return country;
}

std::string Supplier::role() const
{
    return "Поставщик";
}

void Supplier::print(std::ostream& os) const
{
    Organization::print(os);
    os << ", " << country;
}

PharmacyClient::PharmacyClient(const std::string& name, const std::string& taxId, const std::string& licenseNumber, double discount) : Organization(name, taxId), licenseNumber(licenseNumber), discount(discount)
{
    if (licenseNumber.empty())
        throw std::invalid_argument("License number cannot be empty");
    if (!std::isfinite(discount) || discount < 0.0 || discount > MAX_DISCOUNT)
        throw std::invalid_argument("Discount must be in range [0, 0.3]");
}

const std::string& PharmacyClient::getLicenseNumber() const
{
    return licenseNumber;
}

double PharmacyClient::getDiscount() const
{
    return discount;
}

std::string PharmacyClient::role() const
{
    return "Аптека";
}

void PharmacyClient::print(std::ostream& os) const
{
    Organization::print(os);
    os << ", лицензия " << licenseNumber << ", скидка " << discount * 100.0 << "%";
}
