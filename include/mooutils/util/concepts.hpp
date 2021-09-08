#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>

namespace mooutils {

// clang-format off

template<typename T>
concept decision_vector =
  std::ranges::sized_range<T> &&
  std::ranges::random_access_range<T> &&
  std::ranges::common_range<T>;

template <typename T>
concept objective_vector =
  std::ranges::sized_range<T> &&
  std::ranges::random_access_range<T> &&
  std::ranges::common_range<T> &&
  std::is_arithmetic_v<std::ranges::range_value_t<T>>;

template <typename T>
concept constraint_vector =
  std::ranges::sized_range<T> &&
  std::ranges::random_access_range<T> &&
  std::ranges::common_range<T> &&
  std::is_arithmetic_v<std::ranges::range_value_t<T>>;

template<typename T>
concept has_decision_vector =
  requires(T const& t) {
    { t.decision_vector() } -> decision_vector;
  };

template<typename T>
concept has_objective_vector =
  requires(T const& t) {
    { t.objective_vector() } -> objective_vector;
  };

template<typename T>
concept has_constraint_vector =
  requires(T const& t) {
    { t.constraint_vector() } -> constraint_vector;
  };

template<typename T>
concept is_or_has_decision_vector =
  decision_vector<T> ||
  has_decision_vector<T>;

template<typename T>
concept is_or_has_objective_vector =
  objective_vector<T> ||
  has_objective_vector<T>;

template<typename T>
concept is_or_has_constraint_vector =
  constraint_vector<T> ||
  has_constraint_vector<T>;

template<typename T>
concept solution_set =
  std::ranges::sized_range<T> &&
  std::ranges::common_range<T> &&
  std::ranges::forward_range<T>;

template<solution_set T>
using solution_t = std::ranges::range_value_t<T>;

template<typename T>
concept mutable_solution_set =
  solution_set<T> &&
  requires(T &t, solution_t<T> &&s, std::ranges::iterator_t<T> it) {
    { t.insert(std::move(s)) } -> std::same_as<bool>;
    { t.insert_unchecked(std::move(s)) };
    { t.erase(it) } -> std::same_as<bool>;
    { t.erase(it, it) } -> std::same_as<bool>;
  };

template<typename T>
concept solution_queue =
  requires(T &&t, typename T::solution_type &&s) {
    { t.push(std::move(s)) };
    { t.pop() } -> std::same_as<typename T::solution_type>;
  };


}
