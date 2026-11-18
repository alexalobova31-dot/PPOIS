#include "Polynomial.h"

#include <cmath>
#include <iomanip>
#include <stdexcept>

void Polynomial::normalize()
{
    while (coefficients.size() > 1 && std::abs(coefficients.back()) < 1e-12) coefficients.pop_back();
    if (coefficients.empty()) coefficients.push_back(0.0);
}

Polynomial::Polynomial() : coefficients(1, 0.0)  
{
}

Polynomial::Polynomial(int degree)
{
    if (degree < 0)
        throw std::invalid_argument("Degree cannot be negative");
    coefficients.resize(degree + 1, 0.0);
}

Polynomial::Polynomial(const std::vector<double>& coefficients) : coefficients(coefficients)
{
    if (this->coefficients.empty()) this->coefficients.push_back(0.0);
    normalize();
}

Polynomial::Polynomial(const Polynomial& other) : coefficients(other.coefficients)
{
}

Polynomial& Polynomial::operator=(const Polynomial& other)
{
    if (this != &other) coefficients = other.coefficients;
    return *this;
}

Polynomial::~Polynomial() = default;

int Polynomial::degree() const
{
    return static_cast<int>(coefficients.size()) - 1;
}

double& Polynomial::operator[](int index)
{
    if (index < 0 || index > degree()) 
        throw std::out_of_range("Coefficient index out of range");
    return coefficients[index];
}

const double& Polynomial::operator[](int index) const
{
    if (index < 0 || index > degree())
        throw std::out_of_range("Coefficient index out of range");
    return coefficients[index];
}

double Polynomial::operator()(double x) const
{
    double result = 0.0;
    for (int i = degree(); i >= 0; i--)
        result = result * x + coefficients[i];
    return result;
}

Polynomial Polynomial::operator+(const Polynomial& other) const
{
    const int resultDegree = std::max(degree(), other.degree());
    Polynomial result(resultDegree);
    for (int i = 0; i <= resultDegree; i++)
    {
        const double left = i <= degree() ? coefficients[i] : 0.0;
        const double right = i <= other.degree() ? other.coefficients[i] : 0.0;
        result.coefficients[i] = left + right;
    }
    result.normalize();
    return result;
}

Polynomial& Polynomial::operator+=(const Polynomial& other)
{
    *this = *this + other;
    return *this;
}

Polynomial Polynomial::operator-(const Polynomial& other) const
{
    const int resultDegree = std::max(degree(), other.degree());
    Polynomial result(resultDegree);
    for (int i = 0; i <= resultDegree; i++)
    {
        const double left = i <= degree() ? coefficients[i] : 0.0;
        const double right = i <= other.degree() ? other.coefficients[i] : 0.0;
        result.coefficients[i] = left - right;
    }
    result.normalize();
    return result;
}

Polynomial& Polynomial::operator-=(const Polynomial& other)
{
    *this = *this - other;
    return *this;
}

Polynomial Polynomial::operator*(const Polynomial& other) const
{
    Polynomial result(degree() + other.degree());
    for (int i = 0; i <= degree(); i++)
        for (int j = 0; j <= other.degree(); j++)
            result.coefficients[i + j] += coefficients[i] * other.coefficients[j];
    result.normalize();
    return result;
}

Polynomial& Polynomial::operator*=(const Polynomial& other)
{
    *this = *this * other;
    return *this;
}

Polynomial Polynomial::operator/(const Polynomial& other) const
{
    if (other.degree() == 0 && std::abs(other.coefficients[0]) < 1e-12)
        throw std::invalid_argument("Division by zero polynomial");

    if (degree() < other.degree())
        return Polynomial();

    Polynomial remainder(*this);
    Polynomial quotient(degree() - other.degree());

    while (remainder.degree() >= other.degree() && !(remainder.degree() == 0 && std::abs(remainder.coefficients[0]) < 1e-12))
    {
        const int degreeDifference = remainder.degree() - other.degree();

        const double factor = remainder.coefficients.back() / other.coefficients.back();

        quotient.coefficients[degreeDifference] = factor;

        for (int i = 0; i <= other.degree(); i++)
            remainder.coefficients[i + degreeDifference] -= factor * other.coefficients[i];

        remainder.normalize();

        if (remainder.degree() < other.degree()) break;
    }

    quotient.normalize();

    return quotient;
}

Polynomial& Polynomial::operator/=(const Polynomial& other)
{
    *this = *this / other;
    return *this;
}

bool Polynomial::operator==(const Polynomial& other) const
{
    if (degree() != other.degree()) return false;

    for (int i = 0; i <= degree(); i++)
        if (std::abs(coefficients[i] - other.coefficients[i]) > 1e-9) return false;

    return true;
}

bool Polynomial::operator!=(const Polynomial& other) const
{
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Polynomial& polynomial)
{
    bool first = true;

    for (int i = polynomial.degree(); i >= 0; i--)
    {
        const double coefficient = polynomial.coefficients[i];

        if (std::abs(coefficient) < 1e-12) continue;

        if (!first)
        {
            if (coefficient >= 0) os << " + ";
            else os << " - "; 
        }
        else if (coefficient < 0) os << "-";

        const double absCoefficient = std::abs(coefficient);

        if (i == 0)
            os << absCoefficient;
        else
        {
            if (std::abs(absCoefficient - 1.0) > 1e-12) 
                os << absCoefficient;
            os << "x";

            if (i > 1) os << "^" << i;
        }

        first = false;
    }

    if (first) os << "0";

    return os;
}

std::istream& operator>>(std::istream& is, Polynomial& polynomial)
{
    int degree;

    is >> degree;

    if (!is) return is;

    if (degree < 0)
    {
        is.setstate(std::ios::failbit);
        return is;
    }

    std::vector<double> coefficients(degree + 1);

    for (int i = 0; i <= degree; ++i)
    {
        is >> coefficients[i];
        if (!is) return is;
    }

    polynomial.coefficients = coefficients;
    polynomial.normalize();

    return is;
}