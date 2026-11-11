#include "Common.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

namespace
{
    bool isLeapYear(int year)
    {
        return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
    }

    int daysInMonth(int year, int month)
    {
        const int lengths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        return lengths[month - 1] + (month == 2 && isLeapYear(year) ? 1 : 0);
    }

    std::string phoneDigits(const std::string& phone)
    {
        std::string result;
        for (unsigned char symbol : phone)
            if (std::isdigit(symbol)) result += static_cast<char>(symbol);
        return result;
    }
}

Address::Address(const std::string& city, const std::string& street, const std::string& building, const std::string& postalCode) : city(city), street(street), building(building), postalCode(postalCode)
{
    if (city.empty() || street.empty() || building.empty() || postalCode.empty())
        throw ValidationException("Address must be complete");
}

std::string Address::format() const
{
    return postalCode + ", " + city + ", " + street + ", " + building;
}

bool Address::sameCity(const Address& other) const
{
    return city == other.city;
}

bool Address::canDeliverTo(const std::string& servicedPostalPrefix) const
{
    return !servicedPostalPrefix.empty() && postalCode.compare(0, servicedPostalPrefix.size(), servicedPostalPrefix) == 0;
}

Contact::Contact(const std::string& person, const std::string& phone, const std::string& email) : person(person), phone(phone), email(email)
{
    const std::size_t separator = email.find('@');
    if (person.empty() || phoneDigits(phone).size() < 7 || separator == std::string::npos || separator == 0 || separator + 1 == email.size() || email.find('@', separator + 1) != std::string::npos)
        throw ValidationException("Invalid contact details");
}

bool Contact::matchesPhone(const std::string& number) const
{
    return phoneDigits(phone) == phoneDigits(number);
}

bool Contact::belongsToDomain(const std::string& domain) const
{
    return !domain.empty() && email.substr(email.find('@') + 1) == domain;
}

std::string Contact::mailingLabel() const
{
    return person + " <" + email + ">";
}

Date::Date(int year, int month, int day) : year(year), month(month), day(day)
{
    if (year < 1 || year > 9999 || month < 1 || month > 12 || day < 1 || day > daysInMonth(year, month))
        throw DateException("Invalid calendar date");
}

int Date::serial() const
{
    const int previous = year - 1;
    int result = previous * 365 + previous / 4 - previous / 100 + previous / 400 + day;
    for (int current = 1; current < month; current++) result += daysInMonth(year, current);
    return result;
}

bool Date::isBefore(const Date& other) const
{
    return serial() < other.serial();
}

int Date::daysUntil(const Date& other) const
{
    return other.serial() - serial();
}

Date Date::addDays(int count) const
{
    const std::int64_t target = static_cast<std::int64_t>(serial()) + count;
    if (target < 1 || target > Date(9999, 12, 31).serial())
        throw DateException("Date is outside supported range");

    int targetYear = 1;
    int remaining = static_cast<int>(target);
    while (remaining > (isLeapYear(targetYear) ? 366 : 365))
    {
        remaining -= isLeapYear(targetYear) ? 366 : 365;
        targetYear++;
    }
    int targetMonth = 1;
    while (remaining > daysInMonth(targetYear, targetMonth))
    {
        remaining -= daysInMonth(targetYear, targetMonth);
        targetMonth++;
    }
    return Date(targetYear, targetMonth, remaining);
}

std::string Date::format() const
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(4) << year << '-' << std::setw(2) << month << '-' << std::setw(2) << day;
    return stream.str();
}

Money::Money(std::int64_t kopecks, const std::string& currency) : kopecks(kopecks), currency(currency)
{
    if (kopecks < 0 || currency.size() != 3 || !std::all_of(currency.begin(), currency.end(), [](unsigned char symbol) { return symbol >= 'A' && symbol <= 'Z'; }))
        throw PaymentException("Money requires a nonnegative amount and a three-letter currency");
}

void Money::requireSameCurrency(const Money& other) const
{
    if (currency != other.currency)
        throw PaymentException("Currencies do not match");
}

std::int64_t Money::getKopecks() const
{
    return kopecks;
}

const std::string& Money::getCurrency() const
{
    return currency;
}

Money Money::add(const Money& other) const
{
    requireSameCurrency(other);
    if (other.kopecks > std::numeric_limits<std::int64_t>::max() - kopecks)
        throw PaymentException("Money overflow");
    return Money(kopecks + other.kopecks, currency);
}

Money Money::subtract(const Money& other) const
{
    requireSameCurrency(other);
    if (other.kopecks > kopecks)
        throw PaymentException("Insufficient funds");
    return Money(kopecks - other.kopecks, currency);
}

Money Money::multiply(int quantity) const
{
    if (quantity < 0 || (quantity > 0 && kopecks > std::numeric_limits<std::int64_t>::max() / quantity))
        throw PaymentException("Invalid money multiplier");
    return Money(kopecks * quantity, currency);
}

Money Money::discounted(int basisPoints) const
{
    if (basisPoints < 0 || basisPoints > 10000)
        throw PaymentException("Discount must be in range [0, 10000]");
    const int rate = 10000 - basisPoints;
    const std::int64_t whole = (kopecks / 10000) * rate;
    const std::int64_t fraction = (kopecks % 10000 * rate + 5000) / 10000;
    return Money(whole + fraction, currency);
}

bool Money::covers(const Money& other) const
{
    requireSameCurrency(other);
    return kopecks >= other.kopecks;
}

License::License(const std::string& number, const std::string& activity, const Date& validUntil) : number(number), activity(activity), validUntil(validUntil), revoked(false)
{
    if (number.empty() || activity.empty())
        throw LicenseException("License number and activity are required");
}

bool License::permits(const std::string& requestedActivity, const Date& date) const
{
    return !revoked && activity == requestedActivity && !validUntil.isBefore(date);
}

void License::requireValid(const std::string& requestedActivity, const Date& date) const
{
    if (!permits(requestedActivity, date))
        throw LicenseException("License does not permit this operation on the requested date");
}

void License::revoke()
{
    if (revoked)
        throw LicenseException("License is already revoked");
    revoked = true;
}

BankAccount::BankAccount(const std::string& number, const std::string& owner, const Money& openingBalance) : number(number), owner(owner), funds(openingBalance), blocked(false)
{
    if (number.empty() || owner.empty())
        throw PaymentException("Account number and owner are required");
}

const Money& BankAccount::getBalance() const
{
    return funds;
}

void BankAccount::deposit(const Money& amount)
{
    if (blocked || amount.getKopecks() == 0)
        throw PaymentException("Account is blocked or deposit is zero");
    funds = funds.add(amount);
}

void BankAccount::withdraw(const Money& amount)
{
    if (blocked || amount.getKopecks() == 0)
        throw PaymentException("Account is blocked or withdrawal is zero");
    funds = funds.subtract(amount);
}

bool BankAccount::canTransferTo(const BankAccount& destination, const Money& amount) const
{
    return this != &destination && !blocked && !destination.blocked && amount.getKopecks() > 0 && funds.getCurrency() == amount.getCurrency() && destination.funds.getCurrency() == amount.getCurrency() && funds.getKopecks() >= amount.getKopecks() && amount.getKopecks() <= std::numeric_limits<std::int64_t>::max() - destination.funds.getKopecks();
}

void BankAccount::transferTo(BankAccount& destination, const Money& amount)
{
    if (!canTransferTo(destination, amount))
        throw PaymentException("Transfer is not permitted");
    Money remaining = funds.subtract(amount);
    Money received = destination.funds.add(amount);
    funds = std::move(remaining);
    destination.funds = std::move(received);
}

void BankAccount::block()
{
    blocked = true;
}
