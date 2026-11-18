#include "Company.h"

#include <UnitTest++/UnitTest++.h>

#include <limits>
#include <sstream>
#include <type_traits>

namespace
{
    struct DistributionFixture
    {
        Date today{2026, 10, 29};
        Date expiry{2027, 10, 29};
        License wholesale{"W", "wholesale", expiry};
        License quality{"Q", "quality", expiry};
        License driving{"D", "driver", expiry};
        Supplier supplier{"Factory", "111111111", "BY"};
        PharmacyClient client{"Pharmacy", "222222222", "L", 0.0};
        std::shared_ptr<const Product> product = std::make_shared<OverTheCounterDrug>("S", "Drug", "Factory", 10.0, "substance", 10.0, 0.0);
        std::shared_ptr<const Product> vaccine = std::make_shared<Vaccine>("V", "Vaccine", "Factory", 10.0, "antigen", 1.0, 2.0, 8.0, 1);
        ProductBatch batch{product, "B", expiry, 100};
        StorageZone zone{"Zone", StorageCondition(15.0, 25.0, 70.0, true), 500};
        SalesOrder order{"O", client, today};
        Address origin{"Minsk", "A", "1", "220001"};
        Address destination{"Minsk", "B", "2", "220002"};
        DeliveryRoute route{origin, destination, 30.0, 30.0};
        Vehicle vehicle{"AB-1", 100, 20.0};
        Driver driver{"Ivan", driving, 8};

        DistributionFixture()
        {
            zone.receive(batch, today, 20.0, 50.0);
            order.addLine(OrderLine(*product, 10, Money(1000), 500));
        }

        void confirm()
        {
            order.confirm(wholesale, today);
        }
    };
}

SUITE(CommonValues)
{
    TEST(CalendarValidatesLeapYears)
    {
        CHECK_EQUAL("2024-02-29", Date(2024, 2, 29).format());
        CHECK_THROW(Date(2023, 2, 29), DateException);
        CHECK_THROW(Date(1900, 2, 29), DateException);
        CHECK_EQUAL("2000-02-29", Date(2000, 2, 29).format());
        CHECK_THROW(Date(2026, 0, 1), DateException);
        CHECK_THROW(Date(2026, 13, 1), DateException);
        CHECK_THROW(Date(0, 1, 1), DateException);
    }

    TEST(CalendarArithmeticCrossesYearsAndLeapDays)
    {
        CHECK_EQUAL("2024-02-29", Date(2024, 3, 1).addDays(-1).format());
        CHECK_EQUAL("2027-01-01", Date(2026, 12, 31).addDays(1).format());
        CHECK_EQUAL(366, Date(2024, 1, 1).daysUntil(Date(2025, 1, 1)));
        CHECK_EQUAL(-366, Date(2025, 1, 1).daysUntil(Date(2024, 1, 1)));
        CHECK_THROW(Date(1, 1, 1).addDays(-1), DateException);
        CHECK_THROW(Date(9999, 12, 31).addDays(1), DateException);
        CHECK_THROW(Date(2026, 1, 1).addDays(std::numeric_limits<int>::max()), DateException);
    }

    TEST(MoneyComputesWithoutFloatingPointErrors)
    {
        CHECK_EQUAL(30, Money(10).add(Money(20)).getKopecks());
        CHECK_EQUAL(1, Money(101).subtract(Money(100)).getKopecks());
        CHECK_EQUAL(300, Money(100).multiply(3).getKopecks());
        CHECK_EQUAL(101, Money(101).discounted(0).getKopecks());
        CHECK_EQUAL(0, Money(101).discounted(10000).getKopecks());
        CHECK_EQUAL(51, Money(101).discounted(5000).getKopecks());
    }

    TEST(MoneyRejectsInvalidAmountsCurrenciesAndOverflow)
    {
        const Money maximum(std::numeric_limits<std::int64_t>::max());
        CHECK_THROW(Money(-1), PaymentException);
        CHECK_THROW(Money(1, "byn"), PaymentException);
        CHECK_THROW(Money(1).add(Money(1, "USD")), PaymentException);
        CHECK_THROW(Money(1).subtract(Money(2)), PaymentException);
        CHECK_THROW(maximum.add(Money(1)), PaymentException);
        CHECK_THROW(maximum.multiply(2), PaymentException);
        CHECK_THROW(Money(1).multiply(-1), PaymentException);
        CHECK_THROW(Money(1).discounted(10001), PaymentException);
        CHECK_EQUAL(maximum.getKopecks(), maximum.discounted(0).getKopecks());
    }

    TEST(AccountTransferValidatesBothSidesBeforeChangingBalances)
    {
        BankAccount source("A", "Buyer", Money(1000));
        BankAccount target("B", "Seller", Money(0));
        source.transferTo(target, Money(250));
        CHECK_EQUAL(750, source.getBalance().getKopecks());
        CHECK_EQUAL(250, target.getBalance().getKopecks());
        target.block();
        CHECK_THROW(source.transferTo(target, Money(100)), PaymentException);
        CHECK_EQUAL(750, source.getBalance().getKopecks());
        CHECK_THROW(source.transferTo(source, Money(1)), PaymentException);
        CHECK_THROW(target.withdraw(Money(1)), PaymentException);
    }

    TEST(AccountRejectsOverflowAndCurrencyMismatchWithoutDebiting)
    {
        BankAccount source("A", "Buyer", Money(1000));
        BankAccount full("B", "Seller", Money(std::numeric_limits<std::int64_t>::max()));
        BankAccount foreign("C", "Foreign", Money(0, "USD"));
        CHECK_THROW(source.transferTo(full, Money(1)), PaymentException);
        CHECK_THROW(source.transferTo(foreign, Money(1)), PaymentException);
        CHECK_EQUAL(1000, source.getBalance().getKopecks());
        source.deposit(Money(10));
        source.withdraw(Money(20));
        CHECK_EQUAL(990, source.getBalance().getKopecks());
    }

    TEST(AddressAndContactValidateAndMatch)
    {
        const Address address("Minsk", "A", "1", "220001");
        CHECK(address.sameCity(Address("Minsk", "B", "2", "220002")));
        CHECK(address.canDeliverTo("220"));
        CHECK(!address.canDeliverTo(""));
        CHECK(!address.canDeliverTo("210"));
        CHECK_THROW(Address("", "A", "1", "220001"), ValidationException);
        const Contact contact("Anna", "+375 (29) 123-45-67", "anna@example.com");
        CHECK(contact.matchesPhone("375291234567"));
        CHECK(contact.belongsToDomain("example.com"));
        CHECK(!contact.belongsToDomain("com"));
        CHECK_EQUAL("Anna <anna@example.com>", contact.mailingLabel());
        CHECK_THROW(Contact("Anna", "1234567", "invalid"), ValidationException);
    }

    TEST(LicenseExpiresAfterItsLastValidDayAndCanBeRevoked)
    {
        License license("L", "wholesale", Date(2026, 10, 29));
        CHECK(license.permits("wholesale", Date(2026, 10, 29)));
        CHECK(!license.permits("quality", Date(2026, 10, 29)));
        CHECK_THROW(license.requireValid("wholesale", Date(2026, 10, 30)), LicenseException);
        license.revoke();
        CHECK(!license.permits("wholesale", Date(2026, 10, 29)));
        CHECK_THROW(license.revoke(), LicenseException);
    }
}

SUITE(BatchesAndQuality)
{
    TEST_FIXTURE(DistributionFixture, BatchReservationAndShipmentPreserveQuantities)
    {
        batch.reserve(20, today);
        CHECK_EQUAL(80, batch.available());
        CHECK_THROW(batch.reserve(81, today), StockException);
        CHECK_EQUAL(80, batch.available());
        batch.shipReserved(5, today);
        CHECK_EQUAL(95, batch.getQuantity());
        CHECK_EQUAL(80, batch.available());
        batch.releaseReservation(15);
        CHECK_EQUAL(95, batch.available());
        CHECK_THROW(batch.shipReserved(1, today), StockException);
    }

    TEST_FIXTURE(DistributionFixture, ExpiredAndQuarantinedBatchesCannotBeReserved)
    {
        CHECK(!batch.isExpired(expiry));
        CHECK(batch.isExpired(expiry.addDays(1)));
        CHECK_THROW(batch.reserve(1, expiry.addDays(1)), ExpiredProductException);
        batch.quarantine();
        CHECK_THROW(batch.reserve(1, today), QualityException);
        batch.releaseQuarantine();
        CHECK(batch.canRelease(today));
        CHECK_THROW(batch.releaseQuarantine(), QualityException);
    }

    TEST_FIXTURE(DistributionFixture, CertificateChecksDatesLicenseAndIdentity)
    {
        QualityCertificate certificate(batch, quality, today, expiry);
        CHECK(!certificate.covers(batch, today));
        certificate.approve();
        CHECK(certificate.covers(batch, today));
        CHECK(!certificate.covers(batch, today.addDays(-1)));
        ProductBatch other(product, "B2", expiry, 5);
        CHECK(!certificate.covers(other, today));
        quality.revoke();
        CHECK(!certificate.covers(batch, today));
        CHECK_THROW(certificate.approve(), LicenseException);
        CHECK_THROW(QualityCertificate(batch, quality, today, expiry.addDays(1)), QualityException);
        certificate.withdraw();
        CHECK(!certificate.covers(batch, today));
    }

    TEST_FIXTURE(DistributionFixture, RecallMatchesExactBatchAndQuarantinesIt)
    {
        RecallNotice notice("R", "S", "B", "Quality defect", today);
        ProductBatch other(product, "B2", expiry, 5);
        CHECK(notice.appliesTo(batch));
        CHECK(!notice.appliesTo(other));
        CHECK_THROW(notice.execute(batch, today.addDays(-1)), QualityException);
        CHECK_THROW(notice.execute(other, today), QualityException);
        notice.execute(batch, today);
        CHECK(!batch.canRelease(today));
        CHECK(notice.describe().find("Quality defect") != std::string::npos);
    }

    TEST(StorageConditionChecksHumidityLightAndNonfiniteValues)
    {
        const StorageCondition condition(2.0, 8.0, 60.0, true);
        CHECK(condition.accepts(2.0, 60.0, false));
        CHECK(condition.accepts(8.0, 0.0, false));
        CHECK(!condition.accepts(5.0, 61.0, false));
        CHECK(!condition.accepts(5.0, 50.0, true));
        CHECK(!condition.accepts(std::numeric_limits<double>::quiet_NaN(), 50.0, false));
        CHECK_THROW(condition.requireAcceptable(9.0, 50.0, false), TemperatureException);
        CHECK(condition.overlaps(StorageCondition(8.0, 10.0, 50.0, false)));
        CHECK(!condition.overlaps(StorageCondition(15.0, 25.0, 50.0, false)));
    }

    TEST_FIXTURE(DistributionFixture, CatalogRejectsDuplicateSkuAndFiltersByStorage)
    {
        ProductCatalog catalog("Catalog");
        CHECK_THROW(catalog.publish(), OrderException);
        catalog.add(product);
        catalog.add(vaccine);
        CHECK_THROW(catalog.add(product), DuplicateException);
        CHECK_THROW(catalog.add(nullptr), ValidationException);
        CHECK_EQUAL(1u, catalog.suitableFor(5.0).size());
        CHECK(catalog.find("S") == product);
        catalog.publish();
        CHECK_THROW(catalog.publish(), OrderException);
        catalog.remove("V");
        CHECK(!catalog.find("V"));
        CHECK_THROW(catalog.remove("V"), StockException);
    }

    TEST_FIXTURE(DistributionFixture, PackagingEnforcesColdChainAndCapacity)
    {
        Packaging ordinary("Cardboard", 10, false);
        CHECK_THROW(ordinary.pack(*vaccine, 1), TemperatureException);
        ordinary.pack(*product, 10);
        CHECK_EQUAL(0, ordinary.remainingCapacity());
        CHECK_THROW(ordinary.pack(*product, 1), StockException);
        ordinary.unpack(4);
        CHECK_EQUAL(4, ordinary.remainingCapacity());
        Packaging insulated("Thermal box", 10, true);
        insulated.pack(*vaccine, 1);
        CHECK_EQUAL(9, insulated.remainingCapacity());
    }
}

SUITE(CommercialRules)
{
    TEST_FIXTURE(DistributionFixture, ContractRequiresValidLicenseAndAppliesDiscount)
    {
        Contract contract(supplier, client, wholesale, expiry, 1000);
        CHECK(contract.permitsSaleTo(client, today));
        CHECK_EQUAL(9000, contract.calculateTotal(Money(1000), 10, today).getKopecks());
        contract.terminate();
        CHECK_THROW(contract.calculateTotal(Money(1000), 10, today), OrderException);
        CHECK_THROW(contract.terminate(), OrderException);
    }

    TEST_FIXTURE(DistributionFixture, DiscountPolicyRespectsQuantityExpiryAndCap)
    {
        DiscountPolicy policy(10, 500, 1000, expiry);
        CHECK_EQUAL(1000, policy.apply(Money(1000), 9, today).getKopecks());
        CHECK_EQUAL(950, policy.apply(Money(1000), 10, today).getKopecks());
        CHECK_EQUAL(1000, policy.apply(Money(1000), 10, expiry.addDays(1)).getKopecks());
        CHECK_EQUAL(1000, policy.combineWith(800, 10, today));
        CHECK_EQUAL(800, policy.combineWith(800, 9, today));
        CHECK_THROW(policy.combineWith(-1, 10, today), ValidationException);
    }

    TEST_FIXTURE(DistributionFixture, CreditLimitIsEnforcedAndRepaymentRemainsPossibleWhenFrozen)
    {
        CreditAccount credit(client, Money(1000));
        credit.charge(Money(600));
        CHECK_THROW(credit.charge(Money(401)), CreditLimitException);
        CHECK_EQUAL(600, credit.getDebt().getKopecks());
        credit.freeze();
        CHECK(!credit.canCharge(Money(1)));
        credit.repay(Money(100));
        CHECK_EQUAL(500, credit.getDebt().getKopecks());
        CHECK_THROW(credit.repay(Money(501)), PaymentException);
    }

    TEST_FIXTURE(DistributionFixture, TenderSelectsCheapestValidBidAfterDeadline)
    {
        Supplier second("Second", "333333333", "BY");
        Tender tender("T", *product, 10, Money(10000), today.addDays(1));
        SupplierBid firstBid(supplier, *product, Money(900), 10, expiry);
        SupplierBid secondBid(second, *product, Money(800), 10, expiry);
        tender.submit(firstBid, today);
        tender.submit(secondBid, today);
        CHECK_THROW(tender.submit(firstBid, today), DuplicateException);
        CHECK_THROW(tender.selectWinner(today), OrderException);
        const SupplierBid winner = tender.selectWinner(today.addDays(1));
        CHECK_EQUAL("333333333", winner.getSupplier().getTaxId());
        CHECK_EQUAL(8000, winner.total().getKopecks());
        CHECK_THROW(tender.selectWinner(today.addDays(1)), OrderException);
    }

    TEST_FIXTURE(DistributionFixture, TenderRejectsWrongQuantityProductAndExpiredBids)
    {
        Tender tender("T", *product, 10, Money(10000), today);
        CHECK_THROW(tender.submit(SupplierBid(supplier, *product, Money(1), 9, expiry), today), OrderException);
        CHECK_THROW(tender.submit(SupplierBid(supplier, *vaccine, Money(1), 10, expiry), today), OrderException);
        CHECK_THROW(tender.submit(SupplierBid(supplier, *product, Money(1), 10, today.addDays(-1)), today), OrderException);
        CHECK_THROW(tender.selectWinner(today), OrderException);
        tender.cancel();
        CHECK_THROW(tender.submit(SupplierBid(supplier, *product, Money(1), 10, expiry), today), OrderException);
    }

    TEST_FIXTURE(DistributionFixture, PriceListMustBeActivatedAndHasFixedCurrency)
    {
        PriceList prices("Retail", "BYN");
        CHECK_THROW(prices.quote(*product, 1), OrderException);
        CHECK_THROW(prices.addPrice(*product, Money(100, "USD")), PaymentException);
        prices.addPrice(*product, Money(1000));
        CHECK_THROW(prices.addPrice(*product, Money(1000)), DuplicateException);
        prices.activate();
        CHECK_EQUAL(3000, prices.quote(*product, 3).getKopecks());
        CHECK_THROW(prices.addPrice(*vaccine, Money(100)), PaymentException);
        prices.retire();
        CHECK_THROW(prices.quote(*product, 1), OrderException);
    }
}

SUITE(OrderProcessing)
{
    TEST_FIXTURE(DistributionFixture, OrderLineRoundsDiscountPerUnitAndRefundsConsistently)
    {
        OrderLine line(*product, 3, Money(101), 5000);
        CHECK_EQUAL(153, line.total().getKopecks());
        CHECK_EQUAL(102, line.refundFor(2).getKopecks());
        line.increase(1);
        line.reduce(2);
        CHECK_EQUAL(2, line.getQuantity());
        CHECK_THROW(line.reduce(2), OrderException);
        CHECK_THROW(line.refundFor(3), OrderException);
    }

    TEST_FIXTURE(DistributionFixture, OrderStateMachineRejectsEditsAndShippingAfterCancellation)
    {
        CHECK_EQUAL(9500, order.total().getKopecks());
        CHECK_THROW(order.addLine(OrderLine(*product, 1, Money(1))), DuplicateException);
        CHECK_THROW(order.addLine(OrderLine(*vaccine, 1, Money(1, "USD"))), PaymentException);
        CHECK_THROW(order.markShipped(), OrderException);
        confirm();
        CHECK_THROW(order.addLine(OrderLine(*vaccine, 1, Money(1))), OrderException);
        CHECK_THROW(order.confirm(wholesale, today), OrderException);
        order.cancel();
        CHECK_THROW(order.markShipped(), OrderException);
    }

    TEST_FIXTURE(DistributionFixture, OrderConfirmationChecksCreationDateLicenseAndContents)
    {
        CHECK_THROW(order.confirm(wholesale, today.addDays(-1)), OrderException);
        wholesale.revoke();
        CHECK_THROW(order.confirm(wholesale, today), LicenseException);
        CHECK(order.getState() == SalesOrder::State::Draft);
        SalesOrder empty("Empty", client, today);
        CHECK_THROW(empty.confirm(quality, today), OrderException);
    }

    TEST_FIXTURE(DistributionFixture, PurchaseOrderReceivesOnceAndPreservesStateOnFailure)
    {
        Warehouse warehouse("Warehouse", 20.0);
        PurchaseOrder purchase("PO", supplier, warehouse, product, 5, today);
        CHECK(purchase.isOverdue(today.addDays(1)));
        purchase.receive();
        CHECK_EQUAL(5, warehouse.quantityOf("S"));
        CHECK(!purchase.isOverdue(today.addDays(1)));
        CHECK_THROW(purchase.receive(), OrderException);
        PurchaseOrder cold("PO2", supplier, warehouse, vaccine, 5, today);
        CHECK_THROW(cold.receive(), std::runtime_error);
        CHECK(cold.getState() == PurchaseOrder::State::Pending);
        cold.cancel();
        CHECK_THROW(cold.receive(), OrderException);
    }

    TEST_FIXTURE(DistributionFixture, InvoiceAcceptsPartialPaymentsButRejectsOverpayment)
    {
        CHECK_THROW(Invoice("I", order, expiry), OrderException);
        confirm();
        Invoice invoice("I", order, today);
        CHECK(invoice.isOverdue(today.addDays(1)));
        invoice.applyPayment(Money(500));
        CHECK_EQUAL(9000, invoice.outstanding().getKopecks());
        CHECK_THROW(invoice.applyPayment(Money(9001)), PaymentException);
        invoice.applyPayment(Money(9000));
        CHECK(invoice.isSettled());
        CHECK(!invoice.isOverdue(today.addDays(1)));
    }

    TEST_FIXTURE(DistributionFixture, PaymentTransfersAndSettlesExactlyOnce)
    {
        confirm();
        Invoice invoice("I", order, expiry);
        BankAccount payer("A", "Buyer", Money(10000));
        BankAccount seller("B", "Seller", Money(0));
        Payment payment("P", invoice, payer, seller, Money(9500));
        CHECK_THROW(payment.receipt(), PaymentException);
        payment.execute();
        CHECK(invoice.isSettled());
        CHECK_EQUAL(500, payer.getBalance().getKopecks());
        CHECK_EQUAL(9500, seller.getBalance().getKopecks());
        CHECK_THROW(payment.execute(), PaymentException);
        CHECK(payment.receipt().find("9500") != std::string::npos);
    }

    TEST_FIXTURE(DistributionFixture, FailedPaymentDoesNotChangeInvoiceOrBankBalances)
    {
        confirm();
        Invoice invoice("I", order, expiry);
        BankAccount payer("A", "Buyer", Money(10000));
        BankAccount seller("B", "Seller", Money(0));
        seller.block();
        Payment payment("P", invoice, payer, seller, Money(9500));
        CHECK(!payment.canExecute());
        CHECK_THROW(payment.execute(), PaymentException);
        CHECK_EQUAL(10000, payer.getBalance().getKopecks());
        CHECK_EQUAL(9500, invoice.outstanding().getKopecks());
        order.cancel();
        CHECK(!invoice.canApply(Money(100)));
    }

    TEST_FIXTURE(DistributionFixture, ReturnRequiresShippedOrderAndQuarantinesBatch)
    {
        ReturnRequest request("R", order, batch, 2, "Damaged box");
        CHECK_THROW(request.approve(), OrderException);
        confirm();
        order.markShipped();
        request.approve();
        CHECK_EQUAL(1900, request.refundableAmount().getKopecks());
        CHECK(!batch.canRelease(today));
        CHECK_THROW(request.approve(), OrderException);
        CHECK_THROW(request.reject(), OrderException);
        CHECK_THROW(ReturnRequest("R2", order, batch, 11, "Damaged"), OrderException);
    }

    TEST_FIXTURE(DistributionFixture, MultipleReturnRequestsCannotExceedShippedQuantity)
    {
        confirm();
        order.markShipped();
        ReturnRequest first("R1", order, batch, 7, "Damaged");
        ReturnRequest second("R2", order, batch, 4, "Damaged");
        first.approve();
        CHECK_THROW(second.approve(), OrderException);
        CHECK(second.getState() == ReturnRequest::State::Pending);
        ReturnRequest remainder("R3", order, batch, 3, "Damaged");
        remainder.approve();
        CHECK_EQUAL(2850, remainder.refundableAmount().getKopecks());
    }
}

SUITE(StockControl)
{
    TEST_FIXTURE(DistributionFixture, ZoneUsesFirstExpiryAndSkipsBlockedBatches)
    {
        ProductBatch earlier(product, "Early", today.addDays(5), 10);
        zone.receive(earlier, today, 20.0, 50.0);
        CHECK(&zone.selectFirstExpiring("S", today) == &earlier);
        earlier.quarantine();
        CHECK(&zone.selectFirstExpiring("S", today) == &batch);
        CHECK_THROW(zone.selectFirstExpiring("S", expiry.addDays(1)), StockException);
        CHECK_EQUAL(110, zone.quantityOf("S"));
    }

    TEST_FIXTURE(DistributionFixture, ZoneRejectsOverflowDuplicatesAndWrongTemperature)
    {
        ProductBatch tooLarge(product, "Large", expiry, 401);
        ProductBatch cold(vaccine, "Cold", expiry, 1);
        CHECK_THROW(zone.receive(tooLarge, today, 20.0, 50.0), StockException);
        CHECK_THROW(zone.receive(batch, today, 20.0, 50.0), DuplicateException);
        CHECK_THROW(zone.receive(cold, today, 20.0, 50.0), TemperatureException);
        batch.reserve(1, today);
        CHECK_THROW(zone.remove(batch), StockException);
        batch.releaseReservation(1);
        zone.remove(batch);
        CHECK_EQUAL(0, zone.occupied());
    }

    TEST_FIXTURE(DistributionFixture, RackChecksDimensionsCapacityAndZoneStock)
    {
        StorageRack rack("R", zone, 2, 30);
        rack.allocate(50);
        CHECK_EQUAL(10, rack.freeCapacity());
        CHECK_THROW(rack.allocate(11), StockException);
        rack.release(20);
        CHECK_EQUAL(30, rack.freeCapacity());
        CHECK_THROW(StorageRack("Huge", zone, std::numeric_limits<int>::max(), 2), ValidationException);
    }

    TEST_FIXTURE(DistributionFixture, ReservationCanBeCancelledAndCannotShipAfterExpiry)
    {
        Reservation reservation("R", batch, 10, today);
        reservation.activate(today);
        CHECK_EQUAL(90, batch.available());
        CHECK_THROW(reservation.activate(today), StockException);
        CHECK_THROW(reservation.ship(today.addDays(1)), StockException);
        CHECK_EQUAL(100, batch.getQuantity());
        reservation.cancel();
        CHECK_EQUAL(100, batch.available());
        CHECK_THROW(reservation.cancel(), StockException);
        CHECK((!std::is_copy_constructible<Reservation>::value));
    }

    TEST_FIXTURE(DistributionFixture, TemperatureLogDetectsExcursionsAndEmptyReadings)
    {
        TemperatureLog log(zone, today, 15.0, 25.0);
        CHECK_THROW(log.average(), TemperatureException);
        log.record(20.0);
        log.record(30.0);
        CHECK_CLOSE(25.0, log.average(), 1e-9);
        CHECK_EQUAL(1, log.excursionCount());
        CHECK_THROW(log.requireSafeFor(*product), TemperatureException);
        CHECK_THROW(log.record(std::numeric_limits<double>::infinity()), TemperatureException);
    }

    TEST_FIXTURE(DistributionFixture, AuditReportsMissingExcessAndUncountedProducts)
    {
        StockAudit audit(zone, today);
        audit.record("S", 95);
        audit.record("UNKNOWN", 2);
        CHECK_EQUAL(-5, audit.discrepancies().at("S"));
        CHECK_EQUAL(2, audit.discrepancies().at("UNKNOWN"));
        CHECK_THROW(audit.record("S", 100), DuplicateException);
        audit.close();
        CHECK_THROW(audit.record("V", 0), StockException);
        StockAudit uncounted(zone, today);
        CHECK_EQUAL(-100, uncounted.discrepancies().at("S"));
    }

    TEST_FIXTURE(DistributionFixture, ReorderRoundsUpToWholePackages)
    {
        ReorderPolicy policy(*product, 100, 151, 20);
        CHECK(policy.needsOrder(zone));
        CHECK_EQUAL(60, policy.recommendedQuantity(zone));
        CHECK_EQUAL(6000, policy.estimateCost(zone, Money(100)).getKopecks());
        ReorderPolicy noOrder(*product, 99, 151, 20);
        CHECK_EQUAL(0, noOrder.recommendedQuantity(zone));
    }
}

SUITE(DeliveryFlow)
{
    TEST_FIXTURE(DistributionFixture, ShipmentDispatchAndSignedDeliveryCompleteTheOrder)
    {
        confirm();
        Reservation reservation("R", batch, 10, expiry);
        reservation.activate(today);
        Shipment shipment("S", order, vehicle, driver, route, today);
        shipment.addReservation(reservation);
        CHECK_THROW(shipment.addReservation(reservation), DuplicateException);
        shipment.dispatch(today);
        CHECK_EQUAL(90, batch.getQuantity());
        CHECK(order.getState() == SalesOrder::State::Shipped);
        CHECK(!vehicle.canCarry(*product, 1));
        CHECK(shipment.isLate(today.addDays(2)));
        DeliveryNote note("N", shipment, destination, Contact("Anna", "1234567", "a@example.com"));
        CHECK_THROW(note.proof(), DeliveryException);
        CHECK(note.matchesDestination(destination));
        note.sign("Anna", today);
        CHECK(shipment.getState() == Shipment::State::Delivered);
        CHECK(vehicle.canCarry(*product, 1));
        CHECK(!shipment.isLate(today.addDays(2)));
        CHECK_THROW(note.sign("Anna", today), DeliveryException);
        CHECK(note.proof().find("Anna") != std::string::npos);
    }

    TEST_FIXTURE(DistributionFixture, ShipmentChecksFullOrderBeforeMutatingAnyReservation)
    {
        order.addLine(OrderLine(*vaccine, 5, Money(1000)));
        confirm();
        ProductBatch cold(vaccine, "Cold", expiry, 10);
        Reservation first("R1", batch, 10, expiry);
        Reservation second("R2", cold, 5, expiry);
        first.activate(today);
        second.activate(today);
        Shipment shipment("S", order, vehicle, driver, route, today);
        shipment.addReservation(first);
        shipment.addReservation(second);
        CHECK_THROW(shipment.dispatch(today), DeliveryException);
        CHECK_EQUAL(100, batch.getQuantity());
        CHECK_EQUAL(10, cold.getQuantity());
        CHECK(first.getState() == Reservation::State::Active);
        CHECK(vehicle.canCarry(*product, 1));
        CHECK(driver.canDrive(today, 1));
        first.cancel();
        second.cancel();
    }

    TEST_FIXTURE(DistributionFixture, ShipmentRejectsPartialOrderOverloadAndRepeatedDeparture)
    {
        confirm();
        Reservation partial("R", batch, 9, expiry);
        partial.activate(today);
        Shipment shipment("S", order, vehicle, driver, route, today);
        shipment.addReservation(partial);
        CHECK_THROW(shipment.dispatch(today), DeliveryException);
        CHECK_EQUAL(100, batch.getQuantity());
        partial.cancel();
        Reservation full("R2", batch, 10, expiry);
        full.activate(today);
        Vehicle small("Small", 9, 20.0);
        Shipment overloaded("S2", order, small, driver, route, today);
        overloaded.addReservation(full);
        CHECK_THROW(overloaded.dispatch(today), DeliveryException);
        Shipment valid("S3", order, vehicle, driver, route, today);
        valid.addReservation(full);
        valid.dispatch(today);
        CHECK_THROW(valid.dispatch(today), DeliveryException);
        CHECK_EQUAL(90, batch.getQuantity());
        valid.complete(today);
    }

    TEST_FIXTURE(DistributionFixture, RouteAndDriverEnforceWorkdayAndLicense)
    {
        CHECK(route.isLocal());
        CHECK_EQUAL(1, route.estimatedHours());
        CHECK_EQUAL(6000, route.transportCost(Money(200)).getKopecks());
        driver.beginTrip(today, 8);
        CHECK_THROW(driver.rest(), DeliveryException);
        driver.finishTrip();
        CHECK(!driver.canDrive(today, 1));
        driver.rest();
        CHECK(driver.canDrive(today, 8));
        driving.revoke();
        CHECK(!driver.canDrive(today, 1));
        CHECK_THROW(DeliveryRoute(origin, destination, -1.0, 30.0), DeliveryException);
    }

    TEST_FIXTURE(DistributionFixture, InsuranceOnlyPaysOnceAndAppliesDeductible)
    {
        confirm();
        Reservation reservation("R", batch, 10, expiry);
        reservation.activate(today);
        Shipment shipment("S", order, vehicle, driver, route, today);
        shipment.addReservation(reservation);
        InsurancePolicy insurance("I", shipment, Money(10000), 1000, expiry);
        CHECK(!insurance.covers(Money(1000), today));
        shipment.dispatch(today);
        CHECK_THROW(insurance.claim(Money(10001), today), PaymentException);
        CHECK_EQUAL(900, insurance.claim(Money(1000), today).getKopecks());
        CHECK_THROW(insurance.claim(Money(1000), today), PaymentException);
        shipment.complete(today);
    }
}

SUITE(CompanyModel)
{
    TEST_FIXTURE(DistributionFixture, CompanyRegistersOrdersAndCalculatesVirtualPayroll)
    {
        PharmaCompany company("Company", wholesale);
        Employee employee("E", "Anna", Money(500));
        SalesRepresentative representative("S", "Ivan", Money(500), client, 1000);
        company.hire(employee);
        company.hire(representative);
        CHECK_THROW(company.hire(employee), DuplicateException);
        CHECK_THROW(company.confirmOrder(order, today), OrderException);
        company.registerOrder(order);
        CHECK_THROW(company.registerOrder(order), DuplicateException);
        SalesOrder duplicate("O", client, today);
        CHECK_THROW(company.registerOrder(duplicate), DuplicateException);
        company.confirmOrder(order, today);
        order.markShipped();
        representative.bookSale(order);
        CHECK_THROW(representative.bookSale(order), DuplicateException);
        const Employee& polymorphic = representative;
        CHECK_EQUAL(4950, polymorphic.payroll(8).getKopecks());
        CHECK_EQUAL(8950, company.payrollCost(8).getKopecks());
        employee.dismiss();
        CHECK_EQUAL(4950, company.payrollCost(8).getKopecks());
    }

    TEST_FIXTURE(DistributionFixture, QualityInspectorQuarantinesAndCanReleaseUnexpiredBatch)
    {
        QualityInspector inspector("Q", "Anna", Money(500), quality);
        QualityCertificate certificate(batch, quality, today, expiry);
        CHECK(!inspector.inspect(batch, certificate, today));
        CHECK(!batch.canRelease(today));
        CHECK_CLOSE(1.0, inspector.rejectionRate(), 1e-9);
        inspector.releaseBatch(batch, today);
        certificate.approve();
        CHECK(inspector.inspect(batch, certificate, today));
        CHECK_CLOSE(0.5, inspector.rejectionRate(), 1e-9);
        batch.quarantine();
        License longerLicense("Q2", "quality", expiry.addDays(10));
        QualityInspector secondInspector("Q2", "Ivan", Money(500), longerLicense);
        CHECK_THROW(secondInspector.releaseBatch(batch, expiry.addDays(1)), ExpiredProductException);
    }

    TEST_FIXTURE(DistributionFixture, CompanyQuarantinesExpiredBatchesWithoutChangingQuantity)
    {
        PharmaCompany company("Company", wholesale);
        company.attachZone(zone);
        CHECK_THROW(company.attachZone(zone), DuplicateException);
        CHECK_EQUAL(0, company.quarantineExpired(today));
        CHECK_EQUAL(1, company.quarantineExpired(expiry.addDays(1)));
        CHECK_EQUAL(100, batch.getQuantity());
    }

    TEST(CompleteCompanyExampleRuns)
    {
        std::ostringstream output;
        runCompanyExample(output);
        CHECK(output.str().find("10450") != std::string::npos);
        CHECK(output.str().find("90") != std::string::npos);
    }
}

SUITE(NumericRegression)
{
    TEST(OriginalConstructorsRejectNonfiniteNumbers)
    {
        const double nan = std::numeric_limits<double>::quiet_NaN();
        const double infinity = std::numeric_limits<double>::infinity();
        CHECK_THROW(PrescriptionDrug("S", "A", "B", nan, "C", 1.0), std::invalid_argument);
        CHECK_THROW(PrescriptionDrug("S", "A", "B", 1.0, "C", infinity), std::invalid_argument);
        CHECK_THROW(OverTheCounterDrug("S", "A", "B", 1.0, "C", 1.0, nan), std::invalid_argument);
        CHECK_THROW(ColdChainProduct("S", "A", "B", 1.0, "C", 1.0, nan, 8.0), std::invalid_argument);
        CHECK_THROW(PharmacyClient("A", "123456789", "L", nan), std::invalid_argument);
        CHECK_THROW(Warehouse("A", infinity), std::invalid_argument);
    }

    TEST_FIXTURE(DistributionFixture, OriginalInventoryRejectsIntegerOverflowWithoutChangingStock)
    {
        Warehouse warehouse("A", 20.0);
        warehouse.receive(product, std::numeric_limits<int>::max());
        CHECK_THROW(warehouse.receive(product, 1), std::overflow_error);
        CHECK_EQUAL(std::numeric_limits<int>::max(), warehouse.quantityOf("S"));
    }
}
