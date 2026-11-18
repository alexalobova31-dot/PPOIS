#include "Matrix.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace
{
    constexpr double EPSILON = 1e-9;
}

void Matrix::checkIndex(int row, int col) const
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        throw std::out_of_range("Matrix index out of range");
}

Matrix::Matrix() : rows(0), cols(0)
{
}

Matrix::Matrix(int rows, int cols)
{
    if (rows < 0 || cols < 0)
        throw std::invalid_argument("Matrix dimensions cannot be negative");

    this->rows = rows;
    this->cols = cols;

    data.resize(rows, std::vector<double>(cols, 0.0));
}

Matrix::Matrix(int rows, int cols, double value)
{
    if (rows < 0 || cols < 0)
        throw std::invalid_argument("Matrix dimensions cannot be negative");

    this->rows = rows;
    this->cols = cols;

    data.resize(rows, std::vector<double>(cols, value));
}

Matrix::Matrix(const Matrix& other)
    : data(other.data),
      rows(other.rows),
      cols(other.cols)
{
}

Matrix& Matrix::operator=(const Matrix& other)
{
    if (this != &other)
    {
        data = other.data;
        rows = other.rows;
        cols = other.cols;
    }

    return *this;
}

Matrix::~Matrix() = default;

int Matrix::getRows() const
{
    return rows;
}

int Matrix::getCols() const
{
    return cols;
}

void Matrix::setRows(int newRows)
{
    if (newRows < 0)
        throw std::invalid_argument("Number of rows cannot be negative");

    data.resize(newRows, std::vector<double>(cols, 0.0));

    rows = newRows;
}

void Matrix::setCols(int newCols)
{
    if (newCols < 0)
        throw std::invalid_argument("Number of columns cannot be negative");

    for (auto& row : data)
        row.resize(newCols, 0.0);

    cols = newCols;
}

double& Matrix::operator()(int row, int col)
{
    checkIndex(row, col);
    return data[row][col];
}

double Matrix::operator()(int row, int col) const
{
    checkIndex(row, col);
    return data[row][col];
}

void Matrix::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
        throw std::runtime_error("Cannot open file: " + filename);

    int newRows;
    int newCols;

    file >> newRows >> newCols;

    if (!file || newRows < 0 || newCols < 0)
        throw std::runtime_error("Invalid matrix dimensions in file");

    std::vector<std::vector<double>> newData(newRows, std::vector<double>(newCols));

    for (int i = 0; i < newRows; i++)
        for (int j = 0; j < newCols; j++)
        {
            file >> newData[i][j];
            if (!file)
                throw std::runtime_error("Invalid matrix data in file");
        }

    data = newData;
    rows = newRows;
    cols = newCols;
}

Matrix Matrix::submatrix(int subRows, int subCols) const
{
    if (subRows < 0 || subCols < 0)
        throw std::invalid_argument("Submatrix dimensions cannot be negative");

    if (subRows > rows || subCols > cols)
        throw std::invalid_argument("Submatrix is larger than the original matrix");

    Matrix result(subRows, subCols);

    for (int i = 0; i < subRows; i++)
        for (int j = 0; j < subCols; j++) result(i, j) = data[i][j];

    return result;
}

bool Matrix::isSquare() const
{
    return rows == cols;
}

bool Matrix::isDiagonal() const
{
    if (!isSquare())
        return false;

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (i != j && std::abs(data[i][j]) > EPSILON) return false;

    return true;
}

bool Matrix::isZero() const
{
    for (const auto& row : data)
        for (double value : row)
            if (std::abs(value) > EPSILON) return false;

    return true;
}

bool Matrix::isIdentity() const
{
    if (!isSquare())
        return false;

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
        {
            const double expected = (i == j) ? 1.0 : 0.0;

            if (std::abs(data[i][j] - expected) > EPSILON) return false;
        }

    return true;
}

bool Matrix::isSymmetric() const
{
    if (!isSquare())
        return false;

    for (int i = 0; i < rows; i++)
        for (int j = i + 1; j < cols; j++)
            if (std::abs(data[i][j] - data[j][i]) > EPSILON) return false;

    return true;
}

bool Matrix::isUpperTriangular() const
{
    if (!isSquare())
        return false;

    for (int i = 1; i < rows; i++)
        for (int j = 0; j < i; j++)
            if (std::abs(data[i][j]) > EPSILON) return false;
    return true;
}

bool Matrix::isLowerTriangular() const
{
    if (!isSquare())
        return false;

    for (int i = 0; i < rows; i++)
        for (int j = i + 1; j < cols; j++)
            if (std::abs(data[i][j]) > EPSILON) return false;

    return true;
}

Matrix Matrix::transpose() const
{
    Matrix result(cols, rows);

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) result(j, i) = data[i][j];

    return result;
}

bool Matrix::operator==(const Matrix& other) const
{
    if (rows != other.rows || cols != other.cols) return false;

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (std::abs(data[i][j] - other.data[i][j]) > EPSILON) return false;

    return true;
}

bool Matrix::operator!=(const Matrix& other) const
{
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Matrix& matrix)
{
    for (int i = 0; i < matrix.rows; i++)
    {
        for (int j = 0; j < matrix.cols; j++) os << std::setw(10) << matrix.data[i][j];
        os << '\n';
    }

    return os;
}

std::istream& operator>>(std::istream& is, Matrix& matrix)
{
    int newRows;
    int newCols;

    is >> newRows >> newCols;

    if (!is) return is;

    if (newRows < 0 || newCols < 0)
    {
        is.setstate(std::ios::failbit);
        return is;
    }

    std::vector<std::vector<double>> newData(
        newRows,
        std::vector<double>(newCols)
    );

    for (int i = 0; i < newRows; i++)
        for (int j = 0; j < newCols; j++)
        {
            is >> newData[i][j];

            if (!is)
            {
                return is;
            }
        }

    matrix.data = newData;
    matrix.rows = newRows;
    matrix.cols = newCols;

    return is;
}