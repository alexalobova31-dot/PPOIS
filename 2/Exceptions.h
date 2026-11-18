#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class PharmaException : public std::runtime_error
{
public:
    explicit PharmaException(const std::string& message);
};

class ValidationException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class DateException : public ValidationException
{
public:
    using ValidationException::ValidationException;
};

class LicenseException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class ExpiredProductException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class StockException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class TemperatureException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class OrderException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class PaymentException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class CreditLimitException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class DeliveryException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class QualityException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

class DuplicateException : public PharmaException
{
public:
    using PharmaException::PharmaException;
};

#endif
