#include "Matrix.h"

#include <iostream>
#include <limits>

void printMenu()
{
    std::cout << "\n========== МАТРИЦА ==========\n";
    std::cout << "1. Ввести матрицу\n";
    std::cout << "2. Показать матрицу\n";
    std::cout << "3. Изменить количество строк\n";
    std::cout << "4. Изменить количество столбцов\n";
    std::cout << "5. Загрузить матрицу из файла\n";
    std::cout << "6. Получить подматрицу\n";
    std::cout << "7. Проверить тип матрицы\n";
    std::cout << "8. Транспонировать матрицу\n";
    std::cout << "9. Получить элемент\n";
    std::cout << "10. Изменить элемент\n";
    std::cout << "11. Сравнить с другой матрицей\n";
    std::cout << "12. Проверить копирование и присваивание\n";
    std::cout << "0. Выход\n";
    std::cout << "Выберите пункт: ";
}

int main()
{
    Matrix matrix;

    int choice;

    do
    {
        printMenu();
        std::cin >> choice;

        if (!std::cin)
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n'
            );

            std::cout << "Некорректный ввод.\n";
            continue;
        }

        try
        {
            switch (choice)
            {
                case 1:
                    std::cout << "Введите количество строк, "
                              << "столбцов и элементы:\n";

                    std::cin >> matrix;

                    std::cout << "Матрица:\n"
                              << matrix;
                    break;

                case 2:
                    std::cout << "Матрица:\n"
                              << matrix;

                    std::cout << "Размер: "
                              << matrix.getRows()
                              << " x "
                              << matrix.getCols()
                              << "\n";
                    break;

                case 3:
                {
                    int rows;

                    std::cout << "Новое количество строк: ";
                    std::cin >> rows;

                    matrix.setRows(rows);

                    std::cout << "Матрица:\n"
                              << matrix;
                    break;
                }

                case 4:
                {
                    int cols;

                    std::cout << "Новое количество столбцов: ";
                    std::cin >> cols;

                    matrix.setCols(cols);

                    std::cout << "Матрица:\n"
                              << matrix;
                    break;
                }

                case 5:
                {
                    std::string filename;

                    std::cout << "Имя файла: ";
                    std::cin >> filename;

                    matrix.loadFromFile(filename);

                    std::cout << "Матрица загружена:\n"
                              << matrix;
                    break;
                }

                case 6:
                {
                    int rows;
                    int cols;

                    std::cout << "Количество строк подматрицы: ";
                    std::cin >> rows;

                    std::cout << "Количество столбцов подматрицы: ";
                    std::cin >> cols;

                    Matrix result = matrix.submatrix(rows, cols);

                    std::cout << "Подматрица:\n"
                              << result;
                    break;
                }

                case 7:
                    std::cout << "\nТипы матрицы:\n";
                    std::cout << "Квадратная: "
                              << (matrix.isSquare() ? "да" : "нет")
                              << "\n";

                    std::cout << "Диагональная: "
                              << (matrix.isDiagonal() ? "да" : "нет")
                              << "\n";

                    std::cout << "Нулевая: "
                              << (matrix.isZero() ? "да" : "нет")
                              << "\n";

                    std::cout << "Единичная: "
                              << (matrix.isIdentity() ? "да" : "нет")
                              << "\n";

                    std::cout << "Симметрическая: "
                              << (matrix.isSymmetric() ? "да" : "нет")
                              << "\n";

                    std::cout << "Верхняя треугольная: "
                              << (matrix.isUpperTriangular()
                                      ? "да"
                                      : "нет")
                              << "\n";

                    std::cout << "Нижняя треугольная: "
                              << (matrix.isLowerTriangular()
                                      ? "да"
                                      : "нет")
                              << "\n";
                    break;

                case 8:
                {
                    Matrix transposed = matrix.transpose();

                    std::cout << "Транспонированная матрица:\n"
                              << transposed;
                    break;
                }

                case 9:
                {
                    int row;
                    int col;

                    std::cout << "Строка: ";
                    std::cin >> row;

                    std::cout << "Столбец: ";
                    std::cin >> col;

                    std::cout << "matrix("
                              << row << ", "
                              << col << ") = "
                              << matrix(row, col)
                              << "\n";
                    break;
                }

                case 10:
                {
                    int row;
                    int col;
                    double value;

                    std::cout << "Строка: ";
                    std::cin >> row;

                    std::cout << "Столбец: ";
                    std::cin >> col;

                    std::cout << "Новое значение: ";
                    std::cin >> value;

                    matrix(row, col) = value;

                    std::cout << "Матрица:\n"
                              << matrix;
                    break;
                }

                case 11:
                {
                    Matrix other;

                    std::cout << "Введите вторую матрицу:\n";
                    std::cin >> other;

                    std::cout << "matrix == other: "
                              << (matrix == other
                                      ? "true"
                                      : "false")
                              << "\n";

                    std::cout << "matrix != other: "
                              << (matrix != other
                                      ? "true"
                                      : "false")
                              << "\n";
                    break;
                }

                case 12:
                {
                    Matrix copy(matrix);

                    Matrix assigned;
                    assigned = matrix;

                    std::cout << "Копия:\n"
                              << copy;

                    std::cout << "После operator=:\n"
                              << assigned;

                    std::cout << "Копия равна исходной: "
                              << (copy == matrix
                                      ? "да"
                                      : "нет")
                              << "\n";

                    std::cout << "Присвоенная равна исходной: "
                              << (assigned == matrix
                                      ? "да"
                                      : "нет")
                              << "\n";
                    break;
                }

                case 0:
                    std::cout << "Выход.\n";
                    break;

                default:
                    std::cout << "Нет такого пункта.\n";
                    break;
            }
        }
        catch (const std::exception& error)
        {
            std::cout << "Ошибка: "
                      << error.what()
                      << "\n";
        }

    } while (choice != 0);

    return 0;
}