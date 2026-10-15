#include "Polynomial.h"

#include <iostream>
#include <limits>
#include <vector>

void printMenu()
{
    std::cout << "\n========== МНОГОЧЛЕН ==========\n";
    std::cout << "1. Ввести многочлен A\n";
    std::cout << "2. Ввести многочлен B\n";
    std::cout << "3. Показать A и B\n";
    std::cout << "4. Вычислить A(x)\n";
    std::cout << "5. A + B\n";
    std::cout << "6. A - B\n";
    std::cout << "7. A * B\n";
    std::cout << "8. A / B\n";
    std::cout << "9. A += B\n";
    std::cout << "10. A -= B\n";
    std::cout << "11. A *= B\n";
    std::cout << "12. A /= B\n";
    std::cout << "13. Сравнить A и B\n";
    std::cout << "14. Получить коэффициент A[i]\n";
    std::cout << "0. Выход\n";
    std::cout << "Выберите пункт: ";
}

int main()
{
    Polynomial A;
    Polynomial B;

    int choice;

    do
    {
        printMenu();
        std::cin >> choice;

        if (!std::cin)
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Некорректный ввод.\n";
            continue;
        }

        try
        {
            switch (choice)
            {
                case 1:
                    std::cout << "Введите степень, затем коэффициенты " << "от a0 до an:\n";
                    std::cin >> A;
                    std::cout << "A = " << A << "\n";
                    break;

                case 2:
                    std::cout << "Введите степень, затем коэффициенты " << "от a0 до an:\n";
                    std::cin >> B;
                    std::cout << "B = " << B << "\n";
                    break;

                case 3:
                    std::cout << "A = " << A << "\n";
                    std::cout << "B = " << B << "\n";
                    std::cout << "degree(A) = " << A.degree() << "\n";
                    std::cout << "degree(B) = " << B.degree() << "\n";
                    break;

                case 4:
                {
                    double x;

                    std::cout << "Введите x: ";
                    std::cin >> x;

                    std::cout << "A(" << x << ") = " << A(x) << "\n";
                    break;
                }

                case 5:
                    std::cout << "A + B = " << A + B << "\n";
                    break;

                case 6:
                    std::cout << "A - B = " << A - B << "\n";
                    break;

                case 7:
                    std::cout << "A * B = " << A * B << "\n";
                    break;

                case 8:
                    std::cout << "A / B = " << A / B << "\n";
                    break;

                case 9:
                    A += B;
                    std::cout << "A = " << A << "\n";
                    break;

                case 10:
                    A -= B;
                    std::cout << "A = " << A << "\n";
                    break;

                case 11:
                    A *= B;
                    std::cout << "A = " << A << "\n";
                    break;

                case 12:
                    A /= B;
                    std::cout << "A = " << A << "\n";
                    break;

                case 13:
                    std::cout << "A == B: " << (A == B ? "true" : "false") << "\n";

                    std::cout << "A != B: " << (A != B ? "true" : "false") << "\n";
                    break;

                case 14:
                {
                    int index;

                    std::cout << "Введите индекс коэффициента: ";
                    std::cin >> index;

                    std::cout << "A[" << index << "] = " << A[index] << "\n";
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
            std::cout << "Ошибка: " << error.what() << "\n";
        }

    } while (choice != 0);

    return 0;
}