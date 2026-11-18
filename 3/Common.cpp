#include "Common.h"
namespace learning
{
void require(bool condition, const std::string &message)
{
    if (!condition)
        throw ValidationError(message);
}
void nonempty(const std::string &text)
{
    require(text.find_first_not_of(" \t\r\n") != std::string::npos, "Empty text");
}
void money(Money value)
{
    require(value >= 0 && value <= 1000000000, "Invalid money amount");
}
}
