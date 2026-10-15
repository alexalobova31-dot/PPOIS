#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <iostream>
#include <vector>

/**
 * @brief Класс многочлена от одной переменной.
 *
 * Представляет многочлен вида:
 * a0 + a1*x + a2*x^2 + ... + an*x^n.
 *
 * Коэффициенты хранятся в порядке возрастания степени:
 * coefficients[0] - свободный член,
 * coefficients[1] - коэффициент при x,
 * coefficients[n] - коэффициент при x^n.
 */
class Polynomial
{
    private:
        std::vector<double> coefficients;

        /**
        * @brief Удаляет старшие нулевые коэффициенты.
        */
        void normalize();

    public:
        /**
        * @brief Конструктор по умолчанию.
        */
        Polynomial();

        /**
        * @brief Создает многочлен заданной степени.
        * @param degree Степень многочлена.
        */
        explicit Polynomial(int degree);

        /**
        * @brief Создает многочлен из массива коэффициентов.
        * @param coefficients Коэффициенты многочлена.
        */
        explicit Polynomial(const std::vector<double>& coefficients);

        /**
        * @brief Конструктор копирования.
        * @param other Копируемый многочлен.
        */
        Polynomial(const Polynomial& other);

        /**
        * @brief Оператор присваивания.
        * @param other Многочлен для копирования.
        * @return Ссылка на текущий объект.
        */
        Polynomial& operator=(const Polynomial& other);

        /**
        * @brief Деструктор.
        */
        ~Polynomial();

        /**
        * @brief Возвращает степень многочлена.
        * @return Степень многочлена.
        */
        int degree() const;

        /**
        * @brief Получает коэффициент.
        * @param index Степень соответствующего члена.
        * @return Значение коэффициента.
        */
        double& operator[](int index);

        /**
        * @brief Получает коэффициент для константного объекта.
        * @param index Степень соответствующего члена.
        * @return Значение коэффициента.
        */
        const double& operator[](int index) const;

        /**
        * @brief Вычисляет значение многочлена.
        * @param x Значение переменной.
        * @return Значение многочлена в точке x.
        */
        double operator()(double x) const;

        /**
        * @brief Сложение многочленов.
        * @param other Второй многочлен.
        * @return Результат сложения.
        */
        Polynomial operator+(const Polynomial& other) const;

        /**
        * @brief Сложение с присваиванием.
        * @param other Второй многочлен.
        * @return Ссылка на текущий объект.
        */
        Polynomial& operator+=(const Polynomial& other);

        /**
        * @brief Вычитание многочленов.
        * @param other Второй многочлен.
        * @return Результат вычитания.
        */
        Polynomial operator-(const Polynomial& other) const;

        /**
        * @brief Вычитание с присваиванием.
        * @param other Второй многочлен.
        * @return Ссылка на текущий объект.
        */
        Polynomial& operator-=(const Polynomial& other);

        /**
        * @brief Умножение многочленов.
        * @param other Второй многочлен.
        * @return Результат умножения.
        */
        Polynomial operator*(const Polynomial& other) const;

        /**
        * @brief Умножение с присваиванием.
        * @param other Второй многочлен.
        * @return Ссылка на текущий объект.
        */
        Polynomial& operator*=(const Polynomial& other);

        /**
        * @brief Деление многочленов.
        *
        * Возвращает частное от деления.
        *
        * @param other Делитель.
        * @return Частное.
        */
        Polynomial operator/(const Polynomial& other) const;

        /**
        * @brief Деление с присваиванием.
        * @param other Делитель.
        * @return Ссылка на текущий объект.
        */
        Polynomial& operator/=(const Polynomial& other);

        /**
        * @brief Сравнивает два многочлена.
        * @param other Второй многочлен.
        * @return true, если многочлены равны.
        */
        bool operator==(const Polynomial& other) const;

        /**
        * @brief Проверяет неравенство двух многочленов.
        * @param other Второй многочлен.
        * @return true, если многочлены не равны.
        */
        bool operator!=(const Polynomial& other) const;

        /**
        * @brief Выводит многочлен в поток.
        * @param os Поток вывода.
        * @param polynomial Многочлен.
        * @return Поток вывода.
        */
        friend std::ostream& operator<<(std::ostream& os, const Polynomial& polynomial);

        /**
        * @brief Вводит многочлен из потока.
        *
        * Сначала вводится степень, затем коэффициенты от a0 до an.
        *
        * @param is Поток ввода.
        * @param polynomial Многочлен.
        * @return Поток ввода.
        */
        friend std::istream& operator>>(std::istream& is, Polynomial& polynomial);
};

#endif