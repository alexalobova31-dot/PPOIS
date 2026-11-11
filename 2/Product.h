#ifndef PRODUCT_H
#define PRODUCT_H

#include <iostream>
#include <string>

struct Prescription
{
    std::string number;
    std::string activeSubstance;
};

class Product
{
private:
    std::string sku;
    std::string name;
    std::string manufacturer;
    double basePrice;

public:
    Product(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice);

    virtual ~Product();

    const std::string& getSku() const;

    const std::string& getName() const;

    const std::string& getManufacturer() const;

    double getBasePrice() const;

    virtual double price() const = 0;

    virtual std::string category() const = 0;

    virtual bool requiresPrescription() const;

    virtual bool canBeDispensed(const Prescription* prescription) const;

    virtual bool canBeStoredAt(double temperature) const;

    virtual void print(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const Product& product);
};

class Medicine : public virtual Product
{
private:
    std::string activeSubstance;
    double dosageMg;

public:
    static constexpr double VAT_RATE = 0.10;

    Medicine(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg);

    const std::string& getActiveSubstance() const;

    double getDosageMg() const;

    double price() const override;
    std::string category() const override;
    void print(std::ostream& os) const override;
};

class OverTheCounterDrug : public Medicine
{
private:
    double promoDiscount;

public:
    OverTheCounterDrug(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg, double promoDiscount);

    double getPromoDiscount() const;

    double price() const override;
    std::string category() const override;
};

class PrescriptionDrug : public virtual Medicine
{
public:
    PrescriptionDrug(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg);

    bool isAcceptable(const Prescription& prescription) const;

    std::string category() const override;
    bool requiresPrescription() const override;
    bool canBeDispensed(const Prescription* prescription) const override;
};

class ColdChainProduct : public virtual Medicine
{
private:
    double minTemperature;
    double maxTemperature;

public:
    static constexpr double COLD_CHAIN_FEE = 2.5;

    ColdChainProduct(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg, double minTemperature, double maxTemperature);

    bool isAcceptable(double temperature) const;

    double getMinTemperature() const;

    double getMaxTemperature() const;

    double price() const override;
    std::string category() const override;
    bool canBeStoredAt(double temperature) const override;
    void print(std::ostream& os) const override;
};

class Vaccine : public PrescriptionDrug, public ColdChainProduct
{
private:
    int dosesPerPackage;

public:
    using PrescriptionDrug::isAcceptable;
    using ColdChainProduct::isAcceptable;

    Vaccine(const std::string& sku, const std::string& name, const std::string& manufacturer, double basePrice, const std::string& activeSubstance, double dosageMg, double minTemperature, double maxTemperature, int dosesPerPackage);

    int getDosesPerPackage() const;

    double price() const override;
    std::string category() const override;
    void print(std::ostream& os) const override;
};

#endif
