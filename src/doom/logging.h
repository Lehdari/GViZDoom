#pragma once

#include <string>

namespace doom_logging
{
inline void print(const char* fmt, ...)
{
    (void)fmt;
    // Left intentionally blank
}

inline void print(int loglevel, const char* fmt, ...)
{
    (void)loglevel;
    (void)fmt;
    // Left intentionally blank
}

} // namespace