# Лабораторная работа №2. Крупный ООП-проект

## Предметная область

Компания по сбыту лекарственных препаратов.

Проект моделирует лекарственные препараты, поставщиков и аптечных клиентов, склады, партии товаров, продажи, заказы, платежи, доставку, контроль качества, сотрудников, тендеры, возвраты и управление запасами.

## Классы

Формат строки: `Класс | число полей | число методов | связанные классы`.

> В столбце «Методы» учитываются конструкторы и содержательные методы, объявленные у класса. Простые автоматически унаследованные конструкторы исключений отдельно не считаются.

| Класс | Поля | Методы | Ассоциации (связанные классы) |
|---|---:|---:|---|
| Address | 4 | 4 | — |
| Contact | 3 | 4 | — |
| Date | 3 | 5 | Date |
| Money | 2 | 7 | Money |
| License | 4 | 4 | Date |
| BankAccount | 4 | 6 | Money, BankAccount |
| Product | 4 | 10 | Prescription |
| Medicine | 2 | 6 | Product |
| OverTheCounterDrug | 1 | 4 | Medicine |
| PrescriptionDrug | 0 | 5 | Medicine, Prescription |
| ColdChainProduct | 2 | 8 | Medicine |
| Vaccine | 1 | 5 | PrescriptionDrug, ColdChainProduct |
| Organization | 2 | 5 | — |
| Supplier | 1 | 4 | Organization |
| PharmacyClient | 2 | 5 | Organization |
| StorageCondition | 4 | 4 | StorageCondition |
| ProductBatch | 6 | 12 | Product, Date |
| QualityCertificate | 5 | 4 | ProductBatch, License, Date |
| ProductCatalog | 3 | 6 | Product |
| RecallNotice | 5 | 4 | ProductBatch, Date |
| Packaging | 4 | 5 | Product |
| Inventory | 1 | 8 | Product |
| Warehouse | 2 | 8 | Inventory, Product |
| ColdStorageWarehouse | 0 | 3 | Warehouse |
| StorageZone | 4 | 6 | StorageCondition, ProductBatch, Date |
| StorageRack | 5 | 4 | StorageZone |
| Reservation | 5 | 7 | ProductBatch, Date |
| TemperatureLog | 5 | 5 | StorageZone, Date, Product |
| StockAudit | 4 | 4 | StorageZone, Date |
| ReorderPolicy | 4 | 4 | Product, StorageZone, Money |
| Ledger | 1 | 4 | — |
| SalesDepartment | 1 | 7 | Ledger, Warehouse, Supplier, Product, PharmacyClient, Prescription |
| Contract | 6 | 4 | Supplier, PharmacyClient, License, Date, Money |
| DiscountPolicy | 4 | 4 | Date, Money |
| CreditAccount | 4 | 5 | PharmacyClient, Money |
| SupplierBid | 5 | 6 | Supplier, Product, Money, Date |
| Tender | 7 | 4 | Product, Money, Date, SupplierBid |
| PriceList | 4 | 5 | Product, Money |
| OrderLine | 4 | 6 | Product, Money |
| SalesOrder | 7 | 11 | PharmacyClient, Date, OrderLine, License, Product, Money |
| PurchaseOrder | 7 | 4 | Supplier, Warehouse, Product, Date |
| Invoice | 5 | 6 | SalesOrder, Money, Date |
| Payment | 6 | 4 | Invoice, BankAccount, Money |
| ReturnRequest | 6 | 5 | SalesOrder, ProductBatch, Product, Money |
| DeliveryRoute | 4 | 4 | Address, Money |
| Vehicle | 4 | 4 | Product |
| Driver | 5 | 5 | License, Date |
| Shipment | 8 | 6 | SalesOrder, Vehicle, Driver, DeliveryRoute, Date, Reservation, ProductBatch, Product |
| DeliveryNote | 6 | 4 | Shipment, Address, Contact, Date |
| InsurancePolicy | 6 | 4 | Shipment, Money, Date |
| Employee | 4 | 6 | Money |
| SalesRepresentative | 4 | 5 | Employee, PharmacyClient, SalesOrder, Money |
| QualityInspector | 3 | 5 | Employee, License, ProductBatch, QualityCertificate, Date |
| PharmaCompany | 7 | 8 | License, ProductCatalog, SalesDepartment, Employee, StorageZone, SalesOrder, Date, Money |
| PharmaException | 0 | 1 | std::runtime_error |
| ValidationException | 0 | 0 | PharmaException |
| DateException | 0 | 0 | ValidationException |
| LicenseException | 0 | 0 | PharmaException |
| ExpiredProductException | 0 | 0 | PharmaException |
| StockException | 0 | 0 | PharmaException |
| TemperatureException | 0 | 0 | PharmaException |
| OrderException | 0 | 0 | PharmaException |
| PaymentException | 0 | 0 | PharmaException |
| CreditLimitException | 0 | 0 | PharmaException |
| DeliveryException | 0 | 0 | PharmaException |
| QualityException | 0 | 0 | PharmaException |
| DuplicateException | 0 | 0 | PharmaException |

## Исключения (13)

- `PharmaException`
- `ValidationException`
- `DateException`
- `LicenseException`
- `ExpiredProductException`
- `StockException`
- `TemperatureException`
- `OrderException`
- `PaymentException`
- `CreditLimitException`
- `DeliveryException`
- `QualityException`
- `DuplicateException`

## Итоговая статистика

| Показатель | Значение |
|---|---:|
| Классы | 67 |
| Поля | 210 |
| Методы | 293 |
| Уникальные ассоциации между классами | 133 |
| Собственные классы исключений | 13 |

## Основные иерархии наследования

```text
Product
└── Medicine
    ├── OverTheCounterDrug
    ├── PrescriptionDrug
    │   └── Vaccine
    └── ColdChainProduct
        └── Vaccine

Organization
├── Supplier
└── PharmacyClient

Inventory
└── Warehouse
    └── ColdStorageWarehouse

Employee
├── SalesRepresentative
└── QualityInspector

std::runtime_error
└── PharmaException
    ├── ValidationException
    │   └── DateException
    ├── LicenseException
    ├── ExpiredProductException
    ├── StockException
    ├── TemperatureException
    ├── OrderException
    ├── PaymentException
    ├── CreditLimitException
    ├── DeliveryException
    ├── QualityException
    └── DuplicateException
```

## Примечание

README описывает объектную модель проекта. Не все классы создаются пользователем через интерактивное меню `main.cpp`: часть используется во внутренней логике, демонстрационном сценарии `./pharma --example` и модульных тестах.