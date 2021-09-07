#pragma once

#include <moutils/util/concepts.hpp>
#include <moutils/util/solution.hpp>

#include <algorithm>
#include <cassert>
#include <iterator>

namespace moutils {

// TODO add dominance relations with custom dominance order (currently assumes maximizing)
// TODO add execution policy
// TODO add dominated variants?
// TODO use macros to automatically generate the range based functions

template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2>
[[nodiscard]] constexpr auto equivalent(I1 first1, S1 last1, I2 first2) -> bool {
  for (; first1 != last1; ++first1, ++first2) {
    if (*first1 != *first2) {
      return false;
    }
  }
  return true;
}

// Check if vector `v1` and `v2` are equivalent.
//
// Undefined behavior if `v1` and `v2` have different sizes.
template <moutils::is_or_has_objective_vector V1, moutils::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto equivalent(V1 const& v1, V2 const& v2) noexcept -> bool {
  auto const& ov1 = get_objective_vector(v1);
  auto const& ov2 = get_objective_vector(v2);
  return equivalent(std::ranges::begin(ov1), std::ranges::end(ov1), std::ranges::begin(ov2));
}

template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2>
[[nodiscard]] constexpr auto weakly_dominates(I1 first1, S1 last1, I2 first2) -> bool {
  for (; first1 != last1; ++first1, ++first2) {
    if (*first1 < *first2) {
      return false;
    }
  }
  return true;
}

// Check if a vector `v1` weakly dominates a vector `v2`.
//
// Undefined behavior if `v1` and `v2` have different sizes.
template <moutils::is_or_has_objective_vector V1, moutils::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto weakly_dominates(V1 const& v1, V2 const& v2) noexcept -> bool {
  auto const& ov1 = get_objective_vector(v1);
  auto const& ov2 = get_objective_vector(v2);
  return weakly_dominates(std::ranges::begin(ov1), std::ranges::end(ov1), std::ranges::begin(ov2));
}

template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2>
[[nodiscard]] constexpr auto dominates(I1 first1, S1 last1, I2 first2) -> bool {
  for (; first1 != last1; ++first1, ++first2) {
    if (*first1 > *first2) {
      return weakly_dominates(++first1, last1, ++first2);
    } else if (*first1 < *first2) {
      return false;
    }
  }
  return false;
}

// Check if a vector `v1` dominates a vector `v2`.
//
// Undefined behavior if `v1` and `v2` have different sizes.
template <moutils::is_or_has_objective_vector V1, moutils::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto dominates(V1 const& v1, V2 const& v2) noexcept -> bool {
  auto const& ov1 = get_objective_vector(v1);
  auto const& ov2 = get_objective_vector(v2);
  return dominates(std::ranges::begin(ov1), std::ranges::end(ov1), std::ranges::begin(ov2));
}

template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2>
[[nodiscard]] constexpr auto strictly_dominates(I1 first1, S1 last1, I2 first2) -> bool {
  for (; first1 != last1; ++first1, ++first2) {
    if (*first1 <= *first2) {
      return false;
    }
  }
  return true;
}

// Check if a vector `v1` strictly dominates a vector `v2`.
//
// Undefined behavior if `v1` and `v2` have different sizes.
template <moutils::is_or_has_objective_vector V1, moutils::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto strictly_dominates(V1 const& v1, V2 const& v2) noexcept -> bool {
  auto const& ov1 = get_objective_vector(v1);
  auto const& ov2 = get_objective_vector(v2);
  return strictly_dominates(std::ranges::begin(ov1), std::ranges::end(ov1),
                            std::ranges::begin(ov2));
}

template <std::input_iterator I1, std::sentinel_for<I1> S1,  // noformat
          std::input_iterator I2, std::sentinel_for<I2> S2>
[[nodiscard]] constexpr auto incomparable(I1 first1, S1 last1, I2 first2, S2 last2) -> bool {
  for (; first1 != last1; ++first1, ++first2) {
    if (*first1 > *first2) {
      return dominates(++first2, last2, ++first1);
    } else if (*first2 > *first1) {
      return dominates(++first1, last1, ++first2);
    }
  }
  return false;
}

// Check if vector `v1` and `v2` are incomparable.
//
// Undefined behavior if `v1` and `v2` have different sizes.
template <moutils::is_or_has_objective_vector V1, moutils::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto incomparable(V1 const& v1, V2 const& v2) noexcept -> bool {
  auto const& ov1 = get_objective_vector(v1);
  auto const& ov2 = get_objective_vector(v2);
  return incomparable(std::ranges::begin(ov1), std::ranges::end(ov1), std::ranges::begin(ov2),
                      std::ranges::end(ov2));
}

/* Dominance relations between an objective vector and a set */

template <moutils::is_or_has_objective_vector V, moutils::solution_set S>
requires moutils::is_or_has_objective_vector<moutils::solution_t<S>>
[[nodiscard]] constexpr auto weakly_dominates(V const& v, S const& s) -> bool {
  if constexpr (requires {
                  { s.weakly_dominated(v) } -> std::same_as<bool>;
                }) {
    // if the set has an equivalent method call that method
    s.weakly_dominated(v);
  } else {
    // otherwise use a default implementation
    return std::ranges::all_of(s, [&v](auto const& s) { return weakly_dominates(v, s); });
  }
}

template <moutils::is_or_has_objective_vector V, moutils::solution_set S>
requires moutils::is_or_has_objective_vector<moutils::solution_t<S>>
[[nodiscard]] constexpr auto dominates(V const& v, S const& s) -> bool {
  if constexpr (requires {
                  { s.dominated(v) } -> std::same_as<bool>;
                }) {
    // if the set has an equivalent method call that method
    s.dominated(v);
  } else {
    // otherwise use a default implementation
    auto end = std::ranges::end(s);
    for (auto it = std::ranges::begin(s); it != end; ++it) {
      if (!weakly_dominates(v, *it)) {
        return false;
      }
      if (!weakly_dominates(*it, v)) {
        return std::all_of(++it, end, [&v](auto const& s) { return weakly_dominates(v, s); });
      }
    }
    return false;
  }
}

// For every solution
template <moutils::is_or_has_objective_vector V, moutils::solution_set S>
requires moutils::is_or_has_objective_vector<moutils::solution_t<S>>
[[nodiscard]] constexpr auto strictly_dominates(V const& v, S const& s) -> bool {
  if constexpr (requires {
                  { s.strictly_dominated(v) } -> std::same_as<bool>;
                }) {
    // if the set has an equivalent method call that method
    s.strictly_dominated(v);
  } else {
    // otherwise use a default implementation
    return std::ranges::all_of(s, [&v](auto const& s) { return dominates(v, s); });
  }
}

/* Dominance relations between a set and and an objective vector */

template <moutils::solution_set S, moutils::is_or_has_objective_vector V>
requires moutils::is_or_has_objective_vector<moutils::solution_t<S>>
[[nodiscard]] constexpr auto weakly_dominates(S const& s, V const& v) -> bool {
  if constexpr (requires {
                  { s.weakly_dominates(v) } -> std::same_as<bool>;
                }) {
    // if the set has such a method call that method
    s.weakly_dominates(v);
  } else {
    // otherwise use a default implementation
    return std::ranges::any_of(s, [&v](auto const& s) { return weakly_dominates(s, v); });
  }
}

template <moutils::solution_set S, moutils::is_or_has_objective_vector V>
requires moutils::is_or_has_objective_vector<moutils::solution_t<S>>
[[nodiscard]] constexpr auto dominates(S const& s, V const& v) -> bool {
  if constexpr (requires {
                  { s.dominates(v) } -> std::same_as<bool>;
                }) {
    // if the set has such a method call that method
    s.dominates(v);
  } else {
    // otherwise use a default implementation
    // TODO this can be improved
    return weakly_dominates(s, v) && !weakly_dominates(v, s);
  }
}

template <moutils::solution_set S, moutils::is_or_has_objective_vector V>
requires moutils::is_or_has_objective_vector<moutils::solution_t<S>>
[[nodiscard]] constexpr auto strictly_dominates(S const& s, V const& v) -> bool {
  if constexpr (requires {
                  { s.strictly_dominates(v) } -> std::same_as<bool>;
                }) {
    // if the set has such a method call that method
    s.strictly_dominates(v);
  } else {
    // otherwise use a default implementation
    return std::ranges::any_of(s, [&v](auto const& s) { return dominates(s, v); });
  }
}

/* TODO Dominance relations between sets */

template <typename T, typename U = T>
concept dominance_comparable = requires(T const& t, U const& u) {
  {equivalent(t, u)};
  {equivalent(u, t)};
  {weakly_dominates(t, u)};
  {weakly_dominates(u, t)};
  {dominates(t, u)};
  {dominates(u, t)};
  {strictly_dominates(t, u)};
  {strictly_dominates(u, t)};
  {incomparable(t, u)};
  {incomparable(u, t)};
};

}  // namespace moutils
