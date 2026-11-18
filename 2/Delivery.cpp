#include "Delivery.h"

#include <cmath>
#include <limits>
#include <utility>

DeliveryRoute::DeliveryRoute(const Address& origin, const Address& destination, double distanceKm, double speedKmPerHour) : origin(origin), destination(destination), distanceKm(distanceKm), speedKmPerHour(speedKmPerHour)
{
    if (!std::isfinite(distanceKm) || !std::isfinite(speedKmPerHour) || distanceKm <= 0.0 || speedKmPerHour <= 0.0 || distanceKm > std::numeric_limits<int>::max() || distanceKm / speedKmPerHour > std::numeric_limits<int>::max())
        throw DeliveryException("Invalid route distance or speed");
}

int DeliveryRoute::estimatedHours() const
{
    return static_cast<int>(std::ceil(distanceKm / speedKmPerHour));
}

Money DeliveryRoute::transportCost(const Money& pricePerKm) const
{
    return pricePerKm.multiply(static_cast<int>(std::ceil(distanceKm)));
}

bool DeliveryRoute::isLocal() const
{
    return origin.sameCity(destination);
}

Vehicle::Vehicle(const std::string& registration, int capacity, double temperature) : registration(registration), capacity(capacity), temperature(temperature), busy(false)
{
    if (registration.empty() || capacity <= 0 || !std::isfinite(temperature))
        throw DeliveryException("Invalid vehicle");
}

bool Vehicle::canCarry(const Product& product, int quantity) const
{
    return !busy && quantity > 0 && quantity <= capacity && product.canBeStoredAt(temperature);
}

void Vehicle::assign(int quantity)
{
    if (busy || quantity <= 0 || quantity > capacity)
        throw DeliveryException("Vehicle is busy or overloaded");
    busy = true;
}

void Vehicle::release()
{
    if (!busy)
        throw DeliveryException("Vehicle is not assigned");
    busy = false;
}

Driver::Driver(const std::string& name, const License& license, int maximumHours) : name(name), license(&license), maximumHours(maximumHours), workedHours(0), busy(false)
{
    if (name.empty() || maximumHours <= 0 || maximumHours > 24)
        throw DeliveryException("Invalid driver or workday limit");
}

bool Driver::canDrive(const Date& date, int hours) const
{
    return !busy && hours > 0 && hours <= maximumHours - workedHours && license->permits("driver", date);
}

void Driver::beginTrip(const Date& date, int hours)
{
    if (!canDrive(date, hours))
        throw DeliveryException("Driver is unavailable or workday limit exceeded");
    workedHours += hours;
    busy = true;
}

void Driver::finishTrip()
{
    if (!busy)
        throw DeliveryException("Driver has no active trip");
    busy = false;
}

void Driver::rest()
{
    if (busy)
        throw DeliveryException("Driver cannot rest during a trip");
    workedHours = 0;
}

Shipment::Shipment(const std::string& number, SalesOrder& order, Vehicle& vehicle, Driver& driver, const DeliveryRoute& route, const Date& departure) : number(number), order(&order), vehicle(&vehicle), driver(&driver), route(route), departure(departure), state(State::Planned)
{
    if (number.empty())
        throw DeliveryException("Shipment number is required");
}

Shipment::State Shipment::getState() const
{
    return state;
}

void Shipment::addReservation(Reservation& reservation)
{
    if (state != State::Planned || reservation.getState() != Reservation::State::Active || order->quantityOf(reservation.getBatch().getProduct().getSku()) == 0)
        throw DeliveryException("Reservation cannot be added to this shipment");
    for (const Reservation* existing : reservations)
        if (existing == &reservation)
            throw DuplicateException("Reservation is already in the shipment");
    reservations.push_back(&reservation);
}

void Shipment::dispatch(const Date& date)
{
    if (state != State::Planned || order->getState() != SalesOrder::State::Confirmed || date.isBefore(departure) || reservations.empty() || !driver->canDrive(date, route.estimatedHours()))
        throw DeliveryException("Shipment cannot depart");

    std::map<std::string, int> quantities;
    std::map<const ProductBatch*, int> batches;
    int total = 0;
    for (const Reservation* reservation : reservations)
    {
        const ProductBatch& batch = reservation->getBatch();
        const int count = reservation->getQuantity();
        if (!reservation->canShip(date) || count > std::numeric_limits<int>::max() - total)
            throw DeliveryException("Reservation is unavailable or shipment is too large");
        total += count;
        quantities[batch.getProduct().getSku()] += count;
        batches[&batch] += count;
    }
    for (const auto& [batch, count] : batches)
        if (count > batch->getQuantity() - batch->available() || !vehicle->canCarry(batch->getProduct(), total))
            throw DeliveryException("Batch reservation or transport conditions do not cover the shipment");
    for (const OrderLine& line : order->getLines())
        if (quantities[line.getProduct().getSku()] != line.getQuantity() || !vehicle->canCarry(line.getProduct(), total))
            throw DeliveryException("Shipment does not match the order or vehicle conditions");

    vehicle->assign(total);
    driver->beginTrip(date, route.estimatedHours());
    for (Reservation* reservation : reservations) reservation->ship(date);
    order->markShipped();
    departure = date;
    state = State::InTransit;
}

void Shipment::complete(const Date& date)
{
    if (state != State::InTransit || date.isBefore(departure))
        throw DeliveryException("Shipment cannot be completed");
    vehicle->release();
    driver->finishTrip();
    state = State::Delivered;
}

bool Shipment::isLate(const Date& date) const
{
    const int days = (route.estimatedHours() - 1) / 24 + 1;
    return state == State::InTransit && departure.daysUntil(date) > days;
}

DeliveryNote::DeliveryNote(const std::string& number, Shipment& shipment, const Address& destination, const Contact& recipient) : number(number), shipment(&shipment), destination(destination), recipient(recipient), signedByRecipient(false)
{
    if (number.empty())
        throw DeliveryException("Delivery note number is required");
}

void DeliveryNote::sign(const std::string& signature, const Date& date)
{
    if (signedByRecipient || signature.empty())
        throw DeliveryException("Invalid or duplicate delivery signature");
    std::string savedSignature = signature;
    shipment->complete(date);
    this->signature = std::move(savedSignature);
    signedByRecipient = true;
}

bool DeliveryNote::matchesDestination(const Address& address) const
{
    return destination.format() == address.format();
}

std::string DeliveryNote::proof() const
{
    if (!signedByRecipient)
        throw DeliveryException("Delivery note is unsigned");
    return number + ": " + recipient.mailingLabel() + ", " + destination.format() + ", " + signature;
}

InsurancePolicy::InsurancePolicy(const std::string& number, const Shipment& shipment, const Money& coverage, int deductibleBasisPoints, const Date& validUntil) : number(number), shipment(&shipment), coverage(coverage), deductibleBasisPoints(deductibleBasisPoints), validUntil(validUntil), claimed(false)
{
    if (number.empty() || coverage.getKopecks() <= 0 || deductibleBasisPoints < 0 || deductibleBasisPoints > 10000)
        throw ValidationException("Invalid insurance policy");
}

bool InsurancePolicy::covers(const Money& loss, const Date& date) const
{
    return !claimed && shipment->getState() != Shipment::State::Planned && !validUntil.isBefore(date) && loss.getKopecks() > 0 && coverage.covers(loss);
}

Money InsurancePolicy::estimateCompensation(const Money& loss) const
{
    if (!coverage.covers(loss) || loss.getKopecks() <= 0)
        throw PaymentException("Loss exceeds insurance coverage");
    return loss.discounted(deductibleBasisPoints);
}

Money InsurancePolicy::claim(const Money& loss, const Date& date)
{
    if (!covers(loss, date))
        throw PaymentException("Insurance claim is not covered");
    Money result = estimateCompensation(loss);
    claimed = true;
    return result;
}
