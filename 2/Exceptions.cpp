#include "Exceptions.h"

PharmaException::PharmaException(const std::string& message) : std::runtime_error(message)
{
}
