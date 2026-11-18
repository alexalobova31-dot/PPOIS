#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Класс вещественной матрицы.
 *
 * Матрица хранит вещественные элементы типа double.
 */
class Matrix
{
private:
    std::vector<std::vector<double>> data;
    int rows;
    int cols;

    /**
     * @brief Проверяет корректность индексов.
     * @param row Номер строки.
     * @param col Номер столбца.
     */
    void checkIndex(int row, int col) const;

public:
    /**
     * @brief Конструктор пустой матрицы.
     */
    Matrix();

    /**
     * @brief Конструктор матрицы заданного размера.
     * @param rows Количество строк.
     * @param cols Количество столбцов.
     */
    Matrix(int rows, int cols);

    /**
     * @brief Конструктор с заполнением заданным значением.
     * @param rows Количество строк.
     * @param cols Количество столбцов.
     * @param value Начальное значение элементов.
     */
    Matrix(int rows, int cols, double value);

    /**
     * @brief Конструктор копирования.
     * @param other Копируемая матрица.
     */
    Matrix(const Matrix& other);

    /**
     * @brief Оператор присваивания.
     * @param other Матрица для копирования.
     * @return Ссылка на текущий объект.
     */
    Matrix& operator=(const Matrix& other);

    /**
     * @brief Деструктор.
     */
    ~Matrix();

    /**
     * @brief Возвращает количество строк.
     * @return Количество строк.
     */
    int getRows() const;

    /**
     * @brief Возвращает количество столбцов.
     * @return Количество столбцов.
     */
    int getCols() const;

    /**
     * @brief Изменяет количество строк.
     * @param newRows Новое количество строк.
     */
    void setRows(int newRows);

    /**
     * @brief Изменяет количество столбцов.
     * @param newCols Новое количество столбцов.
     */
    void setCols(int newCols);

    /**
     * @brief Получает элемент матрицы.
     * @param row Номер строки.
     * @param col Номер столбца.
     * @return Ссылка на элемент.
     */
    double& operator()(int row, int col);

    /**
     * @brief Получает элемент константной матрицы.
     * @param row Номер строки.
     * @param col Номер столбца.
     * @return Значение элемента.
     */
    double operator()(int row, int col) const;

    /**
     * @brief Загружает матрицу из файла.
     *
     * Формат файла:
     * первая строка — количество строк и столбцов,
     * затем элементы матрицы.
     *
     * @param filename Имя файла.
     */
    void loadFromFile(const std::string& filename);

    /**
     * @brief Извлекает подматрицу из верхнего левого угла.
     *
     * @param subRows Количество строк подматрицы.
     * @param subCols Количество столбцов подматрицы.
     * @return Подматрица.
     */
    Matrix submatrix(int subRows, int subCols) const;

    /**
     * @brief Проверяет, является ли матрица квадратной.
     * @return true, если матрица квадратная.
     */
    bool isSquare() const;

    /**
     * @brief Проверяет, является ли матрица диагональной.
     * @return true, если матрица диагональная.
     */
    bool isDiagonal() const;

    /**
     * @brief Проверяет, является ли матрица нулевой.
     * @return true, если все элементы равны нулю.
     */
    bool isZero() const;

    /**
     * @brief Проверяет, является ли матрица единичной.
     * @return true, если матрица единичная.
     */
    bool isIdentity() const;

    /**
     * @brief Проверяет, является ли матрица симметрической.
     * @return true, если матрица симметрическая.
     */
    bool isSymmetric() const;

    /**
     * @brief Проверяет, является ли матрица верхней треугольной.
     * @return true, если матрица верхняя треугольная.
     */
    bool isUpperTriangular() const;

    /**
     * @brief Проверяет, является ли матрица нижней треугольной.
     * @return true, если матрица нижняя треугольная.
     */
    bool isLowerTriangular() const;

    /**
     * @brief Возвращает транспонированную матрицу.
     * @return Транспонированная матрица.
     */
    Matrix transpose() const;

    /**
     * @brief Сравнивает две матрицы.
     * @param other Вторая матрица.
     * @return true, если матрицы равны.
     */
    bool operator==(const Matrix& other) const;

    /**
     * @brief Проверяет неравенство двух матриц.
     * @param other Вторая матрица.
     * @return true, если матрицы не равны.
     */
    bool operator!=(const Matrix& other) const;

    /**
     * @brief Выводит матрицу в поток.
     * @param os Поток вывода.
     * @param matrix Матрица.
     * @return Поток вывода.
     */
    friend std::ostream& operator<<(std::ostream& os,
                                    const Matrix& matrix);

    /**
     * @brief Вводит матрицу из потока.
     *
     * Сначала вводится количество строк и столбцов,
     * затем элементы матрицы.
     *
     * @param is Поток ввода.
     * @param matrix Матрица.
     * @return Поток ввода.
     */
    friend std::istream& operator>>(std::istream& is,
                                    Matrix& matrix);
};

#endif