#include "Console.h"
#include "Graph.h"
#include "Sorting.h"
#include "Student.h"

#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

class Console {
    std::istream& input_;
    std::ostream& output_;
    Graph<std::string> graph;
    Graph<std::string> saved;

    int read_number(const std::string& prompt, int minimum, int maximum)
    {
        output_ << prompt;
        std::string line;
        if (!std::getline(input_, line))
            throw std::runtime_error("Ввод завершён");
        std::istringstream input(line);
        int value;
        if (!(input >> value) || !(input >> std::ws).eof() || value < minimum || value > maximum)
            throw std::invalid_argument("Введите целое число от " + std::to_string(minimum)
                                        + " до " + std::to_string(maximum));
        return value;
    }

    Graph<std::string>::vertex_id read_id(const std::string& prompt = "ID вершины: ")
    {
        return static_cast<Graph<std::string>::vertex_id>(read_number(prompt, 0, 1000000));
    }

    template <class Iterator>
    void sort_values(Iterator first, Iterator last, int algorithm)
    {
        if (algorithm == 1) {
            lab::in_place_merge_sort(first, last);
        } else {
            std::mt19937 generator(42);
            lab::bogo_sort(first, last, generator);
        }
        std::for_each(first, last, [this](const auto& item) { output_ << item << ' '; });
        output_ << '\n';
    }

    void sorting_menu()
    {
        const int algorithm = read_number("1 — In-place merge sort, 2 — Bogosort: ", 1, 2);
        const int kind = read_number("1 — массив из 5 чисел, 2 — vector<int>, 3 — студенты: ", 1, 3);
        if (kind == 1) {
            int values[5];
            for (auto& value : values)
                value = read_number("Число: ", -1000000, 1000000);
            sort_values(std::begin(values), std::end(values), algorithm);
        } else if (kind == 2) {
            const int size = read_number("Количество элементов: ", 0, algorithm == 1 ? 1000 : 8);
            std::vector<int> values;
            for (int i = 0; i < size; ++i)
                values.push_back(read_number("Число: ", -1000000, 1000000));
            sort_values(values.begin(), values.end(), algorithm);
        } else {
            std::vector<Student> students{{"Анна", 9}, {"Иван", 6}, {"Олег", 8}, {"Мария", 8}};
            output_ << "Студенты по возрастанию оценки:\n";
            sort_values(students.begin(), students.end(), algorithm);
        }
    }

    void show_vertices(const Graph<std::string>& graph)
    {
        auto vertices = graph.vertices();
        for (auto it = vertices.begin(); it != vertices.end(); ++it)
            output_ << it.id() << ": " << *it << '\n';
    }

    void show_iterators(const Graph<std::string>& graph)
    {
        output_ << "Вершины в обратном порядке: ";
        for (auto it = graph.crbegin(); it != graph.crend(); ++it)
            output_ << *it << ' ';
        output_ << "\nРёбра в обратном порядке: ";
        auto edges = graph.edges();
        for (auto it = edges.crbegin(); it != edges.crend(); ++it)
            output_ << '(' << it->first << ", " << it->second << ") ";
        output_ << '\n';
        const auto id = read_id();
        auto adjacent = graph.adjacent_vertices(id);
        output_ << "Соседи, двунаправленный обход: ";
        for (auto it = adjacent.cbegin(); it != adjacent.cend(); it++)
            output_ << *it << ' ';
        output_ << "\nСоседи, обратный обход: ";
        for (auto it = adjacent.crbegin(); it != adjacent.crend(); ++it)
            output_ << *it << ' ';
        output_ << "\nСоседи, однонаправленный обход: ";
        for (const auto& value : graph.adjacent_forward(id))
            output_ << value << ' ';
        output_ << "\nИнцидентные рёбра в обратном порядке: ";
        auto incident = graph.incident_edges(id);
        for (auto it = incident.crbegin(); it != incident.crend(); ++it)
            output_ << '(' << it->first << ", " << it->second << ") ";
        output_ << '\n';
    }

    void graph_action(int command)
    {
        if (command == 2) {
            output_ << "Значение вершины: ";
            std::string value;
            if (!std::getline(input_, value))
                throw std::runtime_error("Ввод завершён");
            output_ << "Добавлена вершина " << graph.add_vertex(value) << '\n';
        } else if (command == 3 || command == 5 || command == 8) {
            const auto first = read_id("ID первой вершины: ");
            const auto second = read_id("ID второй вершины: ");
            if (command == 3)
                graph.add_edge(first, second);
            else if (command == 5)
                graph.remove_edge(first, second);
            else {
                output_ << "Ребро присутствует: " << graph.has_edge(first, second) << '\n';
                output_ << "Степень ребра: " << graph.edge_degree(first, second) << '\n';
            }
        } else if (command == 4) {
            graph.remove_vertex(read_id());
        } else if (command == 6) {
            output_ << graph << '\n';
            show_vertices(graph);
            output_ << "Вершин: " << graph.vertex_count() << ", рёбер: " << graph.edge_count()
                      << ", пустой: " << graph.empty() << '\n';
        } else if (command == 7) {
            const auto id = read_id();
            output_ << "Вершина присутствует: " << graph.has_vertex(id) << '\n';
            output_ << "Степень вершины: " << graph.vertex_degree(id) << '\n';
        } else if (command == 9) {
            show_iterators(graph);
        } else if (command == 10) {
            const auto id = read_id();
            auto vertices = graph.vertices();
            auto it = vertices.begin();
            while (it != vertices.end() && it.id() != id)
                ++it;
            graph.remove_vertex(it);
        } else if (command == 11) {
            const auto first = read_id("ID первой вершины: ");
            const auto second = read_id("ID второй вершины: ");
            auto edges = graph.edges();
            auto it = std::find_if(edges.begin(), edges.end(), [&](const auto& edge) {
                return edge.first == std::min(first, second) && edge.second == std::max(first, second);
            });
            graph.remove_edge(it);
        } else if (command == 12) {
            Graph<std::string> copy(graph);
            saved = copy;
            output_ << "Независимая копия сохранена\n";
        } else if (command == 13) {
            output_ << "Сравнение с сохранённой копией: == " << (graph == saved)
                      << ", != " << (graph != saved) << ", < " << (graph < saved)
                      << ", > " << (graph > saved) << ", <= " << (graph <= saved)
                      << ", >= " << (graph >= saved) << '\n';
        } else if (command == 14) {
            graph.clear();
        } else {
            const auto id = read_id();
            output_ << "Новое значение: ";
            std::string value;
            if (!std::getline(input_, value))
                throw std::runtime_error("Ввод завершён");
            graph.value(id) = value;
        }
    }

public:
    Console(std::istream& input, std::ostream& output) : input_(input), output_(output) {}

    int run()
    {
        output_ << std::boolalpha;
        for (;;) {
            output_ << "\nЛабораторная №4, вариант 11\n"
                      << "1. Сортировка\n2. Добавить вершину\n3. Добавить ребро\n"
                      << "4. Удалить вершину\n5. Удалить ребро\n6. Показать граф\n"
                      << "7. Наличие и степень вершины\n8. Наличие и степень ребра\n"
                      << "9. Итераторы\n10. Удалить вершину по итератору\n"
                      << "11. Удалить ребро по итератору\n12. Сохранить копию\n"
                      << "13. Сравнить с копией\n14. Очистить граф\n15. Изменить значение вершины\n0. Выход\n";
            try {
                const auto command = read_number("Выбор: ", 0, 15);
                if (command == 0)
                    return 0;
                if (command == 1)
                    sorting_menu();
                else
                    graph_action(command);
            } catch (const std::exception& error) {
                if (input_.eof())
                    return 0;
                if (input_.bad()) {
                    output_ << "Ошибка чтения ввода\n";
                    return 1;
                }
                output_ << "Ошибка: " << error.what() << '\n';
            }
        }
    }

};

}

int run_console(std::istream& input, std::ostream& output)
{
    return Console(input, output).run();
}
