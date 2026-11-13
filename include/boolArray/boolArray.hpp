#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
enum class Mode {
  compact, // packed into uint8_t bytes
  fast     // use integer words
};

template <std::size_t N>
using FastStorage = std::conditional_t<
    (N <= 8), std::uint8_t,
    std::conditional_t<
        (N <= 16), std::uint16_t,
        std::conditional_t<
            (N <= 32), std::uint32_t,
            std::conditional_t<
                (N <= 64), std::uint64_t,
                std::array<std::uint64_t, N / 64 + (N % 64 != 0)>>>>>;

template <std::size_t N, Mode mode = Mode::compact>
using BoolStorage =
    std::conditional_t<mode == Mode::compact,
                       std::array<std::uint8_t, N / 8 + (N % 8 != 0)>,
                       FastStorage<N>>;

template <std::size_t N, Mode mode = Mode::compact> struct BoolArray {
  using storage_type = BoolStorage<N, mode>;
  using type = std::conditional_t<
      (N <= UINT8_MAX), std::uint8_t,
      std::conditional_t<
          (N <= UINT16_MAX), std::uint16_t,
          std::conditional_t<
              (N <= UINT32_MAX), std::uint32_t,
              std::conditional_t<(N <= UINT64_MAX), std::uint64_t, void>>>>;

  static_assert(!std::is_void_v<type>,
                "Number overflown, can't fit in a 64 bit");

  storage_type storage{};
  template <typename... U> constexpr BoolArray(U... values) {
    static_assert(sizeof...(U) <= N, "Too many initial values");
    static_assert((std::is_integral_v<U> && ...),
                  "Values must be integral or bool");

    constexpr std::size_t count = sizeof...(U);
    const std::array<bool, count> initial{static_cast<bool>(values)...};

    for (std::size_t i = 0; i < count; ++i) {

      if constexpr (mode == Mode::compact) {
        auto &byte = storage[i / 8];
        const auto mask = static_cast<std::uint8_t>(1u << (i % 8));

        if (initial[i])
          byte |= mask;
        else
          byte &= static_cast<std::uint8_t>(~mask);
      } else if constexpr (N <= 64) {
        const auto mask = static_cast<storage_type>(storage_type{1} << i);

        if (initial[i])
          storage = static_cast<storage_type>(storage | mask);
        else
          storage = static_cast<storage_type>(storage & ~mask);
      } else {
        auto &word = storage[i / 64];
        const auto mask = std::uint64_t{1} << (i % 64);

        if (initial[i])
          word |= mask;
        else
          word &= ~mask;
      }
    }
  }
  void setValue(std::size_t index, bool value = true) {
    if (index >= N)
      return;

    if constexpr (mode == Mode::compact) {
      auto &byte = storage[index / 8];
      const auto mask = static_cast<std::uint8_t>(1u << (index % 8));

      if (value)
        byte |= mask;
      else
        byte &= static_cast<std::uint8_t>(~mask);
    } else if constexpr (N <= 64) {
      const auto mask = static_cast<storage_type>(storage_type{1} << index);

      if (value)
        storage = static_cast<storage_type>(storage | mask);
      else
        storage = static_cast<storage_type>(storage & ~mask);
    } else {
      auto &word = storage[index / 64];
      const auto mask = std::uint64_t{1} << (index % 64);

      if (value)
        word |= mask;
      else
        word &= ~mask;
    }
  }
  template <unsigned index> void setValue(bool value = true) {
    if (index >= N)
      return;

    if constexpr (mode == Mode::compact) {
      auto &byte = storage[index / 8];
      constexpr auto mask = static_cast<std::uint8_t>(1u << (index % 8));

      if (value)
        byte |= mask;
      else
        byte &= static_cast<std::uint8_t>(~mask);
    } else if constexpr (N <= 64) {
      constexpr auto mask = static_cast<storage_type>(storage_type{1} << index);

      if (value)
        storage = static_cast<storage_type>(storage | mask);
      else
        storage = static_cast<storage_type>(storage & ~mask);
    } else {
      auto &word = storage[index / 64];
      constexpr auto mask = std::uint64_t{1} << (index % 64);

      if (value)
        word |= mask;
      else
        word &= ~mask;
    }
  }
  type getByte(std::size_t i) { return storage[i]; }
  void setByte(std::size_t i, type byte) { storage[i] = byte; }

  bool at(std::size_t index) const {
    if (index >= N)
      return false;

    if constexpr (mode == Mode::compact) {
      return (storage[index / 8] & (1u << (index % 8))) != 0;
    } else if constexpr (N <= 64) {
      return (storage & (storage_type{1} << index)) != 0;
    } else {
      return (storage[index / 64] & (std::uint64_t{1} << (index % 64))) != 0;
    }
  }
  bool operator[](std::size_t i) noexcept {
    if constexpr (mode == Mode::compact) {
      return (storage[i / 8] & (1u << (i % 8))) != 0;
    } else if constexpr (N <= 64) {
      return (storage & (storage_type{1} << i)) != 0;
    } else {
      return (storage[i / 64] & (std::uint64_t{1} << (i % 64))) != 0;
    }
  }
  constexpr void operator()(std::size_t i, bool val) noexcept {
    if constexpr (mode == Mode::compact) {
        if (val) storage[i / 8] |= (1u << (i % 8));
        else     storage[i / 8] &= ~(1u << (i % 8));
    } else if constexpr (N <= 64) {
        if (val) storage |= (storage_type{1} << i);
        else     storage &= ~(storage_type{1} << i);
    } else {
        if (val) storage[i / 64] |= (std::uint64_t{1} << (i % 64));
        else     storage[i / 64] &= ~(std::uint64_t{1} << (i % 64));
    }
}

  constexpr std::size_t getSize() { return N; }
  constexpr type getAmountOfVars() {
    if constexpr (mode == Mode::compact) {
      return N / 8 + (N % 8 != 0);
    } else if constexpr (N <= 64) {
      return 1;
    } else {
      return N / 64 + (N % 64 != 0);
    }
  }
  constexpr std::size_t getSpaceUsage() { return sizeof(storage); }
};
