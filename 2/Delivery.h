#ifndef DELIVERY_H
#define DELIVERY_H

#include "Orders.h"
#include "StockManagement.h"

class DeliveryRoute
{
private:
    Address origin;
    Address destination;
    double distanceKm;
    double speedKmPerHour;

public:
    DeliveryRoute(const Address& origin, const Address& destination, double distanceKm, double speedKmPerHour);

    int estimatedHours() const;
    Money transportCost(const Money& pricePerKm) const;
    bool isLocal() const;
};

class Vehicle
{
private:
    std::string registration;
    int capacity;
    double temperature;
    bool busy;

public:
    Vehicle(const std::string& registration, int capacity, double temperature);

    bool canCarry(const Product& product, int quantity) const;
    void assign(int quantity);
    void release();
};

class Driver
{
private:
    std::string name;
    const License* license;
    int maximumHours;
    int workedHours;
    bool busy;

public:
    Driver(const std::string& name, const License& license, int maximumHours);

    bool canDrive(const Date& date, int hours) const;
    void beginTrip(const Date& date, int hours);
    void finishTrip();
    void rest();
};

class Shipment
{
public:
    enum class State { Planned, InTransit, Delivered };

private:
    std::string number;
    SalesOrder* order;
    Vehicle* vehicle;
    Driver* driver;
    DeliveryRoute route;
    Date departure;
    std::vector<Reservation*> reservations;
    State state;

public:
    Shipment(const std::string& number, SalesOrder& order, Vehicle& vehicle, Driver& driver, const DeliveryRoute& route, const Date& departure);

    State getState() const;

    void addReservation(Reservation& reservation);
    void dispatch(const Date& date);
    void complete(const Date& date);
    bool isLate(const Date& date) const;
};

class DeliveryNote
{
private:
    std::string number;
    Shipment* shipment;
    Address destination;
    Contact recipient;
    bool signedByRecipient;
    std::string signature;

public:
    DeliveryNote(const std::string& number, Shipment& shipment, const Address& destination, const Contact& recipient);

    void sign(const std::string& signature, const Date& date);
    bool matchesDestination(const Address& address) const;
    std::string proof() const;
};

class InsurancePolicy
{
private:
    std::string number;
    const Shipment* shipment;
    Money coverage;
    int deductibleBasisPoints;
    Date validUntil;
    bool claimed;

public:
    InsurancePolicy(const std::string& number, const Shipment& shipment, const Money& coverage, int deductibleBasisPoints, const Date& validUntil);

    bool covers(const Money& loss, const Date& date) const;
    Money estimateCompensation(const Money& loss) const;
    Money claim(const Money& loss, const Date& date);
};

#endif
