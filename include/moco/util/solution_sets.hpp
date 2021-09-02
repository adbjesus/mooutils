#pragma once

#include <moco/util/concepts.hpp>
#include <moco/util/dominance.hpp>

#include <concepts>
#include <iostream>
#include <iterator>
#include <ranges>
#include <set>
#include <vector>

namespace moco {

template <dominance_relation Solution, typename Container = std::vector<Solution>>
class ss_nondom_unordered_vec {
 public:
  using value_type = Solution;       // Type of the underlying solution
  using container_type = Container;  // Type of the underlying vector

  template <typename S>
  constexpr auto insert(S&& solution) -> bool {
    return m_insert(std::forward<S>(solution), [](auto const& lhs, auto const& rhs) {
      return moco::dominates(lhs, rhs) || (lhs.objective_vector() == rhs.objective_vector() &&
                                           lhs.decision_vector() == rhs.decision_vector());
    });
  }

  template <typename S>
  constexpr auto insert_unchecked(S&& solution) {
    return m_insert(std::forward<S>(solution), [](auto const& lhs, auto const& rhs) {
      return mobbb::weakly_dominates(lhs, rhs);
    });
  }

  auto begin() const {
    return m_container.begin();
  }

  auto end() const {
    return m_container.end();
  }

  auto size() const {
    return m_container.size();
  }

  auto empty() const {
    return m_container.empty();
  }

  template <typename Set>
  auto weakly_dominates(Set const& other) const {
    return m_dominates(
        other, [](auto const& lhs, auto const& rhs) { return mobbb::weakly_dominates(lhs, rhs); });
  }

  template <typename Set>
  auto dominates(Set const& other) const {
    return m_dominates(other,
                       [](auto const& lhs, auto const& rhs) { return mobbb::dominates(lhs, rhs); });
  }

  template <typename Set>
  auto strictly_dominates(Set const& other) const {
    return m_dominates(other, [](auto const& lhs, auto const& rhs) {
      return mobbb::strictly_dominates(lhs, rhs);
    });
  }

 private:
  template <typename S, typename DominatesPredicate>
  constexpr auto m_insert(S&& solution, DominatesPredicate dominates) {
    for (auto it = m_container.begin(); it != m_container.end(); ++it) {
      if (dominates(*it, solution)) {
        return false;
      }

      if (dominates(solution, *it)) {
        *it = std::move(m_container.back());
        m_container.erase(std::remove_if(it, std::prev(m_container.end()),
                                         [&solution, &dominates](auto const& el) {
                                           return dominates(solution, el);
                                         }),
                          m_container.end());
        break;
      }
    }

    m_container.push_back(std::forward<S>(solution));
    return true;
  }

  template <typename Set, typename DominatesPredicate>
  auto m_dominates(Set const& other, DominatesPredicate dominates) const {
    for (auto other_sol : other) {
      auto it = std::find_if(
          m_container.begin(), m_container.end(),
          [&other_sol, &dominates](auto const& sol) { return dominates(sol, other_sol); });

      if (it == m_container.end()) {
        return false;
      }
    }
    return true;
  }

  container_type m_container;
};

template <std::size_t I>
struct IndexCmp {
  template <std::ranges::input_range Vector>
  [[nodiscard]] constexpr auto operator()(Vector const& lhs, Vector const& rhs) const -> bool {
    return lhs[I] < rhs[I];
  }

  template <typename Solution>
  requires requires(Solution s) {
    { s.objective_vector() } -> std::ranges::input_range;
  }
  [[nodiscard]] constexpr auto operator()(Solution const& lhs, Solution const& rhs) const -> bool {
    return lhs.objective_vector()[I] < rhs.objective_vector()[I];
  }

  template <typename Solution, std::ranges::input_range Vector>
  requires requires(Solution s) {
    { s.objective_vector() } -> std::ranges::input_range;
  }
  [[nodiscard]] constexpr auto operator()(Solution const& lhs, Vector const& rhs) const -> bool {
    return lhs.objective_vector()[I] < rhs[I];
  }

  template <typename Solution, std::ranges::input_range Vector>
  requires requires(Solution s) {
    { s.objective_vector() } -> std::ranges::input_range;
  }
  [[nodiscard]] constexpr auto operator()(Vector const& lhs, Solution const& rhs) const -> bool {
    return lhs[I] < rhs.objective_vector()[I];
  }
};

template <std::ranges::input_range Vector>
[[nodiscard]] constexpr auto num_objectives(Vector const& v) {
  return v.size();
}

template <typename Solution>
requires requires(Solution s) {
  {s.objective_vector().size()};
}
[[nodiscard]] constexpr auto num_objectives(Solution const& s) {
  return s.objective_vector().size();
}

// Ordered solution vec according to index I
template <typename Solution, typename Cmp = IndexCmp<0>, typename Container = std::vector<Solution>>
class ordered_solution_vec {
 public:
  using value_type = Solution;       // Type of the underlying solution
  using container_type = Container;  // Type of the underlying vector

  template <typename S>
  requires std::same_as<std::decay_t<S>, value_type> && requires(value_type s) {
    {s.objective_vector()};
    {s.decision_vector()};
  }
  constexpr auto insert(S&& solution) {
    return m_insert(std::forward<S>(solution), [](auto const& lhs, auto const& rhs) {
      return mobbb::dominates(lhs, rhs) || (lhs.objective_vector() == rhs.objective_vector() &&
                                            lhs.decision_vector() == rhs.decision_vector());
    });
  }

  template <typename S>
  requires std::same_as<std::decay_t<S>, value_type> && std::ranges::input_range<value_type>
  constexpr auto insert(S&& solution) {
    return m_insert(std::forward<S>(solution), [](auto const& lhs, auto const& rhs) {
      return mobbb::weakly_dominates(lhs, rhs);
    });
  }

  auto begin() const {
    return m_container.begin();
  }

  auto end() const {
    return m_container.end();
  }

  auto size() const {
    return m_container.size();
  }

  auto empty() const {
    return m_container.empty();
  }

  template <typename Set>
  auto weakly_dominates(Set const& other) const {
    return m_dominates(
        other, [](auto const& lhs, auto const& rhs) { return mobbb::weakly_dominates(lhs, rhs); });
  }

  template <typename Set>
  auto dominates(Set const& other) const {
    return m_dominates(other,
                       [](auto const& lhs, auto const& rhs) { return mobbb::dominates(lhs, rhs); });
  }

  template <typename Set>
  auto strictly_dominates(Set const& other) const {
    return m_dominates(other, [](auto const& lhs, auto const& rhs) {
      return mobbb::strictly_dominates(lhs, rhs);
    });
  }

  template <typename Set>
  auto operator==(Set const& other) const -> bool {
    return m_container == other.m_container;
  }

 private:
  template <typename S, typename DominatesPredicate>
  constexpr auto m_insert(S&& solution, DominatesPredicate dominates) -> bool {
    auto lb = std::lower_bound(m_container.begin(), m_container.end(), solution, Cmp());

    if (num_objectives(solution) == 2) {
      if (lb != m_container.end() && dominates(*lb, solution)) {
        return false;
      }
    } else {
      if (std::find_if(lb, m_container.end(), [&solution, &dominates](auto const& el) {
            return dominates(el, solution);
          }) != m_container.end()) {
        return false;
      }
    }

    auto ub = std::upper_bound(m_container.begin(), m_container.end(), solution, Cmp());
    auto in = ub;

    if (num_objectives(solution) == 2) {
      auto aux = ub;
      for (; aux != m_container.begin() && dominates(solution, *std::prev(aux)); --aux) {}
      in = m_container.erase(aux, ub);
    } else {
      in = m_container.erase(std::remove_if(m_container.begin(), ub,
                                            [&solution, &dominates](auto const& el) {
                                              return dominates(solution, el);
                                            }),
                             ub);
    }

    m_container.insert(in, std::forward<S>(solution));
    return true;
  }

  template <typename Set, typename DominatesPredicate>
  auto m_dominates(Set const& other, DominatesPredicate dominates) const {
    for (auto other_sol : other) {
      auto it = std::lower_bound(m_container.begin(), m_container.end(), other_sol, Cmp());

      if (it == m_container.end()) {
        return false;
      }

      if (num_objectives(other_sol) == 2) {
        if (!dominates(*it, other_sol)) {
          return false;
        }
      } else {
        auto aux = std::find_if(
            m_container.begin(), m_container.end(),
            [&other_sol, &dominates](auto const& sol) { return dominates(sol, other_sol); });

        if (aux == m_container.end()) {
          return false;
        }
      }
    }
    return true;
  }

  container_type m_container;
};

template <typename Solution, typename Cmp = IndexCmp<0>,
          typename Container = std::multiset<Solution, Cmp>>
class ordered_solution_set {
 public:
  using value_type = Solution;       // Type of the underlying solution
  using container_type = Container;  // Type of the underlying vector

  template <typename S>
  requires std::same_as<std::decay_t<S>, value_type> && requires(value_type s) {
    {s.objective_vector()};
    {s.decision_vector()};
  }
  constexpr auto insert(S&& solution) {
    return m_insert(std::forward<S>(solution), [](auto const& lhs, auto const& rhs) {
      return mobbb::dominates(lhs, rhs) || (lhs.objective_vector() == rhs.objective_vector() &&
                                            lhs.decision_vector() == rhs.decision_vector());
    });
  }

  template <typename S>
  requires std::same_as<std::decay_t<S>, value_type> && std::ranges::input_range<value_type>
  constexpr auto insert(S&& solution) {
    return m_insert(std::forward<S>(solution), [](auto const& lhs, auto const& rhs) {
      return mobbb::weakly_dominates(lhs, rhs);
    });
  }

  auto begin() const {
    return m_container.begin();
  }

  auto end() const {
    return m_container.end();
  }

  auto size() const {
    return m_container.size();
  }

  auto empty() const {
    return m_container.empty();
  }

  template <typename Set>
  auto weakly_dominates(Set const& other) const {
    return m_dominates(
        other, [](auto const& lhs, auto const& rhs) { return mobbb::weakly_dominates(lhs, rhs); });
  }

  template <typename Set>
  auto dominates(Set const& other) const {
    return m_dominates(other,
                       [](auto const& lhs, auto const& rhs) { return mobbb::dominates(lhs, rhs); });
  }

  template <typename Set>
  auto strictly_dominates(Set const& other) const {
    return m_dominates(other, [](auto const& lhs, auto const& rhs) {
      return mobbb::strictly_dominates(lhs, rhs);
    });
  }

 private:
  template <typename S, typename DominatesPredicate>
  constexpr auto m_insert(S&& solution, DominatesPredicate dominates) -> bool {
    auto lb = m_container.lower_bound(solution);

    if (num_objectives(solution) == 2) {
      if (lb != m_container.end() && dominates(*lb, solution)) {
        return false;
      }
    } else {
      if (std::find_if(lb, m_container.end(), [&solution, &dominates](auto const& el) {
            return dominates(el, solution);
          }) != m_container.end()) {
        return false;
      }
    }

    auto ub = m_container.upper_bound(solution);
    auto in = ub;

    if (num_objectives(solution) == 2) {
      auto aux = ub;
      for (; aux != m_container.begin() && dominates(solution, *std::prev(aux)); --aux) {}
      in = m_container.erase(aux, ub);
    } else {
      for (in = m_container.begin(); in != ub;) {
        if (dominates(solution, *in)) {
          in = m_container.erase(in);
        } else {
          ++in;
        }
      }
    }

    m_container.insert(in, std::forward<S>(solution));
    return true;
  }

  template <typename Set, typename DominatesPredicate>
  auto m_dominates(Set const& other, DominatesPredicate dominates) const {
    for (auto const& other_sol : other) {
      auto it = m_container.lower_bound(other_sol);

      if (it == m_container.end()) {
        return false;
      }

      if (num_objectives(other_sol) == 2) {
        if (!dominates(*it, other_sol)) {
          return false;
        }
      } else {
        auto aux = std::find_if(
            m_container.begin(), m_container.end(),
            [&other_sol, &dominates](auto const& sol) { return dominates(sol, other_sol); });

        if (aux == m_container.end()) {
          return false;
        }
      }
    }
    return true;
  }

  container_type m_container;
};

}  // namespace moco
