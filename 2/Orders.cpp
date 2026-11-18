#include "Orders.h"

#include <limits>

OrderLine::OrderLine(const Product& product, int quantity, const Money& unitPrice, int discountBasisPoints) : product(&product), quantity(quantity), unitPrice(unitPrice), discountBasisPoints(discountBasisPoints)
{
    if (quantity <= 0 || unitPrice.getKopecks() <= 0 || discountBasisPoints < 0 || discountBasisPoints > 10000)
        throw OrderException("Invalid order line");
    total();
}

const Product& OrderLine::getProduct() const
{
    return *product;
}

int OrderLine::getQuantity() const
{
    return quantity;
}

Money OrderLine::total() const
{
    return unitPrice.discounted(discountBasisPoints).multiply(quantity);
}

Money OrderLine::refundFor(int count) const
{
    if (count <= 0 || count > quantity)
        throw OrderException("Refund quantity exceeds the order line");
    return unitPrice.discounted(discountBasisPoints).multiply(count);
}

void OrderLine::increase(int count)
{
    if (count <= 0 || count > std::numeric_limits<int>::max() - quantity)
        throw OrderException("Invalid quantity increase");
    unitPrice.discounted(discountBasisPoints).multiply(quantity + count);
    quantity += count;
}

void OrderLine::reduce(int count)
{
    if (count <= 0 || count >= quantity)
        throw OrderException("Order line must retain a positive quantity");
    quantity -= count;
}

SalesOrder::SalesOrder(const std::string& number, const PharmacyClient& client, const Date& created, const std::string& currency) : number(number), client(&client), created(created), state(State::Draft), currency(currency)
{
    Money(0, currency);
    if (number.empty())
        throw OrderException("Order number is required");
}

SalesOrder::State SalesOrder::getState() const
{
    return state;
}

const std::string& SalesOrder::getNumber() const
{
    return number;
}

const std::vector<OrderLine>& SalesOrder::getLines() const
{
    return lines;
}

const PharmacyClient& SalesOrder::getClient() const
{
    return *client;
}

void SalesOrder::addLine(const OrderLine& line)
{
    if (state != State::Draft)
        throw OrderException("Only a draft order can be edited");
    if (quantityOf(line.getProduct().getSku()) != 0)
        throw DuplicateException("Product is already in the order");
    total().add(line.total());
    lines.push_back(line);
}

Money SalesOrder::total() const
{
    Money result(0, currency);
    for (const OrderLine& line : lines) result = result.add(line.total());
    return result;
}

int SalesOrder::quantityOf(const std::string& sku) const
{
    for (const OrderLine& line : lines)
        if (line.getProduct().getSku() == sku) return line.getQuantity();
    return 0;
}

void SalesOrder::confirm(const License& license, const Date& date)
{
    if (state != State::Draft || lines.empty() || date.isBefore(created))
        throw OrderException("Order cannot be confirmed");
    license.requireValid("wholesale", date);
    state = State::Confirmed;
}

void SalesOrder::cancel()
{
    if (state != State::Draft && state != State::Confirmed)
        throw OrderException("Order cannot be cancelled");
    state = State::Cancelled;
}

void SalesOrder::markShipped()
{
    if (state != State::Confirmed)
        throw OrderException("Only a confirmed order can be shipped");
    state = State::Shipped;
}

void SalesOrder::recordReturn(const Product& product, int quantity)
{
    const auto it = returned.find(product.getSku());
    const int previous = it == returned.end() ? 0 : it->second;
    if (state != State::Shipped || quantity <= 0 || quantity > quantityOf(product.getSku()) - previous)
        throw OrderException("Total returns exceed the shipped quantity");
    returned[product.getSku()] = previous + quantity;
}

PurchaseOrder::PurchaseOrder(const std::string& number, const Supplier& supplier, Warehouse& warehouse, const std::shared_ptr<const Product>& product, int quantity, const Date& expected) : number(number), supplier(&supplier), warehouse(&warehouse), product(product), quantity(quantity), expected(expected), state(State::Pending)
{
    if (number.empty() || !product || quantity <= 0)
        throw OrderException("Invalid purchase order");
}

PurchaseOrder::State PurchaseOrder::getState() const
{
    return state;
}

void PurchaseOrder::receive()
{
    if (state != State::Pending || supplier->getCountry().empty())
        throw OrderException("Purchase order cannot be received");
    warehouse->receive(product, quantity);
    state = State::Received;
}

void PurchaseOrder::cancel()
{
    if (state != State::Pending)
        throw OrderException("Purchase order cannot be cancelled");
    state = State::Cancelled;
}

bool PurchaseOrder::isOverdue(const Date& date) const
{
    return state == State::Pending && expected.isBefore(date);
}

Invoice::Invoice(const std::string& number, const SalesOrder& order, const Date& due) : number(number), order(&order), amount(order.total()), paid(0, amount.getCurrency()), due(due)
{
    if (number.empty() || (order.getState() != SalesOrder::State::Confirmed && order.getState() != SalesOrder::State::Shipped) || amount.getKopecks() == 0)
        throw OrderException("Invoice requires a confirmed, nonzero order");
}

Money Invoice::outstanding() const
{
    return amount.subtract(paid);
}

bool Invoice::canApply(const Money& payment) const
{
    return order->getState() != SalesOrder::State::Cancelled && payment.getKopecks() > 0 && outstanding().covers(payment);
}

void Invoice::applyPayment(const Money& payment)
{
    if (!canApply(payment))
        throw PaymentException("Payment exceeds invoice balance or order is cancelled");
    paid = paid.add(payment);
}

bool Invoice::isOverdue(const Date& date) const
{
    return !isSettled() && order->getState() != SalesOrder::State::Cancelled && due.isBefore(date);
}

bool Invoice::isSettled() const
{
    return outstanding().getKopecks() == 0;
}

Payment::Payment(const std::string& number, Invoice& invoice, BankAccount& payer, BankAccount& recipient, const Money& amount) : number(number), invoice(&invoice), payer(&payer), recipient(&recipient), amount(amount), executed(false)
{
    if (number.empty() || amount.getKopecks() <= 0 || &payer == &recipient)
        throw PaymentException("Invalid payment");
}

bool Payment::canExecute() const
{
    return !executed && invoice->canApply(amount) && payer->canTransferTo(*recipient, amount);
}

void Payment::execute()
{
    if (!canExecute())
        throw PaymentException("Payment cannot be executed");
    payer->transferTo(*recipient, amount);
    invoice->applyPayment(amount);
    executed = true;
}

std::string Payment::receipt() const
{
    if (!executed)
        throw PaymentException("Payment has not been executed");
    return number + ": " + std::to_string(amount.getKopecks()) + " " + amount.getCurrency() + " (kopecks)";
}

ReturnRequest::ReturnRequest(const std::string& number, SalesOrder& order, ProductBatch& batch, int quantity, const std::string& reason) : number(number), order(&order), batch(&batch), quantity(quantity), reason(reason), state(State::Pending)
{
    if (number.empty() || reason.empty() || quantity <= 0 || quantity > order.quantityOf(batch.getProduct().getSku()))
        throw OrderException("Invalid return request");
}

ReturnRequest::State ReturnRequest::getState() const
{
    return state;
}

void ReturnRequest::approve()
{
    if (state != State::Pending || order->getState() != SalesOrder::State::Shipped)
        throw OrderException("Only a pending return for a shipped order can be approved");
    order->recordReturn(batch->getProduct(), quantity);
    batch->quarantine();
    state = State::Approved;
}

void ReturnRequest::reject()
{
    if (state != State::Pending)
        throw OrderException("Return request is already resolved");
    state = State::Rejected;
}

Money ReturnRequest::refundableAmount() const
{
    if (state != State::Approved)
        throw OrderException("Return request has not been approved");
    for (const OrderLine& line : order->getLines())
        if (line.getProduct().getSku() == batch->getProduct().getSku()) return line.refundFor(quantity);
    throw OrderException("Product is not in the order");
}
