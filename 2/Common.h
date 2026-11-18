#ifndef COMMON_H
#define COMMON_H

#include "Exceptions.h"

#include <cstdint>
#include <string>

class Address
{
private:
    std::string city;
    std::string street;
    std::string building;
    std::string postalCode;

public:
    Address(const std::string& city, const std::string& street, const std::string& building, const std::string& postalCode);

    std::string format() const;
    bool sameCity(const Address& other) const;
    bool canDeliverTo(const std::string& servicedPostalPrefix) const;
};

class Contact
{
private:
    std::string person;
    std::string phone;
    std::string email;

public:
    Contact(const std::string& person, const std::string& phone, const std::string& email);

    bool matchesPhone(const std::string& number) const;
    bool belongsToDomain(const std::string& domain) const;
    std::string mailingLabel() const;
};

class Date
{
private:
    int year;
    int month;
    int day;

    int serial() const;

public:
    Date(int year, int month, int day);

    bool isBefore(const Date& other) const;
    int daysUntil(const Date& other) const;
    Date addDays(int count) const;
    std::string format() const;
};

class Money
{
private:
    std::int64_t kopecks;
    std::string currency;

    void requireSameCurrency(const Money& other) const;

public:
    explicit Money(std::int64_t kopecks = 0, const std::string& currency = "BYN");

    std::int64_t getKopecks() const;
    const std::string& getCurrency() const;

    Money add(const Money& other) const;
    Money subtract(const Money& other) const;
    Money multiply(int quantity) const;
    Money discounted(int basisPoints) const;
    bool covers(const Money& other) const;
};

class License
{
private:
    std::string number;
    std::string activity;
    Date validUntil;
    bool revoked;

public:
    License(const std::string& number, const std::string& activity, const Date& validUntil);

    bool permits(const std::string& requestedActivity, const Date& date) const;
    void requireValid(const std::string& requestedActivity, const Date& date) const;
    void revoke();
};

class BankAccount
{
private:
    std::string number;
    std::string owner;
    Money funds;
    bool blocked;

public:
    BankAccount(const std::string& number, const std::string& owner, const Money& openingBalance);

    const Money& getBalance() const;

    void deposit(const Money& amount);
    void withdraw(const Money& amount);
    bool canTransferTo(const BankAccount& destination, const Money& amount) const;
    void transferTo(BankAccount& destination, const Money& amount);
    void block();
};

#endif
