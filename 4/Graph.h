#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template <class T>
class Graph {
public:
    using value_type = T;
    using vertex_id = std::size_t;
    using size_type = std::size_t;
    using edge_type = std::pair<vertex_id, vertex_id>;

private:
    struct Vertex {
        vertex_id id;
        T value;
        std::list<Vertex*> adjacent;
    };

    std::list<Vertex> vertices_;
    vertex_id next_id_ = 0;
    size_type revision_ = 0;

    const Vertex& vertex(vertex_id id) const
    {
        auto it = std::find_if(vertices_.begin(), vertices_.end(),
                               [id](const auto& item) { return item.id == id; });
        if (it == vertices_.end())
            throw std::out_of_range("Вершина не найдена");
        return *it;
    }

    Vertex& vertex(vertex_id id)
    {
        return const_cast<Vertex&>(std::as_const(*this).vertex(id));
    }

    auto comparison_key() const
    {
        std::vector<T> values;
        std::vector<vertex_id> ids;
        for (const auto& item : vertices_) {
            values.push_back(item.value);
            ids.push_back(item.id);
        }
        std::vector<edge_type> connections;
        for (const auto& item : vertices_)
            for (const auto* other : item.adjacent)
                if (item.id < other->id) {
                    const auto first = std::find(ids.begin(), ids.end(), item.id) - ids.begin();
                    const auto second = std::find(ids.begin(), ids.end(), other->id) - ids.begin();
                    connections.emplace_back(first, second);
                }
        std::sort(connections.begin(), connections.end());
        return std::make_pair(values, connections);
    }

public:
    template <bool Edge, bool Constant, bool Forward = false>
    class Iterator {
        using graph_type = std::conditional_t<Constant, const Graph, Graph>;
        using entry_type = std::conditional_t<Edge, edge_type, const Vertex*>;
        graph_type* owner_ = nullptr;
        std::shared_ptr<const std::vector<entry_type>> entries_;
        size_type position_ = 0;
        size_type revision_ = 0;
        int kind_ = 0;
        vertex_id anchor_ = 0;

        friend class Graph;
        template <bool, bool, bool> friend class Iterator;

        Iterator(graph_type* owner, std::shared_ptr<const std::vector<entry_type>> entries,
                 size_type position, int kind, vertex_id anchor)
            : owner_(owner), entries_(std::move(entries)), position_(position),
              revision_(owner->revision_), kind_(kind), anchor_(anchor) {}

        void validate(bool dereference = false) const
        {
            if (!owner_ || owner_->revision_ != revision_)
                throw std::logic_error("Итератор не привязан к графу или устарел");
            if (dereference && position_ >= entries_->size())
                throw std::out_of_range("Разыменование конечного итератора");
        }

    public:
        using value_type = std::conditional_t<Edge, edge_type, T>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::conditional_t<Forward, std::forward_iterator_tag,
                                                      std::bidirectional_iterator_tag>;
        using reference = std::conditional_t<Edge || Constant, const value_type&, value_type&>;
        using pointer = std::conditional_t<Edge || Constant, const value_type*, value_type*>;

        Iterator() = default;

        template <bool Other, std::enable_if_t<Constant && !Other, int> = 0>
        Iterator(const Iterator<Edge, Other, Forward>& other)
            : owner_(other.owner_), entries_(other.entries_), position_(other.position_),
              revision_(other.revision_), kind_(other.kind_), anchor_(other.anchor_) {}

        reference operator*() const
        {
            validate(true);
            if constexpr (Edge)
                return (*entries_)[position_];
            else if constexpr (Constant)
                return (*entries_)[position_]->value;
            else
                return const_cast<T&>((*entries_)[position_]->value);
        }

        pointer operator->() const { return std::addressof(operator*()); }

        Iterator& operator++()
        {
            validate(true);
            ++position_;
            return *this;
        }

        Iterator operator++(int) { auto old = *this; ++*this; return old; }

        template <bool Enabled = Forward, std::enable_if_t<!Enabled && !Forward, int> = 0>
        Iterator& operator--()
        {
            validate();
            if (position_ == 0)
                throw std::out_of_range("Декремент начального итератора");
            --position_;
            return *this;
        }

        template <bool Enabled = Forward, std::enable_if_t<!Enabled && !Forward, int> = 0>
        Iterator operator--(int) { auto old = *this; --*this; return old; }

        template <bool Other>
        bool operator==(const Iterator<Edge, Other, Forward>& other) const
        {
            return owner_ == other.owner_ && revision_ == other.revision_
                && kind_ == other.kind_ && anchor_ == other.anchor_ && position_ == other.position_;
        }

        template <bool Other>
        bool operator!=(const Iterator<Edge, Other, Forward>& other) const { return !(*this == other); }

        vertex_id id() const
        {
            static_assert(!Edge, "Идентификатор доступен только у итератора вершин");
            validate(true);
            return (*entries_)[position_]->id;
        }
    };

    template <bool Edge, bool Constant, bool Forward = false>
    class Range {
        using graph_type = std::conditional_t<Constant, const Graph, Graph>;
        using entry_type = std::conditional_t<Edge, edge_type, const Vertex*>;
        graph_type* owner_;
        std::shared_ptr<const std::vector<entry_type>> entries_;
        int kind_;
        vertex_id anchor_;
        size_type revision_;
        friend class Graph;

        Range(graph_type* owner, std::vector<entry_type> entries, int kind, vertex_id anchor)
            : owner_(owner), entries_(std::make_shared<const std::vector<entry_type>>(std::move(entries))),
              kind_(kind), anchor_(anchor), revision_(owner->revision_) {}

        void validate() const
        {
            if (owner_->revision_ != revision_)
                throw std::logic_error("Диапазон устарел после изменения графа");
        }

    public:
        using iterator = Iterator<Edge, Constant, Forward>;
        using const_iterator = Iterator<Edge, true, Forward>;
        iterator begin() const { validate(); return {owner_, entries_, 0, kind_, anchor_}; }
        iterator end() const { validate(); return {owner_, entries_, entries_->size(), kind_, anchor_}; }
        const_iterator cbegin() const { return begin(); }
        const_iterator cend() const { return end(); }

        template <bool Enabled = Forward, std::enable_if_t<!Enabled && !Forward, int> = 0>
        auto rbegin() const { return std::make_reverse_iterator(end()); }
        template <bool Enabled = Forward, std::enable_if_t<!Enabled && !Forward, int> = 0>
        auto rend() const { return std::make_reverse_iterator(begin()); }
        template <bool Enabled = Forward, std::enable_if_t<!Enabled && !Forward, int> = 0>
        auto crbegin() const { return std::make_reverse_iterator(cend()); }
        template <bool Enabled = Forward, std::enable_if_t<!Enabled && !Forward, int> = 0>
        auto crend() const { return std::make_reverse_iterator(cbegin()); }
    };

    using iterator = Iterator<false, false>;
    using const_iterator = Iterator<false, true>;
    using edge_iterator = Iterator<true, false>;
    using const_edge_iterator = Iterator<true, true>;
    using adjacent_iterator = iterator;
    using const_adjacent_iterator = const_iterator;
    using incident_iterator = edge_iterator;
    using const_incident_iterator = const_edge_iterator;
    using forward_iterator = Iterator<false, false, true>;
    using const_forward_iterator = Iterator<false, true, true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    Graph() = default;

    Graph(const Graph& other) : next_id_(other.next_id_)
    {
        for (const auto& item : other.vertices_)
            vertices_.push_back({item.id, item.value, {}});
        for (const auto& item : other.vertices_)
            for (const auto* adjacent : item.adjacent)
                vertex(item.id).adjacent.push_back(&vertex(adjacent->id));
    }

    Graph& operator=(const Graph& other)
    {
        if (this != &other) {
            Graph copy(other);
            vertices_.swap(copy.vertices_);
            std::swap(next_id_, copy.next_id_);
            ++revision_;
        }
        return *this;
    }

    ~Graph() = default;

    bool empty() const noexcept { return vertices_.empty(); }
    size_type vertex_count() const noexcept { return vertices_.size(); }
    size_type edge_count() const noexcept
    {
        size_type count = 0;
        for (const auto& item : vertices_)
            count += item.adjacent.size();
        return count / 2;
    }

    bool has_vertex(vertex_id id) const
    {
        return std::any_of(vertices_.begin(), vertices_.end(),
                           [id](const auto& item) { return item.id == id; });
    }

    bool has_edge(vertex_id first, vertex_id second) const
    {
        if (!has_vertex(first) || !has_vertex(second))
            return false;
        const auto& adjacent = vertex(first).adjacent;
        return std::any_of(adjacent.begin(), adjacent.end(),
                           [second](const auto* item) { return item->id == second; });
    }

    T& value(vertex_id id) { return vertex(id).value; }
    const T& value(vertex_id id) const { return vertex(id).value; }
    size_type vertex_degree(vertex_id id) const { return vertex(id).adjacent.size(); }

    size_type edge_degree(vertex_id first, vertex_id second) const
    {
        if (!has_edge(first, second))
            throw std::out_of_range("Ребро не найдено");
        return vertex_degree(first) + vertex_degree(second) - 2;
    }

    vertex_id add_vertex(T value)
    {
        if (next_id_ == std::numeric_limits<vertex_id>::max())
            throw std::overflow_error("Идентификаторы вершин исчерпаны");
        vertices_.push_back({next_id_, std::move(value), {}});
        ++revision_;
        return next_id_++;
    }

    void add_edge(vertex_id first, vertex_id second)
    {
        auto& from = vertex(first);
        auto& to = vertex(second);
        if (first == second || has_edge(first, second))
            throw std::invalid_argument("Петли и повторные рёбра запрещены");
        from.adjacent.push_back(&to);
        try {
            to.adjacent.push_back(&from);
        } catch (...) {
            from.adjacent.pop_back();
            throw;
        }
        ++revision_;
    }

    void remove_edge(vertex_id first, vertex_id second)
    {
        if (!has_edge(first, second))
            throw std::out_of_range("Ребро не найдено");
        auto& from = vertex(first);
        auto& to = vertex(second);
        from.adjacent.remove(&to);
        to.adjacent.remove(&from);
        ++revision_;
    }

    void remove_vertex(vertex_id id)
    {
        auto& removed = vertex(id);
        for (auto* adjacent : removed.adjacent)
            adjacent->adjacent.remove(&removed);
        auto it = std::find_if(vertices_.begin(), vertices_.end(),
                              [id](const auto& item) { return item.id == id; });
        vertices_.erase(it);
        ++revision_;
    }

    template <bool Constant>
    void remove_vertex(const Iterator<false, Constant>& position)
    {
        position.validate(true);
        if (position.owner_ != this || position.kind_ != 0)
            throw std::invalid_argument("Нужен итератор вершин этого графа");
        remove_vertex(position.id());
    }

    template <bool Constant>
    void remove_edge(const Iterator<true, Constant>& position)
    {
        position.validate(true);
        if (position.owner_ != this)
            throw std::invalid_argument("Нужен итератор рёбер этого графа");
        const auto endpoints = *position;
        remove_edge(endpoints.first, endpoints.second);
    }

    void clear() noexcept
    {
        vertices_.clear();
        ++revision_;
    }

private:
    template <bool Constant, bool Forward = false, class Owner>
    static auto vertex_range(Owner* owner, int kind, vertex_id anchor = 0)
    {
        std::vector<const Vertex*> entries;
        if (kind == 0) {
            for (const auto& item : owner->vertices_)
                entries.push_back(&item);
        } else {
            for (const auto* item : owner->vertex(anchor).adjacent)
                entries.push_back(item);
        }
        return Range<false, Constant, Forward>(owner, std::move(entries), kind, anchor);
    }

    template <bool Constant, class Owner>
    static auto edge_range(Owner* owner, int kind, vertex_id anchor = 0)
    {
        std::vector<edge_type> entries;
        if (kind == 1) {
            for (const auto& item : owner->vertices_)
                for (const auto* other : item.adjacent)
                    if (item.id < other->id)
                        entries.emplace_back(item.id, other->id);
        } else {
            for (const auto* other : owner->vertex(anchor).adjacent)
                entries.emplace_back(std::min(anchor, other->id), std::max(anchor, other->id));
        }
        return Range<true, Constant>(owner, std::move(entries), kind, anchor);
    }

public:
    auto vertices() { return vertex_range<false>(this, 0); }
    auto vertices() const { return vertex_range<true>(this, 0); }
    auto edges() { return edge_range<false>(this, 1); }
    auto edges() const { return edge_range<true>(this, 1); }
    auto incident_edges(vertex_id id) { return edge_range<false>(this, 2, id); }
    auto incident_edges(vertex_id id) const { return edge_range<true>(this, 2, id); }
    auto adjacent_vertices(vertex_id id) { return vertex_range<false>(this, 3, id); }
    auto adjacent_vertices(vertex_id id) const { return vertex_range<true>(this, 3, id); }
    auto adjacent_forward(vertex_id id) { return vertex_range<false, true>(this, 4, id); }
    auto adjacent_forward(vertex_id id) const { return vertex_range<true, true>(this, 4, id); }

    iterator begin() { return vertices().begin(); }
    iterator end() { return vertices().end(); }
    const_iterator begin() const { return vertices().begin(); }
    const_iterator end() const { return vertices().end(); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
    reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
    reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return std::make_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return std::make_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const { return rbegin(); }
    const_reverse_iterator crend() const { return rend(); }

    bool operator==(const Graph& other) const { return comparison_key() == other.comparison_key(); }
    bool operator!=(const Graph& other) const { return !(*this == other); }
    bool operator<(const Graph& other) const { return comparison_key() < other.comparison_key(); }
    bool operator>(const Graph& other) const { return other < *this; }
    bool operator<=(const Graph& other) const { return !(other < *this); }
    bool operator>=(const Graph& other) const { return !(*this < other); }

    friend std::ostream& operator<<(std::ostream& out, const Graph& graph)
    {
        out << "Вершины: ";
        auto vertices = graph.vertices();
        std::for_each(vertices.cbegin(), vertices.cend(), [&](const T& value) { out << value << ' '; });
        out << "\nРёбра (ID): ";
        auto edges = graph.edges();
        std::for_each(edges.cbegin(), edges.cend(), [&](const edge_type& edge) {
            out << '(' << edge.first << ", " << edge.second << ") ";
        });
        return out;
    }
};
