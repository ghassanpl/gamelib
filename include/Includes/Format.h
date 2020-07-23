#pragma once

#include <cstdint>
#define FMT_USE_WINDOWS_H 0
#define FMT_HEADER_ONLY 1
#define FMT_SAFE_DURATION_CAST 0
#if defined(__clang__) || defined(__GNUC__)
#define FMT_BUILTIN_CLZLL __builtin_clzll
#elif defined(_MSC_VER)
extern "C" unsigned char _BitScanReverse64(unsigned long* _Index, unsigned __int64 _Mask);
#pragma intrinsic(_BitScanReverse64)
inline int clz(unsigned long long v) { unsigned long r; _BitScanReverse64(&r, v); return 63-r; }
#define FMT_BUILTIN_CLZLL clz
#else
inline int clz(unsigned long long v)
{
  static constexpr unsigned char MultiplyDeBruijnBitPosition[] = {
     0, 47,  1, 56, 48, 27,  2, 60, 57, 49, 41, 37, 28, 16,  3, 61,
    54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11,  4, 62,
    46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
    25, 39, 14, 33, 19, 30,  9, 24, 13, 18,  8, 12,  7,  6,  5, 63
  };
  v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v |= v >> 32;
  return 63 - MultiplyDeBruijnBitPosition[(v * 0x03F79D71B4CB0A89ULL) >> 58];
}
#define FMT_BUILTIN_CLZLL clz
#endif
#ifdef _WIN32
#undef _WIN32
#define Q_WIN32
#endif
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>
#ifdef Q_WIN32
#undef Q_WIN32
#define _WIN32
#endif

#define GAMELIB_FORMAT
#ifdef GAMELIB_GLM
#include "../Combos/Format+GLM.h"
#endif