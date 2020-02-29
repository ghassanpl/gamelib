#pragma once

#include <cstdint>
#define FMT_USE_WINDOWS_H 0
#define FMT_HEADER_ONLY 1
#define FMT_SAFE_DURATION_CAST 0
#if _MSC_VER
extern "C" {
  unsigned char _BitScanReverse(unsigned long* _Index, unsigned long _Mask);
  unsigned char _BitScanReverse64(unsigned long* _Index, unsigned __int64 _Mask);
  unsigned char _BitScanReverse64(unsigned long* _Index, unsigned __int64 _Mask);
}
namespace fmt {
  inline namespace v6 {
    namespace internal {
#  ifndef __clang__
#    pragma intrinsic(_BitScanReverse)
#  endif
      inline uint32_t clz(uint32_t x) {
        unsigned long r = 0;
        _BitScanReverse(&r, x);
#  pragma warning(suppress : 6102)
        return 31 - r;
      }
#  define FMT_BUILTIN_CLZ(n) fmt::internal::clz(n)
#  if defined(_WIN64) && !defined(__clang__)
#    pragma intrinsic(_BitScanReverse64)
#  endif
      inline uint32_t clzll(uint64_t x) {
        unsigned long r = 0;
#  ifdef _WIN64
        _BitScanReverse64(&r, x);
#  else
        if (_BitScanReverse(&r, static_cast<uint32_t>(x >> 32))) return 63 - (r + 32);
        _BitScanReverse(&r, static_cast<uint32_t>(x));
#  endif
#  pragma warning(suppress : 6102)
        return 63 - r;
      }
#  define FMT_BUILTIN_CLZLL(n) fmt::internal::clzll(n)
    }
  }
}
#endif
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>

#define GAMELIB_FORMAT
#include "../Combos/Format+GLM.h"