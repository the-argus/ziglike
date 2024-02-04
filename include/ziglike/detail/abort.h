#pragma once
// Defines the ABORT() macro which throws an exception in debug mode or just
// calls std::abort in release mode.

#ifdef ZIGLIKE_HEADER_TESTING
#include <exception>
namespace reserve {
class _abort_exception : std::exception
{
  public:
    char *what() { return "Program failure."; }
};
} // namespace reserve
#define ZIGLIKE_ABORT() throw ::reserve::_abort_exception()
#else
#include <cstdlib>
#define ZIGLIKE_ABORT() std::abort()
#endif
