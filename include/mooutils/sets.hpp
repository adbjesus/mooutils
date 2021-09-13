#pragma once

#include <mooutils/concepts.hpp>
#include <mooutils/orders.hpp>
#include <mooutils/solution.hpp>

#include <concepts>
#include <iostream>
#include <iterator>
#include <list>
#include <ranges>
#include <set>
#include <vector>

namespace mooutils::sets {

template <typename Solution, typename Container>
requires std::same_as<Solution, typename Container::value_type>
class _set_base {
 public:
  using container_type = Container;
  using value_type = typename container_type::value_type;
  using reference = typename container_type::reference;
  using const_reference = typename container_type::const_reference;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;
  using difference_type = typename container_type::difference_type;
  using size_type = typename container_type::size_type;

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

 protected:
  container_type c;
};

template <typename Solution, typename Container = std::vector<Solution>>
class vector : public _set_base<Solution, Container> {
 private:
  using _set_base_type = _set_base<Solution, Container>;

 public:
  using typename _set_base_type::iterator;
  using typename _set_base_type::size_type;
  using typename _set_base_type::value_type;

  template <typename S>
  constexpr auto insert(S&& solution) -> iterator {
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
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    return this->c.emplace(this->c.end(), std::forward<S>(solution));
  }

  constexpr auto reserve(size_type n) -> void {
    this->c.reserve(n);
  }
};

template <typename Solution, typename Container = std::vector<Solution>>
class multivector : public _set_base<Solution, Container> {
 private:
  using _set_base_type = _set_base<Solution, Container>;

 public:
  using typename _set_base_type::iterator;
  using typename _set_base_type::size_type;
  using typename _set_base_type::value_type;

  template <typename S>
  constexpr auto insert(S&& solution) -> iterator {
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
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    return this->c.emplace(this->c.end(), std::forward<S>(solution));
  }

  constexpr auto reserve(size_type n) -> void {
    this->c.reserve(n);
  }
};

template <typename Solution, typename Compare = lexicographically_greater<Solution>,
          typename Container = std::vector<Solution>>
class sorted_vector : public _set_base<Solution, Container> {
 private:
  using _set_base_type = _set_base<Solution, Container>;

 public:
  using compare = Compare;
  using typename _set_base_type::iterator;
  using typename _set_base_type::size_type;
  using typename _set_base_type::value_type;

  template <typename S>
  constexpr auto insert(S&& solution) -> iterator {
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
    this->c.erase(std::remove_if(std::next(it), last, [it](auto const& s) { return dominates(*it, s); }), last);
    return it;
  }

  template <typename S>
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    auto mid = std::lower_bound(this->c.begin(), this->c.end(), solution, compare{});
    return this->c.emplace(mid, std::forward<S>(solution));
  }

  constexpr auto reserve(size_type n) -> void {
    this->c.reserve(n);
  }
};

template <typename Solution, typename Compare = lexicographically_greater<Solution>,
          typename Container = std::vector<Solution>>
class sorted_multivector : public _set_base<Solution, Container> {
 private:
  using _set_base_type = _set_base<Solution, Container>;

 public:
  using compare = Compare;
  using typename _set_base_type::iterator;
  using typename _set_base_type::size_type;
  using typename _set_base_type::value_type;

  // TODO can be optimized for d == 2
  template <typename S>
  constexpr auto insert(S&& solution) -> iterator {
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
    this->c.erase(std::remove_if(std::next(it), last, [it](auto const& s) { return dominates(*it, s); }), last);
    return it;
  }

  template <typename S>
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    auto mid = std::lower_bound(this->c.begin(), this->c.end(), solution, compare{});
    return this->c.emplace(mid, std::forward<S>(solution));
  }

  constexpr auto reserve(size_type n) -> void {
    this->c.reserve(n);
  }
};

template <typename Solution, typename Compare = lexicographically_greater<Solution>,
          typename Container = std::multiset<Solution, Compare>>
requires mooutils::dominance_comparable<Solution> && std::same_as<Solution, typename Container::value_type>
class sorted_set : public _set_base<Solution, Container> {
 private:
  using _set_base_type = _set_base<Solution, Container>;

 public:
  using compare = Compare;
  using typename _set_base_type::iterator;

  template <typename S>
  constexpr auto insert(S&& solution) -> iterator {
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
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    return this->c.emplace(std::forward<S>(solution));
  }
};

template <typename Solution, typename Compare = lexicographically_greater<Solution>,
          typename Container = std::multiset<Solution, Compare>>
requires mooutils::dominance_comparable<Solution> && std::same_as<Solution, typename Container::value_type>
class sorted_multiset : public _set_base<Solution, Container> {
 private:
  using _set_base_type = _set_base<Solution, Container>;

 public:
  using compare = Compare;
  using typename _set_base_type::iterator;

  template <typename S>
  constexpr auto insert(S&& solution) -> iterator {
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

  template <typename S>
  constexpr auto insert_unchecked(S&& solution) -> iterator {
    return this->c.emplace(std::forward<S>(solution));
  }
};

}  // namespace mooutils::sets
