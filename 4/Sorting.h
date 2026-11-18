#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <random>
#include <stdexcept>

namespace lab {

template <class Iterator, class Compare = std::less<>>
void in_place_merge_sort(Iterator first, Iterator last, Compare compare = {})
{
    const auto size = std::distance(first, last);
    if (size < 2)
        return;
    auto middle = std::next(first, size / 2);
    in_place_merge_sort(first, middle, compare);
    in_place_merge_sort(middle, last, compare);
    auto left = first;
    auto right = middle;
    while (left != right && right != last) {
        if (compare(*right, *left)) {
            auto next = std::next(right);
            std::rotate(left, right, next);
            ++left;
            ++right;
        } else {
            ++left;
        }
    }
}

template <class Iterator, class Generator, class Compare = std::less<>>
void bogo_sort(Iterator first, Iterator last, Generator& generator,
               std::size_t max_attempts = 100000, Compare compare = {})
{
    std::size_t attempts = 0;
    while (!std::is_sorted(first, last, compare)) {
        if (attempts == max_attempts)
            throw std::runtime_error("Bogosort: превышен лимит перемешиваний");
        std::shuffle(first, last, generator);
        ++attempts;
    }
}

}
