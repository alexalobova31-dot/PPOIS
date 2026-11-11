#ifndef COMMON_H
#define COMMON_H
#include "Exceptions.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
namespace learning
{
using Money = std::int64_t;
void require(bool condition, const std::string &message);
void nonempty(const std::string &text);
void money(Money value);
template <class T> void present(const std::shared_ptr<T> &value)
{
    require(bool(value), "Null association");
}
}
#endif
