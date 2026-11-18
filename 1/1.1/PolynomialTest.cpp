#include "Polynomial.h"

#include <UnitTest++/UnitTest++.h>

#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
    constexpr double EPSILON = 1e-9;

    /**
    * @brief Преобразует многочлен в строку.
    * @param polynomial Многочлен.
    * @return Строковое представление многочлена.
    */
    std::string toString(const Polynomial& polynomial)
    {
        std::ostringstream stream;
        stream << polynomial;
        return stream.str();
    }
}

SUITE(PolynomialConstructors)
{
    TEST(DefaultConstructorCreatesZeroPolynomial)
    {
        const Polynomial polynomial;

        CHECK_EQUAL(0, polynomial.degree());
        CHECK_CLOSE(0.0, polynomial[0], EPSILON);
    }

    TEST(DegreeConstructorCreatesRequestedNumberOfCoefficients)
    {
        const Polynomial polynomial(3);

        CHECK_EQUAL(3, polynomial.degree());

        for (int i = 0; i <= 3; i++) CHECK_CLOSE(0.0, polynomial[i], EPSILON);
    }

    TEST(DegreeConstructorThrowsOnNegativeDegree)
    {
        CHECK_THROW(Polynomial(-1), std::invalid_argument);
    }

    TEST(VectorConstructorKeepsCoefficients)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 2.0, 3.0});

        CHECK_EQUAL(2, polynomial.degree());
        CHECK_CLOSE(1.0, polynomial[0], EPSILON);
        CHECK_CLOSE(2.0, polynomial[1], EPSILON);
        CHECK_CLOSE(3.0, polynomial[2], EPSILON);
    }

    TEST(VectorConstructorRemovesLeadingZeroCoefficients)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 2.0, 0.0, 0.0});

        CHECK_EQUAL(1, polynomial.degree());
        CHECK_CLOSE(2.0, polynomial[1], EPSILON);
    }

    TEST(VectorConstructorFromEmptyVectorCreatesZeroPolynomial)
    {
        const Polynomial polynomial(std::vector<double>{});

        CHECK_EQUAL(0, polynomial.degree());
        CHECK_CLOSE(0.0, polynomial[0], EPSILON);
    }

    TEST(CopyConstructorCopiesCoefficients)
    {
        const Polynomial source(std::vector<double>{1.0, 2.0});
        const Polynomial copy(source);

        CHECK(copy == source);
    }

    TEST(CopyConstructorCreatesIndependentObject)
    {
        Polynomial source(std::vector<double>{1.0, 2.0});
        Polynomial copy(source);

        copy[0] = 5.0;

        CHECK_CLOSE(1.0, source[0], EPSILON);
        CHECK_CLOSE(5.0, copy[0], EPSILON);
    }

    TEST(AssignmentOperatorCopiesCoefficients)
    {
        const Polynomial source(std::vector<double>{1.0, 2.0, 3.0});
        Polynomial target;

        target = source;

        CHECK(target == source);
    }

    TEST(SelfAssignmentKeepsPolynomialUnchanged)
    {
        Polynomial polynomial(std::vector<double>{1.0, 2.0});
        const Polynomial expected(std::vector<double>{1.0, 2.0});
        Polynomial& reference = polynomial;

        polynomial = reference;

        CHECK(polynomial == expected);
    }
}

SUITE(PolynomialAccess)
{
    TEST(IndexOperatorReturnsModifiableCoefficient)
    {
        Polynomial polynomial(2);

        polynomial[2] = 7.0;

        CHECK_CLOSE(7.0, polynomial[2], EPSILON);
    }

    TEST(IndexOperatorThrowsOnNegativeIndex)
    {
        Polynomial polynomial(2);

        CHECK_THROW(polynomial[-1], std::out_of_range);
    }

    TEST(IndexOperatorThrowsOnTooLargeIndex)
    {
        Polynomial polynomial(2);

        CHECK_THROW(polynomial[3], std::out_of_range);
    }

    TEST(ConstIndexOperatorThrowsOutOfRange)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 2.0});

        CHECK_THROW(polynomial[2], std::out_of_range);
        CHECK_THROW(polynomial[-1], std::out_of_range);
    }
}

SUITE(PolynomialEvaluation)
{
    TEST(EvaluatesConstantPolynomial)
    {
        const Polynomial polynomial(std::vector<double>{5.0});

        CHECK_CLOSE(5.0, polynomial(10.0), EPSILON);
    }

    TEST(EvaluatesLinearPolynomial)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 2.0});

        CHECK_CLOSE(7.0, polynomial(3.0), EPSILON);
    }

    TEST(EvaluatesQuadraticPolynomial)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 0.0, 2.0});

        CHECK_CLOSE(19.0, polynomial(3.0), EPSILON);
    }

    TEST(EvaluatesAtZeroReturnsFreeTerm)
    {
        const Polynomial polynomial(std::vector<double>{4.0, 5.0, 6.0});

        CHECK_CLOSE(4.0, polynomial(0.0), EPSILON);
    }

    TEST(EvaluatesAtNegativeArgument)
    {
        const Polynomial polynomial(std::vector<double>{0.0, 1.0, 1.0});

        CHECK_CLOSE(2.0, polynomial(-2.0), EPSILON);
    }
}

SUITE(PolynomialArithmetic)
{
    TEST(AdditionOfPolynomialsOfEqualDegree)
    {
        const Polynomial left(std::vector<double>{1.0, 2.0});
        const Polynomial right(std::vector<double>{3.0, 4.0});
        const Polynomial expected(std::vector<double>{4.0, 6.0});

        CHECK(expected == left + right);
    }

    TEST(AdditionOfPolynomialsOfDifferentDegree)
    {
        const Polynomial left(std::vector<double>{1.0, 2.0, 3.0});
        const Polynomial right(std::vector<double>{1.0});
        const Polynomial expected(std::vector<double>{2.0, 2.0, 3.0});

        CHECK(expected == left + right);
    }

    TEST(AdditionRemovesLeadingZeroCoefficients)
    {
        const Polynomial left(std::vector<double>{1.0, 2.0});
        const Polynomial right(std::vector<double>{1.0, -2.0});
        const Polynomial sum = left + right;

        CHECK_EQUAL(0, sum.degree());
        CHECK_CLOSE(2.0, sum[0], EPSILON);
    }

    TEST(AdditionWithAssignmentChangesLeftOperand)
    {
        Polynomial left(std::vector<double>{1.0, 2.0});
        const Polynomial right(std::vector<double>{3.0, 4.0});
        const Polynomial expected(std::vector<double>{4.0, 6.0});

        left += right;

        CHECK(expected == left);
    }

    TEST(SubtractionOfPolynomials)
    {
        const Polynomial left(std::vector<double>{5.0, 7.0});
        const Polynomial right(std::vector<double>{2.0, 3.0});
        const Polynomial expected(std::vector<double>{3.0, 4.0});

        CHECK(expected == left - right);
    }

    TEST(SubtractionOfEqualPolynomialsGivesZero)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 2.0, 3.0});
        const Polynomial difference = polynomial - polynomial;

        CHECK_EQUAL(0, difference.degree());
        CHECK_CLOSE(0.0, difference[0], EPSILON);
    }

    TEST(SubtractionWithAssignmentChangesLeftOperand)
    {
        Polynomial left(std::vector<double>{5.0, 7.0});
        const Polynomial right(std::vector<double>{2.0, 3.0});
        const Polynomial expected(std::vector<double>{3.0, 4.0});

        left -= right;

        CHECK(expected == left);
    }

    TEST(MultiplicationOfLinearPolynomials)
    {
        const Polynomial left(std::vector<double>{1.0, 1.0});
        const Polynomial right(std::vector<double>{-1.0, 1.0});
        const Polynomial expected(std::vector<double>{-1.0, 0.0, 1.0});

        CHECK(expected == left * right);
    }

    TEST(MultiplicationByZeroPolynomialGivesZero)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 2.0, 3.0});
        const Polynomial zero;
        const Polynomial product = polynomial * zero;

        CHECK_EQUAL(0, product.degree());
        CHECK_CLOSE(0.0, product[0], EPSILON);
    }

    TEST(MultiplicationSumsDegrees)
    {
        const Polynomial left(std::vector<double>{0.0, 0.0, 1.0});
        const Polynomial right(std::vector<double>{0.0, 1.0});

        CHECK_EQUAL(3, (left * right).degree());
    }

    TEST(MultiplicationWithAssignmentChangesLeftOperand)
    {
        Polynomial left(std::vector<double>{1.0, 1.0});
        const Polynomial right(std::vector<double>{-1.0, 1.0});
        const Polynomial expected(std::vector<double>{-1.0, 0.0, 1.0});

        left *= right;

        CHECK(expected == left);
    }

    TEST(DivisionWithoutRemainder)
    {
        const Polynomial dividend(std::vector<double>{-1.0, 0.0, 1.0});
        const Polynomial divisor(std::vector<double>{-1.0, 1.0});
        const Polynomial expected(std::vector<double>{1.0, 1.0});

        CHECK(expected == dividend / divisor);
    }

    TEST(DivisionWithRemainderReturnsQuotient)
    {
        const Polynomial dividend(std::vector<double>{1.0, 0.0, 1.0});
        const Polynomial divisor(std::vector<double>{0.0, 1.0});
        const Polynomial expected(std::vector<double>{0.0, 1.0});

        CHECK(expected == dividend / divisor);
    }

    TEST(DivisionByConstantScalesCoefficients)
    {
        const Polynomial dividend(std::vector<double>{2.0, 4.0});
        const Polynomial divisor(std::vector<double>{2.0});
        const Polynomial expected(std::vector<double>{1.0, 2.0});

        CHECK(expected == dividend / divisor);
    }

    TEST(DivisionBySmallerDegreeDividendGivesZero)
    {
        const Polynomial dividend(std::vector<double>{1.0});
        const Polynomial divisor(std::vector<double>{1.0, 1.0});
        const Polynomial quotient = dividend / divisor;

        CHECK_EQUAL(0, quotient.degree());
        CHECK_CLOSE(0.0, quotient[0], EPSILON);
    }

    TEST(DivisionByZeroPolynomialThrows)
    {
        const Polynomial dividend(std::vector<double>{1.0, 2.0});
        const Polynomial zero;

        CHECK_THROW(dividend / zero, std::invalid_argument);
    }

    TEST(DivisionWithAssignmentChangesLeftOperand)
    {
        Polynomial dividend(std::vector<double>{-1.0, 0.0, 1.0});
        const Polynomial divisor(std::vector<double>{-1.0, 1.0});
        const Polynomial expected(std::vector<double>{1.0, 1.0});

        dividend /= divisor;

        CHECK(expected == dividend);
    }

    TEST(DivisionWithAssignmentByZeroPolynomialThrows)
    {
        Polynomial dividend(std::vector<double>{1.0, 2.0});
        const Polynomial zero;

        CHECK_THROW(dividend /= zero, std::invalid_argument);
    }
}

SUITE(PolynomialComparison)
{
    TEST(EqualPolynomialsAreEqual)
    {
        const Polynomial left(std::vector<double>{1.0, 2.0});
        const Polynomial right(std::vector<double>{1.0, 2.0});

        CHECK(left == right);
        CHECK(!(left != right));
    }

    TEST(PolynomialsWithDifferentDegreesAreNotEqual)
    {
        const Polynomial left(std::vector<double>{1.0, 2.0});
        const Polynomial right(std::vector<double>{1.0, 2.0, 3.0});

        CHECK(left != right);
    }

    TEST(PolynomialsWithDifferentCoefficientsAreNotEqual)
    {
        const Polynomial left(std::vector<double>{1.0, 2.0});
        const Polynomial right(std::vector<double>{1.0, 3.0});

        CHECK(left != right);
    }

    TEST(SmallCoefficientDifferenceIsIgnored)
    {
        const Polynomial left(std::vector<double>{1.0, 2.0});
        const Polynomial right(std::vector<double>{1.0 + 1e-12, 2.0});

        CHECK(left == right);
    }
}

SUITE(PolynomialStreams)
{
    TEST(OutputOfZeroPolynomial)
    {
        const Polynomial polynomial;

        CHECK_EQUAL("0", toString(polynomial));
    }

    TEST(OutputOfConstantPolynomial)
    {
        const Polynomial polynomial(std::vector<double>{3.0});

        CHECK_EQUAL("3", toString(polynomial));
    }

    TEST(OutputStartsFromHighestDegree)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 2.0, 3.0});

        CHECK_EQUAL("3x^2 + 2x + 1", toString(polynomial));
    }

    TEST(OutputSkipsZeroCoefficients)
    {
        const Polynomial polynomial(std::vector<double>{1.0, 0.0, 3.0});

        CHECK_EQUAL("3x^2 + 1", toString(polynomial));
    }

    TEST(OutputHidesUnitCoefficient)
    {
        const Polynomial polynomial(std::vector<double>{0.0, 1.0});

        CHECK_EQUAL("x", toString(polynomial));
    }

    TEST(OutputOfNegativeCoefficients)
    {
        const Polynomial polynomial(std::vector<double>{-1.0, -2.0});

        CHECK_EQUAL("-2x - 1", toString(polynomial));
    }

    TEST(InputReadsDegreeAndCoefficients)
    {
        std::istringstream stream("2 1 2 3");
        Polynomial polynomial;

        stream >> polynomial;

        CHECK(!stream.fail());
        CHECK_EQUAL(2, polynomial.degree());
        CHECK_CLOSE(1.0, polynomial[0], EPSILON);
        CHECK_CLOSE(2.0, polynomial[1], EPSILON);
        CHECK_CLOSE(3.0, polynomial[2], EPSILON);
    }

    TEST(InputRemovesLeadingZeroCoefficients)
    {
        std::istringstream stream("3 1 2 0 0");
        Polynomial polynomial;

        stream >> polynomial;

        CHECK_EQUAL(1, polynomial.degree());
    }

    TEST(InputOfNegativeDegreeSetsFailbit)
    {
        std::istringstream stream("-2 1 2");
        Polynomial polynomial;

        stream >> polynomial;

        CHECK(stream.fail());
    }

    TEST(InputOfNotANumberSetsFailbit)
    {
        std::istringstream stream("abc");
        Polynomial polynomial;

        stream >> polynomial;

        CHECK(stream.fail());
    }

    TEST(InputWithMissingCoefficientsSetsFailbit)
    {
        std::istringstream stream("2 1");
        Polynomial polynomial;

        stream >> polynomial;

        CHECK(stream.fail());
    }

    TEST(FailedInputKeepsPolynomialUnchanged)
    {
        std::istringstream stream("2 1");
        Polynomial polynomial(std::vector<double>{7.0});

        stream >> polynomial;

        CHECK_EQUAL(0, polynomial.degree());
        CHECK_CLOSE(7.0, polynomial[0], EPSILON);
    }

    TEST(WrittenPolynomialCanBeReadBack)
    {
        std::istringstream stream("2 1 2 3");
        Polynomial polynomial;

        stream >> polynomial;

        CHECK_EQUAL("3x^2 + 2x + 1", toString(polynomial));
    }
}

int main()
{
    return UnitTest::RunAllTests();
}
