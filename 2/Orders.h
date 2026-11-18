#ifndef ORDERS_H
#define ORDERS_H

#include "Catalog.h"
#include "Commercial.h"
#include "Warehouse.h"

class OrderLine
{
private:
    const Product* product;
    int quantity;
    Money unitPrice;
    int discountBasisPoints;

public:
    OrderLine(const Product& product, int quantity, const Money& unitPrice, int discountBasisPoints = 0);

    const Product& getProduct() const;
    int getQuantity() const;

    Money total() const;
    Money refundFor(int count) const;
    void increase(int count);
    void reduce(int count);
};

class SalesOrder
{
public:
    enum class State { Draft, Confirmed, Shipped, Cancelled };

private:
    std::string number;
    const PharmacyClient* client;
    Date created;
    std::vector<OrderLine> lines;
    std::map<std::string, int> returned;
    State state;
    std::string currency;

public:
    SalesOrder(const std::string& number, const PharmacyClient& client, const Date& created, const std::string& currency = "BYN");

    State getState() const;
    const std::string& getNumber() const;
    const std::vector<OrderLine>& getLines() const;
    const PharmacyClient& getClient() const;

    void addLine(const OrderLine& line);
    Money total() const;
    int quantityOf(const std::string& sku) const;
    void confirm(const License& license, const Date& date);
    void cancel();
    void markShipped();
    void recordReturn(const Product& product, int quantity);
};

class PurchaseOrder
{
public:
    enum class State { Pending, Received, Cancelled };

private:
    std::string number;
    const Supplier* supplier;
    Warehouse* warehouse;
    std::shared_ptr<const Product> product;
    int quantity;
    Date expected;
    State state;

public:
    PurchaseOrder(const std::string& number, const Supplier& supplier, Warehouse& warehouse, const std::shared_ptr<const Product>& product, int quantity, const Date& expected);

    State getState() const;

    void receive();
    void cancel();
    bool isOverdue(const Date& date) const;
};

class Invoice
{
private:
    std::string number;
    const SalesOrder* order;
    Money amount;
    Money paid;
    Date due;

public:
    Invoice(const std::string& number, const SalesOrder& order, const Date& due);

    Money outstanding() const;
    bool canApply(const Money& payment) const;
    void applyPayment(const Money& payment);
    bool isOverdue(const Date& date) const;
    bool isSettled() const;
};

class Payment
{
private:
    std::string number;
    Invoice* invoice;
    BankAccount* payer;
    BankAccount* recipient;
    Money amount;
    bool executed;

public:
    Payment(const std::string& number, Invoice& invoice, BankAccount& payer, BankAccount& recipient, const Money& amount);

    bool canExecute() const;
    void execute();
    std::string receipt() const;
};

class ReturnRequest
{
public:
    enum class State { Pending, Approved, Rejected };

private:
    std::string number;
    SalesOrder* order;
    ProductBatch* batch;
    int quantity;
    std::string reason;
    State state;

public:
    ReturnRequest(const std::string& number, SalesOrder& order, ProductBatch& batch, int quantity, const std::string& reason);

    State getState() const;

    void approve();
    void reject();
    Money refundableAmount() const;
};

#endif
