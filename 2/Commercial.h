#ifndef COMMERCIAL_H
#define COMMERCIAL_H

#include "Common.h"
#include "Organization.h"
#include "Product.h"

#include <map>
#include <vector>

class Contract
{
private:
    const Supplier* supplier;
    const PharmacyClient* client;
    const License* license;
    Date validUntil;
    int discountBasisPoints;
    bool active;

public:
    Contract(const Supplier& supplier, const PharmacyClient& client, const License& license, const Date& validUntil, int discountBasisPoints);

    bool permitsSaleTo(const PharmacyClient& candidate, const Date& date) const;
    Money calculateTotal(const Money& unitPrice, int quantity, const Date& date) const;
    void terminate();
};

class DiscountPolicy
{
private:
    int minimumQuantity;
    int discountBasisPoints;
    int maximumBasisPoints;
    Date validUntil;

public:
    DiscountPolicy(int minimumQuantity, int discountBasisPoints, int maximumBasisPoints, const Date& validUntil);

    bool appliesTo(int quantity, const Date& date) const;
    Money apply(const Money& total, int quantity, const Date& date) const;
    int combineWith(int clientBasisPoints, int quantity, const Date& date) const;
};

class CreditAccount
{
private:
    const PharmacyClient* client;
    Money limit;
    Money debt;
    bool frozen;

public:
    CreditAccount(const PharmacyClient& client, const Money& limit);

    const Money& getDebt() const;

    bool canCharge(const Money& amount) const;
    void charge(const Money& amount);
    void repay(const Money& amount);
    void freeze();
};

class SupplierBid
{
private:
    const Supplier* supplier;
    const Product* product;
    Money unitPrice;
    int quantity;
    Date validUntil;

public:
    SupplierBid(const Supplier& supplier, const Product& product, const Money& unitPrice, int quantity, const Date& validUntil);

    const Supplier& getSupplier() const;
    const Product& getProduct() const;
    int getQuantity() const;

    Money total() const;
    bool isAvailable(const Date& date) const;
    bool beats(const SupplierBid& other) const;
};

class Tender
{
private:
    std::string number;
    const Product* product;
    int requiredQuantity;
    Money budget;
    Date deadline;
    std::vector<SupplierBid> bids;
    bool closed;

public:
    Tender(const std::string& number, const Product& product, int requiredQuantity, const Money& budget, const Date& deadline);

    void submit(const SupplierBid& bid, const Date& date);
    SupplierBid selectWinner(const Date& date);
    void cancel();
};

class PriceList
{
private:
    std::string name;
    std::string currency;
    std::map<std::string, Money> prices;
    bool active;

public:
    PriceList(const std::string& name, const std::string& currency);

    void addPrice(const Product& product, const Money& price);
    Money quote(const Product& product, int quantity) const;
    void activate();
    void retire();
};

#endif
