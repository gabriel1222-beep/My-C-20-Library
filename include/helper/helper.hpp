#pragma once
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>
namespace helper {
enum Error_type { Fatal_Error, Error, Warning, Count };
using LogFn = void (*)(std::string_view &, bool);
template <typename T, typename... Args>
constexpr bool matchesAny(const T &target, const Args &...items) {
  return ((target == items) || ...);
}
template <typename T, typename... Args>
constexpr bool matchesAll(const T &target, const Args &...items) {
  return ((target == items) && ...);
}
template <typename T> long double average(const std::vector<T> &numbers) {
  static_assert(std::is_arithmetic<T>::value, "Vector must be arithmetic");
  return (numbers.empty())
             ? 0
             : std::accumulate(numbers.begin(), numbers.end(), 0.0L) /
                   numbers.size();
}
void logError(Error_type error, const bool exit, std::string_view str);
} // namespace helper
