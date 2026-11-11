#ifndef FINANCE_H
#define FINANCE_H
#include "Content.h"

namespace learning
{

class Coupon
{
  private:
    std::string code;
    int percent;
    int uses;

  public:
    Coupon(std::string code = {}, int percent = 0, int uses = 0);
    const std::string &getCode() const;
    int getPercent() const;
    int getUses() const;
    Money discount(Money amount) const;
    void redeem();
};

class Payment
{
  private:
    std::shared_ptr<Student> payer;
    Money amount;
    bool paid;
    bool allocated;

  public:
    Payment(std::shared_ptr<Student> payer = {}, Money amount = 0, bool paid = false, bool allocated = false);
    const std::shared_ptr<Student> &getPayer() const;
    Money getAmount() const;
    bool getPaid() const;
    bool getAllocated() const;
    void capture();
    bool covers(Money total) const;
    void allocate();
    void reverse();
};

class Invoice
{
  private:
    std::shared_ptr<Payment> payment;
    std::shared_ptr<Course> course;
    Money total;
    bool calculated;

  public:
    Invoice(std::shared_ptr<Payment> payment = {}, std::shared_ptr<Course> course = {}, Money total = 0,
            bool calculated = false);
    const std::shared_ptr<Payment> &getPayment() const;
    const std::shared_ptr<Course> &getCourse() const;
    Money getTotal() const;
    bool getCalculated() const;
    void calculate(Coupon *coupon = nullptr);
    void settle(std::shared_ptr<Payment> value);
};

class Subscription
{
  private:
    std::shared_ptr<Student> student;
    int expires;
    bool cancelled;

  public:
    Subscription(std::shared_ptr<Student> student = {}, int expires = 0, bool cancelled = false);
    const std::shared_ptr<Student> &getStudent() const;
    int getExpires() const;
    bool getCancelled() const;
    void renew(int day, int days);
    bool active(int day) const;
};

class Refund
{
  private:
    std::shared_ptr<Payment> payment;
    std::string reason;
    bool processed;

  public:
    Refund(std::shared_ptr<Payment> payment = {}, std::string reason = {}, bool processed = false);
    const std::shared_ptr<Payment> &getPayment() const;
    const std::string &getReason() const;
    bool getProcessed() const;
    void process();
    Money amount() const;
};
}
#endif
