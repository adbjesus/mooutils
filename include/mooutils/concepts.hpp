#ifndef MOOUTILS_CONCEPTS_HPP_
#define MOOUTILS_CONCEPTS_HPP_

#include <concepts>
#include <ranges>
#include <type_traits>

namespace mooutils {

// clang-format off

/**
 * \brief Decision vector concept.
 */
template<typename T>
concept is_decision_vector =
  std::ranges::sized_range<T> &&
  std::ranges::common_range<T> &&
  std::ranges::random_access_range<T> &&
  std::equality_comparable<std::ranges::range_value_t<T>> &&
  !std::ranges::range<std::ranges::range_value_t<T>>;

template<typename T>
concept has_decision_vector =
  requires(T&& t) {
    { t.decision_vector() } -> is_decision_vector;
  };

template<typename T>
concept is_or_has_decision_vector =
  is_decision_vector<T> ||
  has_decision_vector<T>;

template <typename T>
concept is_objective_vector =
  std::ranges::sized_range<T> &&
  std::ranges::common_range<T> &&
  std::ranges::random_access_range<T> &&
  std::totally_ordered<std::ranges::range_value_t<T>> &&
  !std::ranges::range<std::ranges::range_value_t<T>>;

template<typename T>
concept has_objective_vector =
  requires(T&& t) {
    { t.objective_vector() } -> is_objective_vector;
  };

template<typename T>
concept is_or_has_objective_vector =
  is_objective_vector<T> ||
  has_objective_vector<T>;

template <typename T>
concept is_constraint_vector =
  std::ranges::sized_range<T> &&
  std::ranges::common_range<T> &&
  std::ranges::random_access_range<T> &&
  std::totally_ordered<std::ranges::range_value_t<T>> &&
  !std::ranges::range<std::ranges::range_value_t<T>>;

template<typename T>
concept has_constraint_vector =
  requires(T&& t) {
    { t.constraint_vector() } -> is_constraint_vector;
  };

template<typename T>
concept is_or_has_constraint_vector =
  is_constraint_vector<T> ||
  has_constraint_vector<T>;

template<typename T>
concept is_decision_vector_set =
  std::ranges::sized_range<T> &&
  std::ranges::input_range<T> &&
  std::ranges::common_range<T> &&
  is_or_has_decision_vector<std::ranges::range_value_t<T>>;

template<typename T>
concept is_objective_vector_set =
  std::ranges::sized_range<T> &&
  std::ranges::input_range<T> &&
  std::ranges::common_range<T> &&
  is_or_has_objective_vector<std::ranges::range_value_t<T>>;

template<typename T>
concept is_constraint_vector_set =
  std::ranges::sized_range<T> &&
  std::ranges::input_range<T> &&
  std::ranges::common_range<T> &&
  is_or_has_constraint_vector<std::ranges::range_value_t<T>>;

}  // namespace mooutils

#endif
