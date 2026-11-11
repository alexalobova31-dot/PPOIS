#include "Finance.h"

namespace learning
{

Coupon::Coupon(std::string code, int percent, int uses)
    : code(std::move(code)), percent(std::move(percent)), uses(std::move(uses))
{
    require(percent >= 0, "Negative percent");
    require(uses >= 0, "Negative uses");
}

const std::string &Coupon::getCode() const
{
    return code;
}

int Coupon::getPercent() const
{
    return percent;
}

int Coupon::getUses() const
{
    return uses;
}

Money Coupon::discount(Money amount) const
{
    money(amount);
    require(percent > 0 && percent <= 100 && uses > 0, "Coupon unavailable");
    nonempty(code);
    return amount * percent / 100;
}

void Coupon::redeem()
{
    discount(0);
    --uses;
}

Payment::Payment(std::shared_ptr<Student> payer, Money amount, bool paid, bool allocated)
    : payer(std::move(payer)), amount(std::move(amount)), paid(std::move(paid)), allocated(std::move(allocated))
{
    money(amount);
}

const std::shared_ptr<Student> &Payment::getPayer() const
{
    return payer;
}

Money Payment::getAmount() const
{
    return amount;
}

bool Payment::getPaid() const
{
    return paid;
}

bool Payment::getAllocated() const
{
    return allocated;
}

void Payment::capture()
{
    present(payer);
    present(payer->getUser());
    money(amount);
    if (paid || allocated || amount == 0 || !payer->getUser()->getActive())
        throw PaymentFailed("Payment cannot be captured");
    paid = true;
}

bool Payment::covers(Money total) const
{
    money(total);
    money(amount);
    return paid && amount >= total;
}

void Payment::allocate()
{
    if (!paid || allocated)
        throw PaymentFailed("Payment unavailable");
    allocated = true;
}

void Payment::reverse()
{
    if (!paid)
        throw PaymentFailed("Payment not refundable");
    paid = false;
}

Invoice::Invoice(std::shared_ptr<Payment> payment, std::shared_ptr<Course> course, Money total, bool calculated)
    : payment(std::move(payment)), course(std::move(course)), total(std::move(total)), calculated(std::move(calculated))
{
    money(total);
}

const std::shared_ptr<Payment> &Invoice::getPayment() const
{
    return payment;
}

const std::shared_ptr<Course> &Invoice::getCourse() const
{
    return course;
}

Money Invoice::getTotal() const
{
    return total;
}

bool Invoice::getCalculated() const
{
    return calculated;
}

void Invoice::calculate(Coupon *coupon)
{
    present(course);
    require(!payment, "Invoice already settled");
    money(course->getPrice());
    total = course->getPrice() - (coupon ? coupon->discount(course->getPrice()) : 0);
    calculated = true;
}

void Invoice::settle(std::shared_ptr<Payment> value)
{
    present(value);
    money(total);
    require(calculated && !payment, "Invoice uncalculated or already settled");
    if (value->getAllocated() || !value->covers(total))
        throw PaymentFailed("Insufficient payment");
    value->allocate();
    payment = std::move(value);
}

Subscription::Subscription(std::shared_ptr<Student> student, int expires, bool cancelled)
    : student(std::move(student)), expires(std::move(expires)), cancelled(std::move(cancelled))
{
    require(expires >= 0, "Negative expires");
}

const std::shared_ptr<Student> &Subscription::getStudent() const
{
    return student;
}

int Subscription::getExpires() const
{
    return expires;
}

bool Subscription::getCancelled() const
{
    return cancelled;
}

void Subscription::renew(int day, int days)
{
    require(day >= 0 && day <= 1000000 && expires >= 0 && days > 0 && days <= 365 &&
                std::max(day, expires) <= 1000000 - days,
            "Invalid subscription period");
    expires = std::max(day, expires) + days;
    cancelled = false;
}

bool Subscription::active(int day) const
{
    require(day >= 0, "Invalid day");
    return !cancelled && day < expires;
}

Refund::Refund(std::shared_ptr<Payment> payment, std::string reason, bool processed)
    : payment(std::move(payment)), reason(std::move(reason)), processed(std::move(processed))
{
}

const std::shared_ptr<Payment> &Refund::getPayment() const
{
    return payment;
}

const std::string &Refund::getReason() const
{
    return reason;
}

bool Refund::getProcessed() const
{
    return processed;
}

void Refund::process()
{
    present(payment);
    nonempty(reason);
    if (processed || !payment->getPaid())
        throw PaymentFailed("Payment is not refundable");
    payment->reverse();
    processed = true;
}

Money Refund::amount() const
{
    present(payment);
    money(payment->getAmount());
    return processed ? payment->getAmount() : 0;
}
}
