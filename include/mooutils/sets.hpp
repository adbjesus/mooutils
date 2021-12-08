#ifndef MOOUTILS_SETS_HPP_
#define MOOUTILS_SETS_HPP_

#include "concepts.hpp"
#include "orders.hpp"
#include "solution.hpp"

#include <concepts>
#include <iostream>
#include <iterator>
#include <list>
#include <ranges>
#include <set>
#include <vector>

namespace mooutils {

// CRTP Base class for sets based on an stl (or equivalent) container.
// This means, that it can implement most required functions by default.
template <typename Derived, typename Container>
requires is_or_has_objective_vector<typename Container::value_type>
class base_set {
  friend Derived;

 public:
  using value_type = Container::value_type;
  using reference = Container::reference;
  using const_reference = Container::const_reference;
  using iterator = Container::iterator;
  using const_iterator = Container::const_iterator;
  using difference_type = Container::difference_type;
  using size_type = Container::size_type;

  base_set()
      : c() {}

  explicit base_set(Container const& cont)
      : c(cont) {}

  explicit base_set(Container&& cont)
      : c(std::move(cont)) {}

  template <typename InputIt>
  base_set(InputIt first, InputIt last)
      : c(first, last) {}

  constexpr auto insert(value_type const& solution) -> iterator {
    return static_cast<Derived&>(*this).insert_impl(solution);
  }

  constexpr auto insert(value_type&& solution) -> iterator {
    return static_cast<Derived&>(*this).insert_impl(std::move(solution));
  }

  constexpr auto insert_unchecked(value_type const& solution) -> iterator {
    return static_cast<Derived&>(*this).insert_unchecked_impl(solution);
  }

  constexpr auto insert_unchecked(value_type&& solution) -> iterator {
    return static_cast<Derived&>(*this).insert_unchecked_impl(std::move(solution));
  }

  constexpr auto erase(const_iterator it) -> iterator {
    return c.erase(it);
  }

  constexpr auto erase(const_iterator first, const_iterator last) -> iterator {
    return c.erase(first, last);
  }

  [[nodiscard]] constexpr auto begin() -> iterator {
    return c.begin();
  }

  [[nodiscard]] constexpr auto end() -> iterator {
    return c.end();
  }

  [[nodiscard]] constexpr auto begin() const -> const_iterator {
    return c.begin();
  }

  [[nodiscard]] constexpr auto end() const -> const_iterator {
    return c.end();
  }

  [[nodiscard]] constexpr auto cbegin() const -> const_iterator {
    return c.cbegin();
  }

  [[nodiscard]] constexpr auto cend() const -> const_iterator {
    return c.cend();
  }

  [[nodiscard]] constexpr auto size() const -> size_type {
    return c.size();
  }

  [[nodiscard]] constexpr auto empty() const -> bool {
    return c.empty();
  }

 private:
  Container c;
};

template <typename Solution, typename Container = std::vector<Solution>>
class unordered_minimal_set : public base_set<unordered_minimal_set<Solution, Container>, Container> {
 private:
  using base_class_type = base_set<unordered_minimal_set<Solution, Container>, Container>;
  friend base_class_type;

 public:
  template <typename... Args>
  explicit unordered_minimal_set(Args&&... args)
      : base_class_type(std::forward<Args>(args)...) {}

 private:
  using typename base_class_type::iterator;

  template <typename S>
  constexpr auto insert_impl(S&& solution) -> iterator {
    auto const first = this->c.begin();
    auto const last = this->c.end();
    for (auto it = first; it != last; ++it) {
      if (dominates(*it, solution)) {
        return last;
      }
      if (weakly_dominates(solution, *it)) {
        if (equivalent(solution, *it)) {
          return last;
        } else {
          *it = std::forward<S>(solution);
          auto remove_pred = [it](auto const& s) {
            return dominates(*it, s);
          };
          this->c.erase(std::remove_if(std::next(it), last, remove_pred), last);
          return it;
        }
      }
    }
    return this->c.emplace(this->c.end(), std::forward<S>(solution));
  }

  template <typename S>
  constexpr auto insert_unchecked_impl(S&& solution) -> iterator {
    return this->c.emplace(this->c.end(), std::forward<S>(solution));
  }
};

template <typename Solution, typename Container = std::vector<Solution>>
class unordered_set : public base_set<unordered_set<Solution, Container>, Container> {
 private:
  using base_class_type = base_set<unordered_set<Solution, Container>, Container>;
  friend base_class_type;

 public:
  template <typename... Args>
  explicit unordered_set(Args&&... args)
      : base_class_type(std::forward<Args>(args)...) {}

 private:
  using typename base_class_type::iterator;

  template <typename S>
  constexpr auto insert_impl(S&& solution) -> iterator {
    auto const first = this->c.begin();
    auto const last = this->c.end();
    for (auto it = first; it != last; ++it) {
      if (dominates(*it, solution)) {
        return last;
      }
      if (weakly_dominates(solution, *it)) {
        if (equivalent(solution, *it)) {
          if (std::any_of(it, last, [solution](auto const& s) { return solution == s; })) {
            return last;
          } else {
            return this->c.emplace(this->c.end(), std::forward<S>(solution));
          }
        } else {
          *it = std::forward<S>(solution);
          auto remove_pred = [it](auto const& s) {
            return dominates(*it, s);
          };
          this->c.erase(std::remove_if(std::next(it), last, remove_pred), last);
          return it;
        }
      }
    }
    return this->c.emplace(this->c.end(), std::forward<S>(solution));
  }

  template <typename S>
  constexpr auto insert_unchecked_impl(S&& solution) -> iterator {
    return this->c.emplace(this->c.end(), std::forward<S>(solution));
  }
};

template <typename Solution,                                // noformat
          typename Compare = lexicographically_greater_fn,  // noformat
          typename Container = std::vector<Solution>>
class flat_minimal_set : public base_set<flat_minimal_set<Solution, Compare, Container>, Container> {
 private:
  using base_class_type = base_set<flat_minimal_set<Solution, Compare, Container>, Container>;
  friend base_class_type;

 public:
  using compare = Compare;

  template <typename... Args>
  explicit flat_minimal_set(Args&&... args)
      : base_class_type(std::forward<Args>(args)...) {}

 private:
  using typename base_class_type::iterator;

  template <typename S>
  constexpr auto insert_impl(S&& solution) -> iterator {
    auto first = this->c.begin();
    auto last = this->c.end();
    auto mid = std::lower_bound(first, last, solution, compare{});

    // Check for equivalent first
    if (mid != last && equivalent(solution, *mid)) {
      return last;
    }

    // TODO an optimization can be made if d==2
    for (auto it = first; it != mid; ++it) {
      if (weakly_dominates(*it, solution)) {
        return last;
      }
    }

    // TODO this could be improved to be O(N) instead of O(2N) if at least one item is to be removed
    auto it = this->c.emplace(mid, std::forward<S>(solution));
    last = this->c.end();
    auto aux = std::remove_if(std::next(it), last, [it](auto const& s) { return dominates(*it, s); });
    this->c.erase(aux, last);
    return it;
  }

  template <typename S>
  constexpr auto insert_unchecked_impl(S&& solution) -> iterator {
    auto mid = std::lower_bound(this->c.begin(), this->c.end(), solution, compare{});
    return this->c.emplace(mid, std::forward<S>(solution));
  }
};

template <typename Solution,                                // noformat
          typename Compare = lexicographically_greater_fn,  // noformat
          typename Container = std::vector<Solution>>
class flat_set : public base_set<flat_set<Solution, Compare, Container>, Container> {
 private:
  using base_class_type = base_set<flat_set<Solution, Compare, Container>, Container>;
  friend base_class_type;

 public:
  using compare = Compare;

  template <typename... Args>
  explicit flat_set(Args&&... args)
      : base_class_type(std::forward<Args>(args)...) {}

 private:
  using typename base_class_type::iterator;

  // TODO can be optimized for d == 2
  template <typename S>
  constexpr auto insert_impl(S&& solution) -> iterator {
    auto first = this->c.begin();
    auto last = this->c.end();
    auto mid1 = std::lower_bound(first, last, solution, compare{});
    auto mid2 = mid1;

    // Check for equivalent first, since it can allow for breaking
    // earlier
    for (; mid2 != last; ++mid2) {
      if (equivalent(solution, *mid2)) {
        if (solution == *mid2) {
          return last;
        }
      } else {
        break;
      }
    }
    if (mid1 != mid2) {
      return this->c.emplace(mid2, std::forward<S>(solution));
    }

    for (auto it = first; it != mid1; ++it) {
      if (dominates(*it, solution)) {
        return last;
      }
    }
    // TODO can be optimized if at least one item is to be removed
    auto it = this->c.emplace(mid2, std::forward<S>(solution));
    last = this->c.end();
    auto aux = std::remove_if(std::next(it), last, [it](auto const& s) { return dominates(*it, s); });
    this->c.erase(aux, last);
    return it;
  }

  template <typename S>
  constexpr auto insert_unchecked_impl(S&& solution) -> iterator {
    auto mid = std::lower_bound(this->c.begin(), this->c.end(), solution, compare{});
    return this->c.emplace(mid, std::forward<S>(solution));
  }
};

template <typename Solution,                                // noformat
          typename Compare = lexicographically_greater_fn,  // noformat
          typename Container = std::multiset<Solution, Compare>>
class minimal_set : public base_set<minimal_set<Solution, Compare, Container>, Container> {
 private:
  using base_class_type = base_set<minimal_set<Solution, Compare, Container>, Container>;
  friend base_class_type;

 public:
  using compare = Compare;

  template <typename... Args>
  explicit minimal_set(Args&&... args)
      : base_class_type(std::forward<Args>(args)...) {}

 private:
  using typename base_class_type::iterator;

  template <typename S>
  constexpr auto insert_impl(S&& solution) -> iterator {
    auto first = this->c.begin();
    auto last = this->c.end();
    auto mid = this->c.lower_bound(solution);

    // Check for equivalent first, since it can allow for breaking
    // earlier
    if (mid != last && equivalent(solution, *mid)) {
      return last;
    }

    for (auto it = first; it != mid; ++it) {
      if (dominates(*it, solution)) {
        return last;
      }
    }

    auto it = this->c.emplace_hint(mid, std::forward<S>(solution));
    for (auto jt = std::next(it); jt != this->c.end();) {
      if (dominates(*it, *jt)) {
        jt = this->c.erase(jt);
      } else {
        ++jt;
      }
    }
    return it;
  }

  template <typename S>
  constexpr auto insert_unchecked_impl(S&& solution) -> iterator {
    return this->c.emplace(std::forward<S>(solution));
  }
};

template <typename Solution,                                // noformat
          typename Compare = lexicographically_greater_fn,  // noformat
          typename Container = std::multiset<Solution, Compare>>
class set : public base_set<set<Solution, Compare, Container>, Container> {
 private:
  using base_class_type = base_set<set<Solution, Compare, Container>, Container>;
  friend base_class_type;

 public:
  using compare = Compare;

  template <typename... Args>
  explicit set(Args&&... args)
      : base_class_type(std::forward<Args>(args)...) {}

 private:
  using typename base_class_type::iterator;

  template <typename S>
  constexpr auto insert_impl(S&& solution) -> iterator {
    auto const& ov = objective_vector(solution);
    if (ov.size() == 2) {
      auto last = this->c.end();
      auto it = this->c.lower_bound(solution);
      auto jt = it;

      for (; jt != last; ++jt) {
        if (equivalent(solution, *jt)) {
          if (solution == *jt) {
            return last;
          }
        } else {
          break;
        }
      }
      if (it != jt) {
        return this->c.emplace_hint(jt, std::forward<S>(solution));
      }

      if (it != last && dominates(*it, solution)) {
        return last;
      }
      if (it != this->c.begin() && dominates(*std::prev(it), solution)) {
        return last;
      }

      it = this->c.emplace_hint(jt, std::forward<S>(solution));
      last = this->c.end();
      for (jt = std::next(it); jt != last && objective_vector(*it)[1] >= objective_vector(*jt)[1]; ++jt)
        ;
      this->c.erase(std::next(it), jt);
      return it;
    } else {
      auto first = this->c.begin();
      auto last = this->c.end();
      auto mid1 = this->c.lower_bound(solution);
      auto mid2 = mid1;

      // Check for equivalent first, since it can allow for breaking
      // earlier
      for (; mid2 != last; ++mid2) {
        if (equivalent(solution, *mid2)) {
          if (solution == *mid2) {
            return last;
          }
        } else {
          break;
        }
      }
      if (mid1 != mid2) {
        return this->c.emplace_hint(mid2, std::forward<S>(solution));
      }

      for (auto it = first; it != mid1; ++it) {
        if (dominates(*it, solution)) {
          return last;
        }
      }
      auto it = this->c.emplace_hint(mid2, std::forward<S>(solution));
      for (auto jt = std::next(it); jt != this->c.end();) {
        if (dominates(*it, *jt)) {
          jt = this->c.erase(jt);
        } else {
          ++jt;
        }
      }
      return it;
    }
  }

  template <typename S>
  constexpr auto insert_unchecked_impl(S&& solution) -> iterator {
    return this->c.emplace(std::forward<S>(solution));
  }
};

}  // namespace mooutils

#endif
