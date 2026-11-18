#include "Company.h"

#include <algorithm>
#include <limits>
#include <utility>

Employee::Employee(const std::string& number, const std::string& name, const Money& hourlyRate) : number(number), name(name), hourlyRate(hourlyRate), active(true)
{
    if (number.empty() || name.empty() || hourlyRate.getKopecks() <= 0)
        throw ValidationException("Invalid employee");
}

Employee::~Employee() = default;

const std::string& Employee::getNumber() const
{
    return number;
}

bool Employee::isActive() const
{
    return active;
}

std::string Employee::role() const
{
    return "Сотрудник";
}

Money Employee::payroll(int hours) const
{
    if (!active || hours < 0 || hours > 744)
        throw PaymentException("Invalid work hours or employee is dismissed");
    return hourlyRate.multiply(hours);
}

void Employee::dismiss()
{
    if (!active)
        throw ValidationException("Employee is already dismissed");
    active = false;
}

SalesRepresentative::SalesRepresentative(const std::string& number, const std::string& name, const Money& hourlyRate, const PharmacyClient& client, int commissionBasisPoints) : Employee(number, name, hourlyRate), client(&client), commissionBasisPoints(commissionBasisPoints), salesVolume(0, hourlyRate.getCurrency())
{
    if (commissionBasisPoints < 0 || commissionBasisPoints > 10000)
        throw ValidationException("Invalid commission rate");
}

void SalesRepresentative::bookSale(const SalesOrder& order)
{
    if (!isActive() || order.getState() != SalesOrder::State::Shipped || order.getClient().getTaxId() != client->getTaxId())
        throw OrderException("Sale cannot be credited to this representative");
    if (std::find(bookedOrders.begin(), bookedOrders.end(), &order) != bookedOrders.end())
        throw DuplicateException("Order has already been credited");
    Money updated = salesVolume.add(order.total());
    bookedOrders.push_back(&order);
    salesVolume = std::move(updated);
}

Money SalesRepresentative::commission() const
{
    return salesVolume.discounted(10000 - commissionBasisPoints);
}

std::string SalesRepresentative::role() const
{
    return "Торговый представитель";
}

Money SalesRepresentative::payroll(int hours) const
{
    return Employee::payroll(hours).add(commission());
}

QualityInspector::QualityInspector(const std::string& number, const std::string& name, const Money& hourlyRate, const License& license) : Employee(number, name, hourlyRate), license(&license), inspections(0), rejected(0)
{
}

bool QualityInspector::inspect(ProductBatch& batch, const QualityCertificate& certificate, const Date& date)
{
    if (!isActive() || inspections == std::numeric_limits<int>::max())
        throw QualityException("Inspector is unavailable");
    license->requireValid("quality", date);
    inspections++;
    if (certificate.covers(batch, date)) return true;
    batch.quarantine();
    rejected++;
    return false;
}

void QualityInspector::releaseBatch(ProductBatch& batch, const Date& date)
{
    if (!isActive())
        throw QualityException("Inspector is dismissed");
    license->requireValid("quality", date);
    if (batch.isExpired(date))
        throw ExpiredProductException("Expired batch cannot be released");
    batch.releaseQuarantine();
}

double QualityInspector::rejectionRate() const
{
    return inspections == 0 ? 0.0 : static_cast<double>(rejected) / inspections;
}

std::string QualityInspector::role() const
{
    return "Инспектор качества";
}

PharmaCompany::PharmaCompany(const std::string& name, const License& license) : name(name), license(license), catalog(name)
{
    if (name.empty())
        throw ValidationException("Company name is required");
}

ProductCatalog& PharmaCompany::getCatalog()
{
    return catalog;
}

SalesDepartment& PharmaCompany::getSales()
{
    return sales;
}

void PharmaCompany::hire(Employee& employee)
{
    if (!employee.isActive())
        throw ValidationException("Cannot hire a dismissed employee");
    for (const Employee* existing : employees)
        if (existing->getNumber() == employee.getNumber())
            throw DuplicateException("Employee number is already registered");
    employees.push_back(&employee);
}

void PharmaCompany::attachZone(StorageZone& zone)
{
    if (std::find(zones.begin(), zones.end(), &zone) != zones.end())
        throw DuplicateException("Zone is already attached");
    zones.push_back(&zone);
}

void PharmaCompany::registerOrder(SalesOrder& order)
{
    if (order.getState() != SalesOrder::State::Draft)
        throw OrderException("Only draft orders can be registered");
    for (const SalesOrder* existing : orders)
        if (existing->getNumber() == order.getNumber())
            throw DuplicateException("Order number is already registered");
    orders.push_back(&order);
}

void PharmaCompany::confirmOrder(SalesOrder& order, const Date& date)
{
    if (std::find(orders.begin(), orders.end(), &order) == orders.end())
        throw OrderException("Order is not registered with the company");
    order.confirm(license, date);
}

Money PharmaCompany::payrollCost(int hours) const
{
    Money result;
    for (const Employee* employee : employees)
        if (employee->isActive()) result = result.add(employee->payroll(hours));
    return result;
}

int PharmaCompany::quarantineExpired(const Date& date)
{
    int count = 0;
    for (StorageZone* zone : zones)
        for (ProductBatch* batch : zone->getBatches())
            if (batch->isExpired(date))
            {
                batch->quarantine();
                count++;
            }
    return count;
}

void runCompanyExample(std::ostream& os)
{
    const Date today(2026, 10, 29);
    const Date expiry(2027, 10, 29);
    const License wholesale("W-1", "wholesale", expiry);
    const License quality("Q-1", "quality", expiry);
    const License driving("D-1", "driver", expiry);
    PharmaCompany company("ФармСбыт", wholesale);
    const PharmacyClient client("Аптека Здоровье", "123456789", "L-1", 0.0);
    const auto product = std::make_shared<OverTheCounterDrug>("DEMO-1", "Учебный препарат", "Фармзавод", 10.0, "вещество", 100.0, 0.0);
    company.getCatalog().add(product);
    company.getCatalog().publish();

    ProductBatch batch(product, "B-1", expiry, 100);
    StorageZone zone("Основная", StorageCondition(15.0, 25.0, 70.0, true), 500);
    zone.receive(batch, today, 20.0, 50.0);
    company.attachZone(zone);
    QualityCertificate certificate(batch, quality, today, expiry);
    certificate.approve();
    QualityInspector inspector("E-1", "Анна", Money(800), quality);
    company.hire(inspector);
    os << "Контроль качества: " << (inspector.inspect(batch, certificate, today) ? "пройден" : "карантин") << '\n';

    SalesOrder order("SO-1", client, today);
    order.addLine(OrderLine(*product, 10, Money(1100), 500));
    company.registerOrder(order);
    company.confirmOrder(order, today);
    Invoice invoice("INV-1", order, today.addDays(7));
    BankAccount buyer("BY-BUYER", client.getName(), Money(50000));
    BankAccount seller("BY-SELLER", "ФармСбыт", Money(0));
    Payment payment("PAY-1", invoice, buyer, seller, order.total());
    payment.execute();
    os << "Заказ оплачен: " << (invoice.isSettled() ? "да" : "нет") << ", сумма " << order.total().getKopecks() << " коп.\n";

    Reservation reservation("RES-1", batch, 10, today.addDays(1));
    reservation.activate(today);
    const Address origin("Минск", "Складская", "1", "220001");
    const Address destination("Минск", "Ленина", "2", "220002");
    const DeliveryRoute route(origin, destination, 30.0, 30.0);
    Vehicle vehicle("1234 AB-7", 100, 20.0);
    Driver driver("Иван", driving, 8);
    Shipment shipment("SH-1", order, vehicle, driver, route, today);
    shipment.addReservation(reservation);
    shipment.dispatch(today);
    DeliveryNote note("DN-1", shipment, destination, Contact("Мария", "+375291234567", "pharmacy@example.com"));
    note.sign("Мария", today);
    os << "Доставка: " << note.proof() << '\n';
    os << "Остаток партии: " << batch.getQuantity() << " шт.\n";
}
