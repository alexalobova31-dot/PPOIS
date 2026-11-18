#ifndef ORGANIZATION_H
#define ORGANIZATION_H

#include <cstddef>
#include <iostream>
#include <string>

class Organization
{
private:
    std::string name;
    std::string taxId;

public:
    static constexpr std::size_t TAX_ID_LENGTH = 9;

    Organization(const std::string& name, const std::string& taxId);

    virtual ~Organization();

    const std::string& getName() const;

    const std::string& getTaxId() const;

    virtual std::string role() const = 0;

    virtual void print(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const Organization& organization);
};

class Supplier : public Organization
{
private:
    std::string country;

public:
    Supplier(const std::string& name, const std::string& taxId, const std::string& country);

    const std::string& getCountry() const;

    std::string role() const override;
    void print(std::ostream& os) const override;
};

class PharmacyClient : public Organization
{
private:
    std::string licenseNumber;
    double discount;

public:
    static constexpr double MAX_DISCOUNT = 0.3;

    PharmacyClient(const std::string& name, const std::string& taxId, const std::string& licenseNumber, double discount);

    const std::string& getLicenseNumber() const;

    double getDiscount() const;

    std::string role() const override;
    void print(std::ostream& os) const override;
};

#endif
