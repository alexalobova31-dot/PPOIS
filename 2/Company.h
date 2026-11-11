#ifndef COMPANY_H
#define COMPANY_H

#include "Delivery.h"
#include "SalesDepartment.h"

#include <ostream>

class Employee
{
private:
    std::string number;
    std::string name;
    Money hourlyRate;
    bool active;

public:
    Employee(const std::string& number, const std::string& name, const Money& hourlyRate);
    virtual ~Employee();

    const std::string& getNumber() const;
    bool isActive() const;

    virtual std::string role() const;
    virtual Money payroll(int hours) const;
    void dismiss();
};

class SalesRepresentative : public Employee
{
private:
    const PharmacyClient* client;
    int commissionBasisPoints;
    Money salesVolume;
    std::vector<const SalesOrder*> bookedOrders;

public:
    SalesRepresentative(const std::string& number, const std::string& name, const Money& hourlyRate, const PharmacyClient& client, int commissionBasisPoints);

    void bookSale(const SalesOrder& order);
    Money commission() const;

    std::string role() const override;
    Money payroll(int hours) const override;
};

class QualityInspector : public Employee
{
private:
    const License* license;
    int inspections;
    int rejected;

public:
    QualityInspector(const std::string& number, const std::string& name, const Money& hourlyRate, const License& license);

    bool inspect(ProductBatch& batch, const QualityCertificate& certificate, const Date& date);
    void releaseBatch(ProductBatch& batch, const Date& date);
    double rejectionRate() const;

    std::string role() const override;
};

class PharmaCompany
{
private:
    std::string name;
    License license;
    ProductCatalog catalog;
    SalesDepartment sales;
    std::vector<Employee*> employees;
    std::vector<StorageZone*> zones;
    std::vector<SalesOrder*> orders;

public:
    PharmaCompany(const std::string& name, const License& license);

    ProductCatalog& getCatalog();
    SalesDepartment& getSales();

    void hire(Employee& employee);
    void attachZone(StorageZone& zone);
    void registerOrder(SalesOrder& order);
    void confirmOrder(SalesOrder& order, const Date& date);
    Money payrollCost(int hours) const;
    int quarantineExpired(const Date& date);
};

void runCompanyExample(std::ostream& os);

#endif
