#pragma once

#include <mooutils/util/concepts.hpp>
#include <mooutils/util/dominance.hpp>
#include <mooutils/util/solution.hpp>

#include <concepts>
#include <iostream>
#include <iterator>
#include <list>
#include <ranges>
#include <set>
#include <vector>

namespace mooutils::solution_sets {

/// Attempts to insert a new solution into a set. If this solution is
/// equal to or dominated by a solution in the set, then this solution
/// is not inserted and the function returns `false`. Otherwise, the
/// solution is inserted (anywhere in the set to optimized speed) and
/// solutions in the set dominated by this solution are removed, and the
/// function returns `true`.
///
/// Assumptions: all solutions in the set are mutually incomparable
// template <typename Solution, typename SolutionSet>
// auto insert_solution(SolutionSet& set, Solution&& solution) -> bool {
//   auto last = set.end();
//   for (auto it = set.begin(); it != last; ++it) {
//     if (dominates(*it, solution)) {
//       return false;
//     }
//     if (weakly_dominates(solution, *it)) {
//       if (equivalent(solution, *it)) {
//         if (std::any_of(it, last, [solution](auto const& s) { return solution == s; })) {
//           return false;
//         } else {
//           break;
//         }
//       } else {
//         *it = std::forward<S>(solution);
//         auto remove_pred = [it](auto const& s) {
//           return dominates(*it, s);
//         };
//         erase(std::remove_if(std::next(it), last, remove_pred), last);
//         return true;
//       }
//     }
//   }
//   insert_unchecked(std::forward<S>(solution));
//   return true;
// }

/// Same as insert_solution but takes into account a dominance
/// preserving order. A dominance preserving order is one such that, for
/// a solution s_{i}, it holds that solutions s_{j}, j < i, weakly
/// dominate or are incomparable to s_{i}, and, solutions s_{k}, k > i,
/// are incomparable or dominated by solutions s_{i+1}. Moreover, a
/// solution may be inserted before the upper bound given by this order.
/// A reverse lexicographical order guarantees this.
// template <typename Solution, typename SolutionSet, typename DominanceOrder>
// auto insert_solution_inorder(SolutionSet& s, Solution&& s, DominanceOrder cmp) -> bool {}

template <typename Solution, typename Container = std::vector<Solution>>
requires mooutils::dominance_comparable<Solution> &&
    std::same_as<Solution, typename Container::value_type>
class multivector {
 public:
  using container_type = Container;
  using value_type = typename container_type::value_type;
  using size_type = typename container_type::size_type;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  template <typename S>
  requires mooutils::dominance_comparable<S, value_type> && std::convertible_to<S, value_type>
  constexpr auto insert(S&& solution) -> iterator {
    auto first = m_container.begin();
    auto last = m_container.end();
    for (auto it = first; it != last; ++it) {
      if (dominates(*it, solution)) {
        return last;
      }
      if (weakly_dominates(solution, *it)) {
        if (equivalent(solution, *it)) {
          if (std::any_of(it, last, [solution](auto const& s) { return solution == s; })) {
            return last;
          } else {
            return insert_unchecked(std::forward<S>(solution));
          }
        } else {
          *it = std::forward<S>(solution);
          auto remove_pred = [it](auto const& s) {
            return dominates(*it, s);
          };
          erase(std::remove_if(std::next(it), last, remove_pred), last);
          return it;
        }
      }
    }
    return insert_unchecked(std::forward<S>(solution));
  }

  template <typename S>
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    return m_container.emplace(m_container.end(), std::forward<S>(solution));
  }

  constexpr auto erase(iterator it) {
    return m_container.erase(it);
  }

  constexpr auto erase(const_iterator it) {
    return m_container.erase(it);
  }

  constexpr auto erase(iterator first, iterator last) {
    return m_container.erase(first, last);
  }

  constexpr auto erase(const_iterator first, const_iterator last) {
    return m_container.erase(first, last);
  }

  [[nodiscard]] constexpr auto begin() {
    return m_container.begin();
  }

  [[nodiscard]] constexpr auto end() {
    return m_container.end();
  }

  [[nodiscard]] constexpr auto begin() const {
    return m_container.begin();
  }

  [[nodiscard]] constexpr auto end() const {
    return m_container.end();
  }

  [[nodiscard]] constexpr auto cbegin() const {
    return m_container.cbegin();
  }

  [[nodiscard]] constexpr auto cend() const {
    return m_container.cend();
  }

  [[nodiscard]] constexpr auto size() const {
    return m_container.size();
  }

  [[nodiscard]] constexpr auto empty() const {
    return m_container.empty();
  }

 private:
  container_type m_container;
};

template <typename Lhs, typename Rhs = Lhs>
requires mooutils::is_or_has_objective_vector<Lhs> && mooutils::is_or_has_objective_vector<Rhs>
struct lexicographical_greater {
  auto operator()(Lhs const& lhs, Rhs const& rhs) const -> bool {
    auto const& ovl = mooutils::get_objective_vector(lhs);
    auto const& ovr = mooutils::get_objective_vector(rhs);
    return std::ranges::lexicographical_compare(ovr, ovl);
  }
};

template <typename Solution, typename Compare = lexicographical_greater<Solution>,
          typename Container = std::vector<Solution>>
requires mooutils::dominance_comparable<Solution> &&
    std::same_as<Solution, typename Container::value_type>
class sorted_multivector {
 public:
  using container_type = Container;
  using value_type = typename container_type::value_type;
  using size_type = typename container_type::size_type;
  using iterator = typename container_type::iterator;
  using compare = Compare;
  using const_iterator = typename container_type::const_iterator;

  template <typename S>
  requires mooutils::dominance_comparable<S, value_type> && std::convertible_to<S, value_type>
  constexpr auto insert(S&& solution) -> iterator {
    auto first = m_container.begin();
    auto last = m_container.end();
    auto mid1 = std::lower_bound(first, last, solution, compare{});
    auto mid2 = mid1;

    // Check for equivalent first, since it can allow for breaking
    // earlier
    bool is_equivalent = false;
    for (; mid2 != last; ++mid2) {
      if (equivalent(solution, *mid2)) {
        is_equivalent = true;
        if (solution == *mid2) {
          return last;
        }
      } else {
        break;
      }
    }
    if (is_equivalent) {
      return m_container.emplace(mid2, std::forward<S>(solution));
    }

    for (auto it = first; it != mid1; ++it) {
      if (dominates(*it, solution)) {
        return last;
      }
    }
    auto it = m_container.emplace(mid2, std::forward<S>(solution));
    // last needs to be updated since the previous operation can change it
    last = m_container.end();
    auto aux =
        std::remove_if(std::next(it), last, [it](auto const& s) { return dominates(*it, s); });
    m_container.erase(aux, last);
    return it;
  }

  template <typename S>
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    auto mid = std::lower_bound(m_container.begin(), m_container.end(), solution, compare{});
    return m_container.emplace(mid, std::forward<S>(solution));
  }

  constexpr auto erase(iterator it) {
    return m_container.erase(it);
  }

  constexpr auto erase(const_iterator it) {
    return m_container.erase(it);
  }

  constexpr auto erase(iterator first, iterator last) {
    return m_container.erase(first, last);
  }

  constexpr auto erase(const_iterator first, const_iterator last) {
    return m_container.erase(first, last);
  }

  [[nodiscard]] constexpr auto begin() {
    return m_container.begin();
  }

  [[nodiscard]] constexpr auto end() {
    return m_container.end();
  }

  [[nodiscard]] constexpr auto begin() const {
    return m_container.begin();
  }

  [[nodiscard]] constexpr auto end() const {
    return m_container.end();
  }

  [[nodiscard]] constexpr auto cbegin() const {
    return m_container.cbegin();
  }

  [[nodiscard]] constexpr auto cend() const {
    return m_container.cend();
  }

  [[nodiscard]] constexpr auto size() const {
    return m_container.size();
  }

  [[nodiscard]] constexpr auto empty() const {
    return m_container.empty();
  }

 private:
  container_type m_container;
};

template <typename Solution, typename Compare = lexicographical_greater<Solution>,
          typename Container = std::list<Solution>>
requires mooutils::dominance_comparable<Solution> &&
    std::same_as<Solution, typename Container::value_type>
class sorted_multilist {
 public:
  using value_type = Solution;
  using compare = Compare;
  using container_type = Container;
  using size_type = typename container_type::size_type;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  template <typename S>
  requires mooutils::dominance_comparable<S, value_type> && std::convertible_to<S, value_type>
  constexpr auto insert(S&& solution) -> iterator {
    auto first = m_container.begin();
    auto last = m_container.end();

    auto it = first;
    for (; it != last && compare()(*it, solution); ++it) {
      if (dominates(*it, solution)) {
        return last;
      }
    }

    bool is_equivalent = false;
    for (; it != last && !compare()(solution, *it); ++it) {
      if (equivalent(solution, *it)) {
        is_equivalent = true;
        if (solution == *it) {
          return last;
        }
      } else {
        break;
      }
    }

    it = m_container.emplace(it, std::forward<S>(solution));

    if (!is_equivalent) {
      for (auto jt = std::next(it); jt != last;) {
        if (dominates(*it, *jt)) {
          jt = m_container.erase(jt);
        } else {
          ++jt;
        }
      }
    }

    return it;
  }

  template <typename S>
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    auto mid = std::lower_bound(m_container.begin(), m_container.end(), solution, compare{});
    return m_container.emplace(mid, std::forward<S>(solution));
  }

  constexpr auto erase(iterator it) {
    return m_container.erase(it);
  }

  constexpr auto erase(const_iterator it) {
    return m_container.erase(it);
  }

  constexpr auto erase(iterator first, iterator last) {
    return m_container.erase(first, last);
  }

  constexpr auto erase(const_iterator first, const_iterator last) {
    return m_container.erase(first, last);
  }

  [[nodiscard]] constexpr auto begin() {
    return m_container.begin();
  }

  [[nodiscard]] constexpr auto end() {
    return m_container.end();
  }

  [[nodiscard]] constexpr auto begin() const {
    return m_container.begin();
  }

  [[nodiscard]] constexpr auto end() const {
    return m_container.end();
  }

  [[nodiscard]] constexpr auto cbegin() const {
    return m_container.cbegin();
  }

  [[nodiscard]] constexpr auto cend() const {
    return m_container.cend();
  }

  [[nodiscard]] constexpr auto size() const {
    return m_container.size();
  }

  [[nodiscard]] constexpr auto empty() const {
    return m_container.empty();
  }

 private:
  container_type m_container;
};

template <typename Solution, typename Compare = lexicographical_greater<Solution>,
          typename Container = std::multiset<Solution, Compare>>
requires mooutils::dominance_comparable<Solution> &&
    std::same_as<Solution, typename Container::value_type>
class sorted_multiset {
 public:
  using value_type = Solution;
  using compare = Compare;
  using container_type = Container;
  using size_type = typename container_type::size_type;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  template <typename S>
  requires mooutils::dominance_comparable<S, value_type> && std::convertible_to<S, value_type>
  constexpr auto insert(S&& solution) -> iterator {
    auto first = m_container.begin();
    auto last = m_container.end();
    auto mid1 = m_container.lower_bound(solution);
    auto mid2 = mid1;

    // Check for equivalent first, since it can allow for breaking
    // earlier
    bool is_equivalent = false;
    for (; mid2 != last; ++mid2) {
      if (equivalent(solution, *mid2)) {
        is_equivalent = true;
        if (solution == *mid2) {
          return last;
        }
      } else {
        break;
      }
    }
    if (is_equivalent) {
      return m_container.emplace_hint(mid2, std::forward<S>(solution));
    }

    for (auto it = first; it != mid1; ++it) {
      if (dominates(*it, solution)) {
        return last;
      }
    }
    auto it = m_container.emplace_hint(mid2, std::forward<S>(solution));
    for (auto jt = std::next(it); jt != m_container.end();) {
      if (dominates(*it, *jt)) {
        jt = m_container.erase(jt);
      } else {
        ++jt;
      }
    }
    return it;
  }

  template <typename S>
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    return m_container.emplace(std::forward<S>(solution));
  }

  constexpr auto erase(const_iterator it) {
    return m_container.erase(it);
  }

  constexpr auto erase(const_iterator first, const_iterator last) {
    return m_container.erase(first, last);
  }

  [[nodiscard]] constexpr auto begin() {
    return m_container.begin();
  }

  [[nodiscard]] constexpr auto end() {
    return m_container.end();
  }

  [[nodiscard]] constexpr auto begin() const {
    return m_container.begin();
  }

  [[nodiscard]] constexpr auto end() const {
    return m_container.end();
  }

  [[nodiscard]] constexpr auto cbegin() const {
    return m_container.cbegin();
  }

  [[nodiscard]] constexpr auto cend() const {
    return m_container.cend();
  }

  [[nodiscard]] constexpr auto size() const {
    return m_container.size();
  }

  [[nodiscard]] constexpr auto empty() const {
    return m_container.empty();
  }

 private:
  container_type m_container;
};

}  // namespace mooutils::solution_sets
