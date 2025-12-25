#pragma once

#include <concepts>
#include <utility>

namespace BitUtils
{

template <unsigned int N, typename T>
  requires std::unsigned_integral<T> || std::convertible_to<T, unsigned int>
constexpr T Set(T &&data)
{
  return (data | (1U << N));
}

template <int N, typename T>
  requires std::unsigned_integral<T> || std::convertible_to<T, unsigned int>
constexpr void Set(T &data)
{
  data = Set<N>(std::move(data));
}

static_assert(Set<0>(0U) == 1U, "Set Not Working");
static_assert(Set<1>(0U) == 2U, "Set Not Working");
static_assert(Set<2>(0U) == 4U, "Set Not Working");

template <unsigned int N, typename T>
  requires std::unsigned_integral<T> || std::convertible_to<T, unsigned int>
constexpr T Unset(T &&data)
{
  return (data & ~(1U << N));
}

template <unsigned int N, typename T>
  requires std::unsigned_integral<T> || std::convertible_to<T, unsigned int>
constexpr void Unset(T &data)
{
  data = Unset<N>(std::move(data));
}

static_assert(Unset<0>(1U) == 0U, "Unset not working");
static_assert(Unset<1>(6U) == 4U, "Unset not working");
static_assert(Unset<3>(8U) == 0U, "Unset not working");

template <unsigned int N, typename T>
  requires std::unsigned_integral<T> || std::convertible_to<T, unsigned int>
constexpr bool Test(T &&data)
{
  return ((data >> N) & 1U) == 1U;
}

static_assert(Test<0>(1U), "Test not working");
static_assert(Test<3>(8U), "Test not working");
static_assert(!Test<2>(8U), "Test not working");

}  // namespace BitUtils
