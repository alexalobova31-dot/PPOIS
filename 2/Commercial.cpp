#include "Commercial.h"

#include <algorithm>

Contract::Contract(const Supplier& supplier, const PharmacyClient& client, const License& license, const Date& validUntil, int discountBasisPoints) : supplier(&supplier), client(&client), license(&license), validUntil(validUntil), discountBasisPoints(discountBasisPoints), active(true)
{
    if (discountBasisPoints < 0 || discountBasisPoints > 3000 || supplier.getTaxId() == client.getTaxId())
        throw ValidationException("Invalid contract parties or discount");
}

bool Contract::permitsSaleTo(const PharmacyClient& candidate, const Date& date) const
{
    return active && candidate.getTaxId() == client->getTaxId() && supplier->getTaxId() != candidate.getTaxId() && !validUntil.isBefore(date) && license->permits("wholesale", date);
}

Money Contract::calculateTotal(const Money& unitPrice, int quantity, const Date& date) const
{
    if (!permitsSaleTo(*client, date) || quantity <= 0)
        throw OrderException("Contract does not permit the sale");
    return unitPrice.multiply(quantity).discounted(discountBasisPoints);
}

void Contract::terminate()
{
    if (!active)
        throw OrderException("Contract is already terminated");
    active = false;
}

DiscountPolicy::DiscountPolicy(int minimumQuantity, int discountBasisPoints, int maximumBasisPoints, const Date& validUntil) : minimumQuantity(minimumQuantity), discountBasisPoints(discountBasisPoints), maximumBasisPoints(maximumBasisPoints), validUntil(validUntil)
{
    if (minimumQuantity <= 0 || discountBasisPoints < 0 || maximumBasisPoints < discountBasisPoints || maximumBasisPoints > 10000)
        throw ValidationException("Invalid discount policy");
}

bool DiscountPolicy::appliesTo(int quantity, const Date& date) const
{
    return quantity >= minimumQuantity && !validUntil.isBefore(date);
}

Money DiscountPolicy::apply(const Money& total, int quantity, const Date& date) const
{
    if (quantity <= 0)
        throw ValidationException("Quantity must be positive");
    return total.discounted(appliesTo(quantity, date) ? discountBasisPoints : 0);
}

int DiscountPolicy::combineWith(int clientBasisPoints, int quantity, const Date& date) const
{
    if (clientBasisPoints < 0 || clientBasisPoints > 10000 || quantity <= 0)
        throw ValidationException("Invalid client discount or quantity");
    return std::min(maximumBasisPoints, clientBasisPoints + (appliesTo(quantity, date) ? discountBasisPoints : 0));
}

CreditAccount::CreditAccount(const PharmacyClient& client, const Money& limit) : client(&client), limit(limit), debt(0, limit.getCurrency()), frozen(false)
{
}

const Money& CreditAccount::getDebt() const
{
    return debt;
}

bool CreditAccount::canCharge(const Money& amount) const
{
    return !frozen && !client->getLicenseNumber().empty() && amount.getKopecks() > 0 && limit.subtract(debt).covers(amount);
}

void CreditAccount::charge(const Money& amount)
{
    if (!canCharge(amount))
        throw CreditLimitException("Credit limit exceeded or account is frozen");
    debt = debt.add(amount);
}

void CreditAccount::repay(const Money& amount)
{
    if (amount.getKopecks() <= 0)
        throw PaymentException("Repayment must be positive");
    debt = debt.subtract(amount);
}

void CreditAccount::freeze()
{
    frozen = true;
}

SupplierBid::SupplierBid(const Supplier& supplier, const Product& product, const Money& unitPrice, int quantity, const Date& validUntil) : supplier(&supplier), product(&product), unitPrice(unitPrice), quantity(quantity), validUntil(validUntil)
{
    if (quantity <= 0 || unitPrice.getKopecks() <= 0)
        throw ValidationException("Invalid supplier bid");
}

const Supplier& SupplierBid::getSupplier() const
{
    return *supplier;
}

const Product& SupplierBid::getProduct() const
{
    return *product;
}

int SupplierBid::getQuantity() const
{
    return quantity;
}

Money SupplierBid::total() const
{
    return unitPrice.multiply(quantity);
}

bool SupplierBid::isAvailable(const Date& date) const
{
    return !validUntil.isBefore(date);
}

bool SupplierBid::beats(const SupplierBid& other) const
{
    if (product->getSku() != other.product->getSku() || quantity != other.quantity)
        throw OrderException("Bids refer to different products or quantities");
    return other.total().covers(total()) && !total().covers(other.total());
}

Tender::Tender(const std::string& number, const Product& product, int requiredQuantity, const Money& budget, const Date& deadline) : number(number), product(&product), requiredQuantity(requiredQuantity), budget(budget), deadline(deadline), closed(false)
{
    if (number.empty() || requiredQuantity <= 0 || budget.getKopecks() <= 0)
        throw ValidationException("Invalid tender");
}

void Tender::submit(const SupplierBid& bid, const Date& date)
{
    if (closed || deadline.isBefore(date) || !bid.isAvailable(date) || bid.getProduct().getSku() != product->getSku() || bid.getQuantity() != requiredQuantity || !budget.covers(bid.total()))
        throw OrderException("Bid does not satisfy tender conditions");
    for (const SupplierBid& existing : bids)
        if (existing.getSupplier().getTaxId() == bid.getSupplier().getTaxId())
            throw DuplicateException("Supplier has already submitted a bid");
    bids.push_back(bid);
}

SupplierBid Tender::selectWinner(const Date& date)
{
    if (closed || date.isBefore(deadline))
        throw OrderException("Tender cannot be closed yet");
    const SupplierBid* best = nullptr;
    for (const SupplierBid& bid : bids)
        if (bid.isAvailable(date) && (best == nullptr || bid.beats(*best))) best = &bid;
    if (best == nullptr)
        throw OrderException("Tender has no valid bids");
    SupplierBid result = *best;
    closed = true;
    return result;
}

void Tender::cancel()
{
    if (closed)
        throw OrderException("Tender is already closed");
    closed = true;
}

PriceList::PriceList(const std::string& name, const std::string& currency) : name(name), currency(currency), active(false)
{
    Money(0, currency);
    if (name.empty())
        throw ValidationException("Price list name is required");
}

void PriceList::addPrice(const Product& product, const Money& price)
{
    if (active || price.getKopecks() <= 0 || price.getCurrency() != currency)
        throw PaymentException("Cannot change an active price list or use an invalid price");
    if (!prices.emplace(product.getSku(), price).second)
        throw DuplicateException("Product price is already specified");
}

Money PriceList::quote(const Product& product, int quantity) const
{
    const auto it = prices.find(product.getSku());
    if (!active || quantity <= 0 || it == prices.end())
        throw OrderException("Price is unavailable");
    return it->second.multiply(quantity);
}

void PriceList::activate()
{
    if (active || prices.empty())
        throw OrderException("Price list is empty or already active");
    active = true;
}

void PriceList::retire()
{
    active = false;
}
