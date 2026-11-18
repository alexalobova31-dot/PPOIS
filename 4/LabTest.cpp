#include "Console.h"
#include "Graph.h"
#include "Sorting.h"
#include "Student.h"
#include <UnitTest++/UnitTest++.h>

#include <array>
#include <list>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>

namespace {

template <class Range>
auto collect(const Range& range)
{
    using T = typename std::iterator_traits<decltype(range.begin())>::value_type;
    return std::vector<T>(range.begin(), range.end());
}

template <class Range>
void check_bidirectional(const Range& range)
{
    const auto expected = collect(range);
    CHECK(std::vector<typename decltype(expected)::value_type>(range.cbegin(), range.cend()) == expected);
    auto reversed = expected;
    std::reverse(reversed.begin(), reversed.end());
    using Value = typename decltype(expected)::value_type;
    CHECK(std::vector<Value>(range.rbegin(), range.rend()) == reversed);
    CHECK(std::vector<Value>(range.crbegin(), range.crend()) == reversed);
    auto it = range.begin();
    CHECK_THROW(--it, std::out_of_range);
    if (it != range.end()) {
        auto old = it++;
        CHECK(*old == expected.front());
        old = it--;
        CHECK(it == range.begin());
        CHECK(old != it);
        auto last = range.end();
        --last;
        CHECK(*last == expected.back());
    }
    auto end = range.end();
    CHECK_THROW(*end, std::out_of_range);
    CHECK_THROW(++end, std::out_of_range);
}

Graph<int> example_graph()
{
    Graph<int> graph;
    for (int value : {10, 20, 30, 40})
        graph.add_vertex(value);
    graph.add_edge(0, 1);
    graph.add_edge(2, 0);
    graph.add_edge(1, 2);
    return graph;
}

struct ThrowingValue {
    int value;
    static bool fail;
    explicit ThrowingValue(int number) : value(number) {}
    ThrowingValue(const ThrowingValue& other) : value(other.value)
    {
        if (fail)
            throw std::runtime_error("copy failed");
    }
    ThrowingValue(ThrowingValue&&) = default;
};
bool ThrowingValue::fail = false;

}

TEST(MergeSortArraysVectorsAndLists)
{
    int values[]{5, -1, 0, 5, 3, -7};
    lab::in_place_merge_sort(std::begin(values), std::end(values));
    CHECK(std::is_sorted(std::begin(values), std::end(values)));
    std::vector<std::string> words{"z", "ab", "a", "ab"};
    lab::in_place_merge_sort(words.begin(), words.end());
    CHECK(words == std::vector<std::string>({"a", "ab", "ab", "z"}));
    std::list<int> linked{2, 4, 1, 3};
    lab::in_place_merge_sort(linked.begin(), linked.end(), std::greater<>{});
    CHECK(collect(linked) == std::vector<int>({4, 3, 2, 1}));
    std::array<int, 0> empty{};
    lab::in_place_merge_sort(empty.begin(), empty.end());
    int single[]{7};
    lab::in_place_merge_sort(std::begin(single), std::end(single));
    CHECK_EQUAL(7, single[0]);
}

TEST(MergeSortMatchesStandardSortOnRandomInputs)
{
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(-15, 15);
    for (int size = 0; size < 100; ++size) {
        std::vector<int> values(static_cast<std::size_t>(size));
        for (auto& value : values)
            value = distribution(generator);
        auto expected = values;
        std::stable_sort(expected.begin(), expected.end());
        lab::in_place_merge_sort(values.begin(), values.end());
        CHECK(values == expected);
        std::stable_sort(expected.begin(), expected.end(), std::greater<>{});
        lab::in_place_merge_sort(values.begin(), values.end(), std::greater<>{});
        CHECK(values == expected);
    }
}

TEST(MergeSortCustomObjectsIsStable)
{
    std::vector<Student> students{{"A", 8}, {"B", 6}, {"C", 8}, {"D", 9}};
    lab::in_place_merge_sort(students.begin(), students.end());
    CHECK_EQUAL(std::string("B"), students[0].name());
    CHECK_EQUAL(std::string("A"), students[1].name());
    CHECK_EQUAL(std::string("C"), students[2].name());
    CHECK_EQUAL(9, students[3].grade());
    lab::in_place_merge_sort(students.begin(), students.end(),
                             [](const auto& first, const auto& second) { return first.name() > second.name(); });
    CHECK_EQUAL(std::string("D"), students.front().name());
    std::ostringstream out;
    out << students.front();
    CHECK_EQUAL(std::string("D (9)"), out.str());
    CHECK(Student("A", 8) == Student("A", 8));
    CHECK(!(Student("A", 8) == Student("A", 7)));
}

TEST(BogosortHandlesArraysVectorsObjectsAndLimits)
{
    std::mt19937 generator(17);
    int values[]{3, 1, 2};
    lab::bogo_sort(std::begin(values), std::end(values), generator);
    CHECK(std::is_sorted(std::begin(values), std::end(values)));
    std::vector<int> empty;
    lab::bogo_sort(empty.begin(), empty.end(), generator, 0);
    std::array<int, 1> single{4};
    lab::bogo_sort(single.begin(), single.end(), generator, 0);
    std::vector<int> equal{1, 1, 1};
    lab::bogo_sort(equal.begin(), equal.end(), generator, 0);
    std::vector<int> reversed{2, 1};
    CHECK_THROW(lab::bogo_sort(reversed.begin(), reversed.end(), generator, 0), std::runtime_error);
    CHECK(reversed == std::vector<int>({2, 1}));
    lab::bogo_sort(reversed.begin(), reversed.end(), generator, 0, std::greater<>{});
    std::vector<Student> students{{"A", 9}, {"B", 5}, {"C", 7}};
    lab::bogo_sort(students.begin(), students.end(), generator);
    CHECK_EQUAL(5, students.front().grade());
    CHECK_EQUAL(9, students.back().grade());
    std::array<Student, 2> custom{{Student("Z", 1), Student("A", 9)}};
    lab::bogo_sort(custom.begin(), custom.end(), generator, 100,
                   [](const auto& first, const auto& second) { return first.name() < second.name(); });
    CHECK_EQUAL(std::string("A"), custom.front().name());
    std::vector<int> repeated{2, 1, 1, 0};
    lab::bogo_sort(repeated.begin(), repeated.end(), generator);
    CHECK(repeated == std::vector<int>({0, 1, 1, 2}));
}

TEST(EmptyGraphAndInvalidOperations)
{
    Graph<int> graph;
    CHECK(graph.empty());
    CHECK_EQUAL(0u, graph.vertex_count());
    CHECK_EQUAL(0u, graph.edge_count());
    CHECK(!graph.has_vertex(0));
    CHECK(!graph.has_edge(0, 1));
    CHECK_THROW(graph.value(0), std::out_of_range);
    CHECK_THROW(std::as_const(graph).value(0), std::out_of_range);
    CHECK_THROW(graph.vertex_degree(0), std::out_of_range);
    CHECK_THROW(graph.edge_degree(0, 1), std::out_of_range);
    CHECK_THROW(graph.add_edge(0, 1), std::out_of_range);
    CHECK_THROW(graph.remove_edge(0, 1), std::out_of_range);
    CHECK_THROW(graph.remove_vertex(0), std::out_of_range);
    CHECK_THROW(graph.adjacent_vertices(0), std::out_of_range);
    CHECK_THROW(graph.incident_edges(0), std::out_of_range);
    CHECK_THROW(graph.adjacent_forward(0), std::out_of_range);
    check_bidirectional(graph.vertices());
    check_bidirectional(graph.edges());
    check_bidirectional(std::as_const(graph).vertices());
    check_bidirectional(std::as_const(graph).edges());
    Graph<int> copy(graph);
    CHECK(copy == graph);
    graph.clear();
    graph.clear();
    CHECK(graph.empty());
    std::ostringstream out;
    out << graph;
    CHECK_EQUAL(std::string("Вершины: \nРёбра (ID): "), out.str());
}

TEST(GraphEdgesDegreesAndDuplicateValidation)
{
    auto graph = example_graph();
    CHECK(!graph.empty());
    CHECK_EQUAL(4u, graph.vertex_count());
    CHECK_EQUAL(3u, graph.edge_count());
    CHECK(graph.has_vertex(0));
    CHECK(graph.has_edge(0, 2));
    CHECK(graph.has_edge(2, 0));
    CHECK(!graph.has_edge(0, 3));
    CHECK(!graph.has_edge(0, 99));
    CHECK_EQUAL(2u, graph.vertex_degree(0));
    CHECK_EQUAL(0u, graph.vertex_degree(3));
    CHECK_EQUAL(2u, graph.edge_degree(0, 1));
    CHECK_THROW(graph.add_edge(0, 0), std::invalid_argument);
    CHECK_THROW(graph.add_edge(1, 0), std::invalid_argument);
    CHECK_THROW(graph.add_edge(0, 99), std::out_of_range);
    CHECK_EQUAL(3u, graph.edge_count());
    graph.add_edge(3, 1);
    CHECK_EQUAL(3u, graph.edge_degree(0, 1));
    graph.remove_edge(1, 3);
    CHECK_EQUAL(3u, graph.edge_count());
    CHECK_EQUAL(0u, graph.vertex_degree(3));
    CHECK_THROW(graph.remove_edge(1, 3), std::out_of_range);
    graph.remove_vertex(0);
    CHECK(!graph.has_vertex(0));
    CHECK_EQUAL(1u, graph.edge_count());
    CHECK_EQUAL(1u, graph.vertex_degree(1));
    CHECK_EQUAL(1u, graph.vertex_degree(2));
    CHECK(graph.has_edge(1, 2));
    graph.remove_vertex(3);
    graph.remove_vertex(1);
    CHECK_EQUAL(0u, graph.edge_count());
    CHECK_EQUAL(30, graph.value(2));
    graph.clear();
    CHECK_EQUAL(4u, graph.add_vertex(50));
}

TEST(CopyAssignmentAndIndependentOwnership)
{
    auto graph = example_graph();
    graph.remove_vertex(3);
    Graph<int> copy(graph);
    CHECK(copy == graph);
    CHECK(copy.has_edge(0, 2));
    copy.value(0) = 99;
    copy.remove_edge(0, 1);
    CHECK_EQUAL(10, graph.value(0));
    CHECK(graph.has_edge(0, 1));
    Graph<int> assigned;
    assigned.add_vertex(-1);
    auto old = assigned.begin();
    assigned = graph;
    CHECK_THROW(*old, std::logic_error);
    CHECK(assigned == graph);
    auto current = assigned.begin();
    auto* alias = &assigned;
    assigned = *alias;
    CHECK_EQUAL(10, *current);
    graph.clear();
    CHECK_EQUAL(3u, assigned.edge_count());
    CHECK_EQUAL(4u, assigned.add_vertex(4));
    CHECK_EQUAL(0u, assigned.vertex_degree(4));
    copy = Graph<int>();
    CHECK(copy.empty());
}

TEST(AssignmentPreservesTargetWhenElementCopyThrows)
{
    Graph<ThrowingValue> source;
    source.add_vertex(ThrowingValue(1));
    source.add_vertex(ThrowingValue(2));
    source.add_edge(0, 1);
    Graph<ThrowingValue> target;
    target.add_vertex(ThrowingValue(9));
    auto current = target.begin();
    ThrowingValue::fail = true;
    CHECK_THROW(target = source, std::runtime_error);
    ThrowingValue::fail = false;
    CHECK_EQUAL(1u, target.vertex_count());
    CHECK_EQUAL(9, current->value);
    target = source;
    CHECK_EQUAL(2u, target.vertex_count());
    CHECK(target.has_edge(0, 1));
}

TEST(GraphComparisonUsesValuesAndTopology)
{
    Graph<int> first;
    first.add_vertex(1);
    first.add_vertex(2);
    Graph<int> second(first);
    CHECK(first == second);
    CHECK(!(first != second));
    CHECK(first <= second);
    CHECK(first >= second);
    CHECK(!(first < second));
    CHECK(!(first > second));
    second.add_edge(0, 1);
    CHECK(first != second);
    CHECK(first < second);
    CHECK(second > first);
    CHECK(!(first >= second));
    CHECK(!(second <= first));
    first.value(0) = 3;
    CHECK(first > second);
    Graph<int> shifted;
    shifted.add_vertex(99);
    shifted.remove_vertex(0);
    shifted.add_vertex(1);
    shifted.add_vertex(2);
    shifted.add_edge(2, 1);
    CHECK(shifted == second);
    Graph<int> reordered;
    for (int value : {1, 2, 3})
        reordered.add_vertex(value);
    reordered.add_edge(0, 2);
    reordered.add_edge(0, 1);
    Graph<int> other;
    for (int value : {1, 2, 3})
        other.add_vertex(value);
    other.add_edge(1, 0);
    other.add_edge(2, 0);
    CHECK(other == reordered);
}

TEST(AllGraphRangesSupportBidirectionalReverseAndConstIteration)
{
    auto graph = example_graph();
    const auto& constant = graph;
    check_bidirectional(graph.vertices());
    check_bidirectional(graph.edges());
    check_bidirectional(constant.vertices());
    check_bidirectional(constant.edges());
    for (auto id : {0u, 1u, 2u, 3u}) {
        check_bidirectional(graph.adjacent_vertices(id));
        check_bidirectional(graph.incident_edges(id));
        check_bidirectional(constant.adjacent_vertices(id));
        check_bidirectional(constant.incident_edges(id));
    }
    CHECK(collect(graph) == std::vector<int>({10, 20, 30, 40}));
    CHECK(std::vector<int>(graph.begin(), graph.end()) == collect(graph));
    CHECK(std::vector<int>(constant.begin(), constant.end()) == collect(graph));
    CHECK(std::vector<int>(graph.cbegin(), graph.cend()) == collect(graph));
    CHECK(std::vector<int>(graph.rbegin(), graph.rend()) == std::vector<int>({40, 30, 20, 10}));
    CHECK(std::vector<int>(constant.rbegin(), constant.rend()) == std::vector<int>({40, 30, 20, 10}));
    CHECK(std::vector<int>(graph.crbegin(), graph.crend()) == std::vector<int>({40, 30, 20, 10}));
    CHECK(collect(graph.adjacent_vertices(0)) == std::vector<int>({20, 30}));
    CHECK(collect(graph.adjacent_vertices(1)) == std::vector<int>({10, 30}));
    using Edge = Graph<int>::edge_type;
    CHECK(collect(graph.edges()) == std::vector<Edge>({{0, 1}, {0, 2}, {1, 2}}));
    CHECK(collect(graph.incident_edges(1)) == std::vector<Edge>({{0, 1}, {1, 2}}));
    CHECK_EQUAL(0u, graph.edges().begin()->first);
    CHECK_EQUAL(1u, constant.edges().begin()->second);
    static_assert(std::is_same_v<decltype(*constant.begin()), const int&>);
    static_assert(std::is_same_v<decltype(*graph.begin()), int&>);
    static_assert(std::is_same_v<decltype(*graph.edges().begin()), const Edge&>);
    static_assert(std::is_same_v<typename Graph<int>::iterator::iterator_category,
                                  std::bidirectional_iterator_tag>);
}

TEST(ForwardAdjacentIteratorsAndMutableValues)
{
    auto graph = example_graph();
    auto range = graph.adjacent_forward(0);
    CHECK(collect(range) == std::vector<int>({20, 30}));
    auto it = range.begin();
    auto old = it++;
    CHECK_EQUAL(20, *old);
    CHECK_EQUAL(30, *it);
    CHECK_EQUAL(2u, it.id());
    *it = 31;
    CHECK_EQUAL(31, graph.value(2));
    Graph<int>::const_forward_iterator constant = it;
    CHECK(constant == it);
    CHECK(it == constant);
    CHECK_EQUAL(31, *constant.operator->());
    CHECK(std::vector<int>(range.cbegin(), range.cend()) == std::vector<int>({20, 31}));
    const auto& const_graph = graph;
    CHECK(collect(const_graph.adjacent_forward(0)) == std::vector<int>({20, 31}));
    CHECK(collect(const_graph.adjacent_forward(3)).empty());
    static_assert(std::is_same_v<typename decltype(it)::iterator_category, std::forward_iterator_tag>);
    static_assert(!std::is_constructible_v<Graph<int>::iterator, Graph<int>::const_iterator>);
    auto adjacent = graph.adjacent_vertices(0);
    *adjacent.begin() = 21;
    CHECK_EQUAL(21, const_graph.value(1));
    Graph<int>::const_iterator vertex = graph.begin();
    CHECK(vertex == graph.begin());
    CHECK_EQUAL(10, *vertex.operator->());
    Graph<int>::const_edge_iterator edge = graph.edges().begin();
    CHECK(edge == graph.edges().begin());
    CHECK_EQUAL(0u, edge->first);
}

TEST(IteratorEraseAndForeignIterators)
{
    auto graph = example_graph();
    auto other = example_graph();
    CHECK_THROW(graph.remove_vertex(other.begin()), std::invalid_argument);
    CHECK_THROW(graph.remove_edge(other.edges().begin()), std::invalid_argument);
    CHECK_THROW(graph.remove_vertex(graph.adjacent_vertices(0).begin()), std::invalid_argument);
    CHECK_THROW(graph.remove_vertex(graph.end()), std::out_of_range);
    CHECK_THROW(graph.remove_edge(graph.edges().end()), std::out_of_range);
    graph.remove_edge(graph.edges().begin());
    CHECK(!graph.has_edge(0, 1));
    auto constant_edge = std::as_const(graph).incident_edges(0).begin();
    graph.remove_edge(constant_edge);
    CHECK(!graph.has_edge(0, 2));
    CHECK_EQUAL(1u, graph.edge_count());
    auto vertices = graph.vertices();
    auto it = vertices.begin();
    ++it;
    graph.remove_vertex(it);
    CHECK(!graph.has_vertex(1));
    CHECK_EQUAL(0u, graph.edge_count());
    graph.remove_vertex(std::as_const(graph).begin());
    CHECK(!graph.has_vertex(0));
    CHECK_EQUAL(2u, graph.vertex_count());
}

TEST(InvalidatedAndDefaultIteratorsThrow)
{
    Graph<int>::iterator unbound;
    Graph<int>::iterator another;
    CHECK(unbound == another);
    CHECK_THROW(*unbound, std::logic_error);
    CHECK_THROW(++unbound, std::logic_error);
    CHECK_THROW(--unbound, std::logic_error);
    CHECK_THROW(unbound.id(), std::logic_error);
    auto graph = example_graph();
    auto vertices = graph.vertices();
    auto vertex = vertices.begin();
    auto edge = graph.edges().begin();
    graph.add_vertex(50);
    CHECK_THROW(*vertex, std::logic_error);
    CHECK_THROW(*edge, std::logic_error);
    CHECK_THROW(vertices.begin(), std::logic_error);
    CHECK_THROW(vertices.end(), std::logic_error);
    CHECK_THROW(graph.remove_vertex(vertex), std::logic_error);
    auto current = graph.begin();
    graph.add_edge(3, 4);
    CHECK_THROW(++current, std::logic_error);
    current = graph.begin();
    graph.remove_edge(3, 4);
    CHECK_THROW(--current, std::logic_error);
    current = graph.begin();
    graph.remove_vertex(4);
    CHECK_THROW(*current, std::logic_error);
    current = graph.begin();
    graph.clear();
    CHECK_THROW(*current, std::logic_error);
}

TEST(CustomObjectsDuplicateValuesAndTextOutput)
{
    Graph<Student> students;
    auto first = students.add_vertex(Student("A", 7));
    auto second = students.add_vertex(Student("A", 7));
    CHECK(first != second);
    students.add_edge(first, second);
    students.begin()->grade();
    CHECK_EQUAL(7, std::as_const(students).value(first).grade());
    Graph<Student> copy(students);
    CHECK(copy == students);
    std::ostringstream out;
    out << students;
    CHECK_EQUAL(std::string("Вершины: A (7) A (7) \nРёбра (ID): (0, 1) "), out.str());
    Graph<std::string> text;
    text.add_vertex("hello");
    text.add_vertex("world");
    text.add_edge(0, 1);
    text.begin()->append("!");
    CHECK_EQUAL(std::string("hello!"), text.value(0));
}

TEST(RandomGraphOperationsMatchReferenceModel)
{
    Graph<int> graph;
    std::set<std::size_t> vertices;
    std::set<Graph<int>::edge_type> edges;
    std::mt19937 generator(2026);
    for (int step = 0; step < 400; ++step) {
        const auto operation = generator() % 4;
        if (operation == 0 || vertices.empty()) {
            vertices.insert(graph.add_vertex(step));
        } else {
            auto selected = vertices.begin();
            std::advance(selected, generator() % vertices.size());
            const auto first = *selected;
            if (operation == 1) {
                graph.remove_vertex(first);
                vertices.erase(first);
                for (auto it = edges.begin(); it != edges.end();) {
                    if (it->first == first || it->second == first)
                        it = edges.erase(it);
                    else
                        ++it;
                }
            } else {
                selected = vertices.begin();
                std::advance(selected, generator() % vertices.size());
                const auto second = *selected;
                const Graph<int>::edge_type edge = std::minmax(first, second);
                if (operation == 2 && first != second && edges.insert(edge).second)
                    graph.add_edge(first, second);
                else if (operation == 3 && edges.erase(edge))
                    graph.remove_edge(first, second);
            }
        }
        CHECK_EQUAL(vertices.size(), graph.vertex_count());
        CHECK_EQUAL(edges.size(), graph.edge_count());
        auto view = graph.edges();
        CHECK(std::set<Graph<int>::edge_type>(view.begin(), view.end()) == edges);
        for (auto id : vertices) {
            std::size_t degree = 0;
            for (const auto& edge : edges)
                degree += edge.first == id || edge.second == id;
            CHECK_EQUAL(degree, graph.vertex_degree(id));
        }
    }
}


TEST(ConsoleGraphMenuDemonstratesAllOperations)
{
    std::istringstream input(
        "2\nA\n2\nB\n2\nC\n3\n0\n1\n3\n0\n2\n6\n7\n0\n8\n0\n1\n9\n0\n"
        "12\n13\n15\n1\nChanged\n6\n13\n11\n1\n0\n3\n0\n1\n5\n0\n1\n"
        "3\n0\n1\n10\n1\n4\n2\n14\n6\n0\n");
    std::ostringstream output;
    CHECK_EQUAL(0, run_console(input, output));
    const auto text = output.str();
    CHECK(text.find("Ошибка:") == std::string::npos);
    CHECK(text.find("Вершин: 3, рёбер: 2, пустой: false") != std::string::npos);
    CHECK(text.find("Степень вершины: 2") != std::string::npos);
    CHECK(text.find("Степень ребра: 1") != std::string::npos);
    CHECK(text.find("Соседи, однонаправленный обход: B C") != std::string::npos);
    CHECK(text.find("Независимая копия сохранена") != std::string::npos);
    CHECK(text.find("Сравнение с сохранённой копией: == true") != std::string::npos);
    CHECK(text.find("Сравнение с сохранённой копией: == false") != std::string::npos);
    CHECK(text.find("1: Changed") != std::string::npos);
    CHECK(text.find("Вершин: 0, рёбер: 0, пустой: true") != std::string::npos);
}

TEST(ConsoleSortingMenuSupportsBothAlgorithmsAndAllDataKinds)
{
    for (const auto& algorithm : {std::string("1"), std::string("2")}) {
        std::istringstream input(
            "1\n" + algorithm + "\n1\n5\n4\n3\n2\n1\n"
            "1\n" + algorithm + "\n2\n3\n3\n1\n2\n"
            "1\n" + algorithm + "\n3\n0\n");
        std::ostringstream output;
        CHECK_EQUAL(0, run_console(input, output));
        const auto text = output.str();
        CHECK(text.find("Ошибка:") == std::string::npos);
        CHECK(text.find("1 2 3 4 5 ") != std::string::npos);
        CHECK(text.find("1 2 3 ") != std::string::npos);
        CHECK(text.find("Иван (6)") != std::string::npos);
    }
    std::istringstream input("1\n1\n2\n0\n0\n");
    std::ostringstream output;
    CHECK_EQUAL(0, run_console(input, output));
    CHECK(output.str().find("Ошибка:") == std::string::npos);
}

TEST(ConsoleRejectsInvalidInputAndRecovers)
{
    std::istringstream input(
        "bad\n1x\n\n-1\n16\n9999999999999999999999999\n"
        "2\nA\n3\n0\n0\n3\n0\n99\n7\n99\n8\n0\n99\n10\n99\n11\n0\n99\n"
        "1\n1\n2\n-1\n1\n2\n2\n9\n6\n0\n");
    std::ostringstream output;
    CHECK_EQUAL(0, run_console(input, output));
    const auto text = output.str();
    CHECK(text.find("Ошибка: Введите целое число") != std::string::npos);
    CHECK(text.find("Петли и повторные рёбра запрещены") != std::string::npos);
    CHECK(text.find("Вершина не найдена") != std::string::npos);
    CHECK(text.find("Ребро не найдено") != std::string::npos);
    CHECK(text.find("Вершин: 1, рёбер: 0, пустой: false") != std::string::npos);
}

TEST(ConsoleHandlesEndOfInputAndReadFailure)
{
    for (const auto& script : {"", "2\n", "2\nA\n15\n0\n", "1\n1\n1\n4\n", "   0   \n"}) {
        std::istringstream input(script);
        std::ostringstream output;
        CHECK_EQUAL(0, run_console(input, output));
    }
    std::istringstream broken;
    broken.setstate(std::ios::badbit);
    std::ostringstream output;
    CHECK_EQUAL(1, run_console(broken, output));
    CHECK(output.str().find("Ошибка чтения ввода") != std::string::npos);
}

int main()
{
    return UnitTest::RunAllTests();
}
