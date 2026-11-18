#include "Matrix.h"

#include <UnitTest++/UnitTest++.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
    constexpr double EPSILON = 1e-9;

    /**
    * @brief Преобразует матрицу в строку.
    * @param matrix Матрица.
    * @return Строковое представление матрицы.
    */
    std::string toString(const Matrix& matrix)
    {
        std::ostringstream stream;
        stream << matrix;
        return stream.str();
    }

    /**
    * @brief Временный файл, удаляемый после использования.
    */
    class TemporaryFile
    {
    private:
        std::string filename;

    public:
        /**
        * @brief Создает файл с заданным содержимым.
        * @param filename Имя файла.
        * @param content Содержимое файла.
        */
        TemporaryFile(const std::string& filename, const std::string& content)
            : filename(filename)
        {
            std::ofstream file(filename);
            file << content;
        }

        /**
        * @brief Удаляет файл.
        */
        ~TemporaryFile()
        {
            std::remove(filename.c_str());
        }

        /**
        * @brief Возвращает имя файла.
        * @return Имя файла.
        */
        const std::string& name() const
        {
            return filename;
        }
    };

    /**
    * @brief Создает матрицу из потока ввода.
    * @param text Текстовое представление матрицы.
    * @return Считанная матрица.
    */
    Matrix fromString(const std::string& text)
    {
        std::istringstream stream(text);
        Matrix matrix;
        stream >> matrix;
        return matrix;
    }
}

SUITE(MatrixConstructors)
{
    TEST(DefaultConstructorCreatesEmptyMatrix)
    {
        const Matrix matrix;

        CHECK_EQUAL(0, matrix.getRows());
        CHECK_EQUAL(0, matrix.getCols());
    }

    TEST(SizeConstructorCreatesMatrixFilledWithZeros)
    {
        const Matrix matrix(2, 3);

        CHECK_EQUAL(2, matrix.getRows());
        CHECK_EQUAL(3, matrix.getCols());

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 3; j++) CHECK_CLOSE(0.0, matrix(i, j), EPSILON);
    }

    TEST(SizeConstructorThrowsOnNegativeRows)
    {
        CHECK_THROW(Matrix(-1, 2), std::invalid_argument);
    }

    TEST(SizeConstructorThrowsOnNegativeCols)
    {
        CHECK_THROW(Matrix(2, -1), std::invalid_argument);
    }

    TEST(ValueConstructorFillsMatrixWithValue)
    {
        const Matrix matrix(2, 2, 5.0);

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++) CHECK_CLOSE(5.0, matrix(i, j), EPSILON);
    }

    TEST(ValueConstructorThrowsOnNegativeDimensions)
    {
        CHECK_THROW(Matrix(-1, -1, 1.0), std::invalid_argument);
    }

    TEST(CopyConstructorCopiesElements)
    {
        const Matrix source(2, 2, 3.0);
        const Matrix copy(source);

        CHECK(copy == source);
    }

    TEST(CopyConstructorCreatesIndependentObject)
    {
        Matrix source(2, 2, 3.0);
        Matrix copy(source);

        copy(0, 0) = 9.0;

        CHECK_CLOSE(3.0, source(0, 0), EPSILON);
        CHECK_CLOSE(9.0, copy(0, 0), EPSILON);
    }

    TEST(AssignmentOperatorCopiesElements)
    {
        const Matrix source(2, 3, 1.0);
        Matrix target;

        target = source;

        CHECK(target == source);
        CHECK_EQUAL(2, target.getRows());
        CHECK_EQUAL(3, target.getCols());
    }

    TEST(SelfAssignmentKeepsMatrixUnchanged)
    {
        Matrix matrix(2, 2, 4.0);
        const Matrix expected(2, 2, 4.0);
        Matrix& reference = matrix;

        matrix = reference;

        CHECK(matrix == expected);
    }
}

SUITE(MatrixSize)
{
    TEST(SetRowsIncreasesNumberOfRows)
    {
        Matrix matrix(2, 3, 1.0);

        matrix.setRows(4);

        CHECK_EQUAL(4, matrix.getRows());
        CHECK_CLOSE(1.0, matrix(1, 0), EPSILON);
        CHECK_CLOSE(0.0, matrix(3, 0), EPSILON);
    }

    TEST(SetRowsDecreasesNumberOfRows)
    {
        Matrix matrix(3, 2, 1.0);

        matrix.setRows(1);

        CHECK_EQUAL(1, matrix.getRows());
        CHECK_THROW(matrix(1, 0), std::out_of_range);
    }

    TEST(SetRowsThrowsOnNegativeValue)
    {
        Matrix matrix(2, 2);

        CHECK_THROW(matrix.setRows(-1), std::invalid_argument);
    }

    TEST(SetColsIncreasesNumberOfCols)
    {
        Matrix matrix(2, 2, 1.0);

        matrix.setCols(4);

        CHECK_EQUAL(4, matrix.getCols());
        CHECK_CLOSE(1.0, matrix(0, 1), EPSILON);
        CHECK_CLOSE(0.0, matrix(0, 3), EPSILON);
    }

    TEST(SetColsDecreasesNumberOfCols)
    {
        Matrix matrix(2, 3, 1.0);

        matrix.setCols(1);

        CHECK_EQUAL(1, matrix.getCols());
        CHECK_THROW(matrix(0, 1), std::out_of_range);
    }

    TEST(SetColsThrowsOnNegativeValue)
    {
        Matrix matrix(2, 2);

        CHECK_THROW(matrix.setCols(-1), std::invalid_argument);
    }
}

SUITE(MatrixAccess)
{
    TEST(CallOperatorReturnsModifiableElement)
    {
        Matrix matrix(2, 2);

        matrix(1, 1) = 7.0;

        CHECK_CLOSE(7.0, matrix(1, 1), EPSILON);
    }

    TEST(CallOperatorThrowsOnNegativeIndex)
    {
        Matrix matrix(2, 2);

        CHECK_THROW(matrix(-1, 0), std::out_of_range);
        CHECK_THROW(matrix(0, -1), std::out_of_range);
    }

    TEST(CallOperatorThrowsOnTooLargeIndex)
    {
        Matrix matrix(2, 2);

        CHECK_THROW(matrix(2, 0), std::out_of_range);
        CHECK_THROW(matrix(0, 2), std::out_of_range);
    }

    TEST(ConstCallOperatorThrowsOutOfRange)
    {
        const Matrix matrix(2, 2);

        CHECK_THROW(matrix(2, 2), std::out_of_range);
    }

    TEST(AnyAccessToEmptyMatrixThrows)
    {
        Matrix matrix;

        CHECK_THROW(matrix(0, 0), std::out_of_range);
    }
}

SUITE(MatrixFile)
{
    TEST(LoadFromFileReadsDimensionsAndElements)
    {
        const TemporaryFile file("matrix_test_valid.txt", "2 3\n1 2 3\n4 5 6\n");
        Matrix matrix;

        matrix.loadFromFile(file.name());

        CHECK_EQUAL(2, matrix.getRows());
        CHECK_EQUAL(3, matrix.getCols());
        CHECK_CLOSE(1.0, matrix(0, 0), EPSILON);
        CHECK_CLOSE(6.0, matrix(1, 2), EPSILON);
    }

    TEST(LoadFromFileThrowsOnMissingFile)
    {
        Matrix matrix;

        CHECK_THROW(matrix.loadFromFile("no_such_matrix_file.txt"), std::runtime_error);
    }

    TEST(LoadFromFileThrowsOnNegativeDimensions)
    {
        const TemporaryFile file("matrix_test_negative.txt", "-2 3\n");
        Matrix matrix;

        CHECK_THROW(matrix.loadFromFile(file.name()), std::runtime_error);
    }

    TEST(LoadFromFileThrowsOnMissingDimensions)
    {
        const TemporaryFile file("matrix_test_empty.txt", "");
        Matrix matrix;

        CHECK_THROW(matrix.loadFromFile(file.name()), std::runtime_error);
    }

    TEST(LoadFromFileThrowsOnMissingElements)
    {
        const TemporaryFile file("matrix_test_short.txt", "2 2\n1 2 3\n");
        Matrix matrix;

        CHECK_THROW(matrix.loadFromFile(file.name()), std::runtime_error);
    }

    TEST(LoadFromFileThrowsOnNotANumber)
    {
        const TemporaryFile file("matrix_test_text.txt", "2 2\n1 2\n3 x\n");
        Matrix matrix;

        CHECK_THROW(matrix.loadFromFile(file.name()), std::runtime_error);
    }

    TEST(FailedLoadKeepsMatrixUnchanged)
    {
        const TemporaryFile file("matrix_test_broken.txt", "2 2\n1 2\n");
        Matrix matrix(1, 1, 8.0);

        CHECK_THROW(matrix.loadFromFile(file.name()), std::runtime_error);

        CHECK_EQUAL(1, matrix.getRows());
        CHECK_EQUAL(1, matrix.getCols());
        CHECK_CLOSE(8.0, matrix(0, 0), EPSILON);
    }
}

SUITE(MatrixSubmatrix)
{
    TEST(SubmatrixTakesUpperLeftCorner)
    {
        Matrix matrix(3, 3);

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) matrix(i, j) = i * 3.0 + j;

        const Matrix result = matrix.submatrix(2, 2);

        CHECK_EQUAL(2, result.getRows());
        CHECK_EQUAL(2, result.getCols());
        CHECK_CLOSE(0.0, result(0, 0), EPSILON);
        CHECK_CLOSE(1.0, result(0, 1), EPSILON);
        CHECK_CLOSE(3.0, result(1, 0), EPSILON);
        CHECK_CLOSE(4.0, result(1, 1), EPSILON);
    }

    TEST(SubmatrixOfFullSizeEqualsOriginal)
    {
        const Matrix matrix(2, 3, 1.0);

        CHECK(matrix == matrix.submatrix(2, 3));
    }

    TEST(SubmatrixOfZeroSizeIsEmpty)
    {
        const Matrix matrix(2, 2, 1.0);
        const Matrix result = matrix.submatrix(0, 0);

        CHECK_EQUAL(0, result.getRows());
        CHECK_EQUAL(0, result.getCols());
    }

    TEST(SubmatrixThrowsOnNegativeDimensions)
    {
        const Matrix matrix(2, 2);

        CHECK_THROW(matrix.submatrix(-1, 1), std::invalid_argument);
        CHECK_THROW(matrix.submatrix(1, -1), std::invalid_argument);
    }

    TEST(SubmatrixThrowsWhenLargerThanOriginal)
    {
        const Matrix matrix(2, 2);

        CHECK_THROW(matrix.submatrix(3, 2), std::invalid_argument);
        CHECK_THROW(matrix.submatrix(2, 3), std::invalid_argument);
    }
}

SUITE(MatrixProperties)
{
    TEST(SquareMatrixIsSquare)
    {
        const Matrix matrix(3, 3);

        CHECK(matrix.isSquare());
    }

    TEST(RectangularMatrixIsNotSquare)
    {
        const Matrix matrix(2, 3);

        CHECK(!matrix.isSquare());
    }

    TEST(DiagonalMatrixIsDiagonal)
    {
        Matrix matrix(3, 3);

        matrix(0, 0) = 1.0;
        matrix(1, 1) = 2.0;
        matrix(2, 2) = 3.0;

        CHECK(matrix.isDiagonal());
    }

    TEST(MatrixWithNonZeroOutsideDiagonalIsNotDiagonal)
    {
        Matrix matrix(2, 2);

        matrix(0, 1) = 1.0;

        CHECK(!matrix.isDiagonal());
    }

    TEST(RectangularMatrixIsNotDiagonal)
    {
        const Matrix matrix(2, 3);

        CHECK(!matrix.isDiagonal());
    }

    TEST(MatrixOfZerosIsZero)
    {
        const Matrix matrix(3, 2);

        CHECK(matrix.isZero());
    }

    TEST(MatrixWithNonZeroElementIsNotZero)
    {
        Matrix matrix(2, 2);

        matrix(1, 0) = 0.5;

        CHECK(!matrix.isZero());
    }

    TEST(IdentityMatrixIsIdentity)
    {
        Matrix matrix(3, 3);

        for (int i = 0; i < 3; i++) matrix(i, i) = 1.0;

        CHECK(matrix.isIdentity());
    }

    TEST(DiagonalMatrixWithOtherValuesIsNotIdentity)
    {
        Matrix matrix(2, 2);

        matrix(0, 0) = 1.0;
        matrix(1, 1) = 2.0;

        CHECK(!matrix.isIdentity());
    }

    TEST(RectangularMatrixIsNotIdentity)
    {
        const Matrix matrix(2, 3);

        CHECK(!matrix.isIdentity());
    }

    TEST(SymmetricMatrixIsSymmetric)
    {
        Matrix matrix(2, 2);

        matrix(0, 0) = 1.0;
        matrix(0, 1) = 2.0;
        matrix(1, 0) = 2.0;
        matrix(1, 1) = 3.0;

        CHECK(matrix.isSymmetric());
    }

    TEST(AsymmetricMatrixIsNotSymmetric)
    {
        Matrix matrix(2, 2);

        matrix(0, 1) = 1.0;
        matrix(1, 0) = 2.0;

        CHECK(!matrix.isSymmetric());
    }

    TEST(RectangularMatrixIsNotSymmetric)
    {
        const Matrix matrix(2, 3);

        CHECK(!matrix.isSymmetric());
    }

    TEST(UpperTriangularMatrixIsUpperTriangular)
    {
        Matrix matrix(3, 3);

        matrix(0, 0) = 1.0;
        matrix(0, 1) = 2.0;
        matrix(0, 2) = 3.0;
        matrix(1, 1) = 4.0;
        matrix(1, 2) = 5.0;
        matrix(2, 2) = 6.0;

        CHECK(matrix.isUpperTriangular());
        CHECK(!matrix.isLowerTriangular());
    }

    TEST(LowerTriangularMatrixIsLowerTriangular)
    {
        Matrix matrix(3, 3);

        matrix(0, 0) = 1.0;
        matrix(1, 0) = 2.0;
        matrix(1, 1) = 3.0;
        matrix(2, 0) = 4.0;
        matrix(2, 1) = 5.0;
        matrix(2, 2) = 6.0;

        CHECK(matrix.isLowerTriangular());
        CHECK(!matrix.isUpperTriangular());
    }

    TEST(DiagonalMatrixIsBothTriangular)
    {
        Matrix matrix(2, 2);

        matrix(0, 0) = 1.0;
        matrix(1, 1) = 2.0;

        CHECK(matrix.isUpperTriangular());
        CHECK(matrix.isLowerTriangular());
    }

    TEST(RectangularMatrixIsNotTriangular)
    {
        const Matrix matrix(2, 3);

        CHECK(!matrix.isUpperTriangular());
        CHECK(!matrix.isLowerTriangular());
    }
}

SUITE(MatrixTranspose)
{
    TEST(TransposeSwapsDimensions)
    {
        const Matrix matrix(2, 3);
        const Matrix result = matrix.transpose();

        CHECK_EQUAL(3, result.getRows());
        CHECK_EQUAL(2, result.getCols());
    }

    TEST(TransposeSwapsElements)
    {
        Matrix matrix(2, 2);

        matrix(0, 1) = 5.0;
        matrix(1, 0) = 7.0;

        const Matrix result = matrix.transpose();

        CHECK_CLOSE(7.0, result(0, 1), EPSILON);
        CHECK_CLOSE(5.0, result(1, 0), EPSILON);
    }

    TEST(DoubleTransposeReturnsOriginalMatrix)
    {
        Matrix matrix(2, 3);

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 3; j++) matrix(i, j) = i * 3.0 + j;

        CHECK(matrix == matrix.transpose().transpose());
    }

    TEST(TransposeOfSymmetricMatrixEqualsOriginal)
    {
        Matrix matrix(2, 2);

        matrix(0, 0) = 1.0;
        matrix(0, 1) = 2.0;
        matrix(1, 0) = 2.0;
        matrix(1, 1) = 3.0;

        CHECK(matrix == matrix.transpose());
    }
}

SUITE(MatrixComparison)
{
    TEST(EqualMatricesAreEqual)
    {
        const Matrix left(2, 2, 1.0);
        const Matrix right(2, 2, 1.0);

        CHECK(left == right);
        CHECK(!(left != right));
    }

    TEST(MatricesWithDifferentDimensionsAreNotEqual)
    {
        const Matrix left(2, 2);
        const Matrix right(2, 3);

        CHECK(left != right);
    }

    TEST(MatricesWithDifferentElementsAreNotEqual)
    {
        const Matrix left(2, 2, 1.0);
        const Matrix right(2, 2, 2.0);

        CHECK(left != right);
    }

    TEST(SmallElementDifferenceIsIgnored)
    {
        const Matrix left(2, 2, 1.0);
        const Matrix right(2, 2, 1.0 + 1e-12);

        CHECK(left == right);
    }

    TEST(EmptyMatricesAreEqual)
    {
        const Matrix left;
        const Matrix right;

        CHECK(left == right);
    }
}

SUITE(MatrixStreams)
{
    TEST(OutputWritesRowsOnSeparateLines)
    {
        Matrix matrix(2, 1);

        matrix(0, 0) = 1.0;
        matrix(1, 0) = 2.0;

        CHECK_EQUAL("         1\n         2\n", toString(matrix));
    }

    TEST(OutputOfEmptyMatrixIsEmpty)
    {
        const Matrix matrix;

        CHECK_EQUAL("", toString(matrix));
    }

    TEST(InputReadsDimensionsAndElements)
    {
        const Matrix matrix = fromString("2 2\n1 2\n3 4\n");

        CHECK_EQUAL(2, matrix.getRows());
        CHECK_EQUAL(2, matrix.getCols());
        CHECK_CLOSE(1.0, matrix(0, 0), EPSILON);
        CHECK_CLOSE(4.0, matrix(1, 1), EPSILON);
    }

    TEST(InputOfEmptyMatrixIsAllowed)
    {
        std::istringstream stream("0 0");
        Matrix matrix(2, 2);

        stream >> matrix;

        CHECK(!stream.fail());
        CHECK_EQUAL(0, matrix.getRows());
        CHECK_EQUAL(0, matrix.getCols());
    }

    TEST(InputOfNegativeDimensionsSetsFailbit)
    {
        std::istringstream stream("-1 2");
        Matrix matrix;

        stream >> matrix;

        CHECK(stream.fail());
    }

    TEST(InputOfNotANumberSetsFailbit)
    {
        std::istringstream stream("abc");
        Matrix matrix;

        stream >> matrix;

        CHECK(stream.fail());
    }

    TEST(InputWithMissingElementsSetsFailbit)
    {
        std::istringstream stream("2 2\n1 2\n");
        Matrix matrix;

        stream >> matrix;

        CHECK(stream.fail());
    }

    TEST(FailedInputKeepsMatrixUnchanged)
    {
        std::istringstream stream("2 2\n1 2\n");
        Matrix matrix(1, 1, 9.0);

        stream >> matrix;

        CHECK_EQUAL(1, matrix.getRows());
        CHECK_EQUAL(1, matrix.getCols());
        CHECK_CLOSE(9.0, matrix(0, 0), EPSILON);
    }

    TEST(WrittenMatrixCanBeReadBack)
    {
        Matrix source(2, 2);

        source(0, 0) = 1.0;
        source(0, 1) = 2.0;
        source(1, 0) = 3.0;
        source(1, 1) = 4.0;

        const Matrix copy = fromString("2 2\n" + toString(source));

        CHECK(copy == source);
    }
}

int main()
{
    return UnitTest::RunAllTests();
}
