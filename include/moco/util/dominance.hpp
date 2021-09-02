#pragma once

#include <moco/util/concepts.hpp>

#include <algorithm>
#include <cassert>
#include <iterator>

namespace moco {

// TODO add dominance relations with custom dominance order (currently assumes maximizing)
// TODO add execution policy
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
template <moco::is_or_has_objective_vector V1, moco::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto equivalent(V1 const& v1, V2 const& v2) noexcept -> bool {
  if constexpr (has_objective_vector<V1> && has_objective_vector<V2>) {
    return equivalent(v1.objective_vector(), v2.objective_vector());
  } else if constexpr (has_objective_vector<V1>) {
    return equivalent(v1.objective_vector(), v2);
  } else if constexpr (has_objective_vector<V1>) {
    return equivalent(v1, v2.objective_vector());
  } else {
    return equivalent(std::ranges::begin(v1), std::ranges::end(v1), std::ranges::begin(v2));
  }
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
template <moco::is_or_has_objective_vector V1, moco::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto weakly_dominates(V1 const& v1, V2 const& v2) noexcept -> bool {
  if constexpr (has_objective_vector<V1> && has_objective_vector<V2>) {
    return weakly_dominates(v1.objective_vector(), v2.objective_vector());
  } else if constexpr (has_objective_vector<V1>) {
    return weakly_dominates(v1.objective_vector(), v2);
  } else if constexpr (has_objective_vector<V1>) {
    return weakly_dominates(v1, v2.objective_vector());
  } else {
    return weakly_dominates(std::ranges::begin(v1), std::ranges::end(v1), std::ranges::begin(v2));
  }
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
template <moco::is_or_has_objective_vector V1, moco::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto dominates(V1 const& v1, V2 const& v2) noexcept -> bool {
  if constexpr (has_objective_vector<V1> && has_objective_vector<V2>) {
    return dominates(v1.objective_vector(), v2.objective_vector());
  } else if constexpr (has_objective_vector<V1>) {
    return dominates(v1.objective_vector(), v2);
  } else if constexpr (has_objective_vector<V1>) {
    return dominates(v1, v2.objective_vector());
  } else {
    return dominates(std::ranges::begin(v1), std::ranges::end(v1), std::ranges::begin(v2));
  }
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
template <moco::is_or_has_objective_vector V1, moco::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto strictly_dominates(V1 const& v1, V2 const& v2) noexcept -> bool {
  if constexpr (has_objective_vector<V1> && has_objective_vector<V2>) {
    return strictly_dominates(v1.objective_vector(), v2.objective_vector());
  } else if constexpr (has_objective_vector<V1>) {
    return strictly_dominates(v1.objective_vector(), v2);
  } else if constexpr (has_objective_vector<V1>) {
    return strictly_dominates(v1, v2.objective_vector());
  } else {
    return strictly_dominates(std::ranges::begin(v1), std::ranges::end(v1), std::ranges::begin(v2));
  }
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
template <moco::is_or_has_objective_vector V1, moco::is_or_has_objective_vector V2>
[[nodiscard]] constexpr auto incomparable(V1 const& v1, V2 const& v2) noexcept -> bool {
  if constexpr (has_objective_vector<V1> && has_objective_vector<V2>) {
    return incomparable(v1.objective_vector(), v2.objective_vector());
  } else if constexpr (has_objective_vector<V1>) {
    return incomparable(v1.objective_vector(), v2);
  } else if constexpr (has_objective_vector<V1>) {
    return incomparable(v1, v2.objective_vector());
  } else {
    return incomparable(std::ranges::begin(v1), std::ranges::end(v1),  // noformat
                        std::ranges::begin(v2), std::ranges::end(v2));
  }
}

/* Dominance relations between an objective vector and a set */

template <moco::is_or_has_objective_vector V, moco::solution_set S>
requires moco::is_or_has_objective_vector<moco::solution_t<S>>
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

template <moco::is_or_has_objective_vector V, moco::solution_set S>
requires moco::is_or_has_objective_vector<moco::solution_t<S>>
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
template <moco::is_or_has_objective_vector V, moco::solution_set S>
requires moco::is_or_has_objective_vector<moco::solution_t<S>>
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

template <moco::solution_set S, moco::is_or_has_objective_vector V>
requires moco::is_or_has_objective_vector<moco::solution_t<S>>
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

template <moco::solution_set S, moco::is_or_has_objective_vector V>
requires moco::is_or_has_objective_vector<moco::solution_t<S>>
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

template <moco::solution_set S, moco::is_or_has_objective_vector V>
requires moco::is_or_has_objective_vector<moco::solution_t<S>>
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
  {weakly_dominates(t, u)};
  {dominates(t, u)};
  {strictly_dominates(t, u)};
  {weakly_dominated(t, u)};
  {dominated(t, u)};
  {strictly_dominated(t, u)};
  {incomparable(t, u)};
};

}  // namespace moco
