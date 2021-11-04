#ifndef MOOUTILS_ORDERS_HPP_
#define MOOUTILS_ORDERS_HPP_

#include "concepts.hpp"
#include "solution.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace mooutils {

// TODO allow dominance relations with custom dominance order (currently always assumes maximizing)
// TODO allow different objective vector(s) views, instead of constraining on concepts
// TODO add execution policy?
// TODO add dominated variants?
// TODO add ordered set variants? would improved performance.

struct equivalent_fn {
 public:
  template <is_or_has_objective_vector V1, is_or_has_objective_vector V2>
  [[nodiscard]] constexpr auto operator()(V1 const& v1, V2 const& v2) const -> bool {
    auto const& ov1 = objective_vector(v1);
    auto const& ov2 = objective_vector(v2);
    assert(std::ranges::size(ov1) > 0);
    assert(std::ranges::size(ov2) > 0);
    assert(std::ranges::size(ov1) == std::ranges::size(ov2));
    auto first1 = std::ranges::begin(ov1);
    auto last1 = std::ranges::end(ov1);
    auto first2 = std::ranges::begin(ov2);
    for (; first1 != last1; ++first1, ++first2) {
      if (*first1 != *first2) {
        return false;
      }
    }
    return true;
  }

  template <is_or_has_objective_vector V, is_objective_vector_set S>
  [[nodiscard]] constexpr auto operator()(V const& v, S const& set) const -> bool {
    assert(std::ranges::size(set) > 0);
    return std::ranges::all_of(set, [this, &v](auto const& s) { return operator()(s, v); });
  }

  template <is_objective_vector_set S, is_or_has_objective_vector V>
  [[nodiscard]] constexpr auto operator()(S const& set, V const& v) const -> bool {
    assert(std::ranges::size(set) > 0);
    return operator()(v, set);
  }

  template <is_objective_vector_set S1, is_objective_vector_set S2>
  [[nodiscard]] constexpr auto operator()(S1 const& set1, S2 const& set2, bool sorted = false) const -> bool {
    assert(std::ranges::size(set1) > 0);
    assert(std::ranges::size(set2) > 0);

    if (sorted) {
      auto prev1 = std::ranges::begin(set1);
      auto last1 = std::ranges::end(set1);
      auto prev2 = std::ranges::begin(set2);
      auto last2 = std::ranges::end(set2);
      if (!operator()(*prev1, *prev2)) {
        return false;
      }
      auto cur1 = std::next(prev1);
      auto cur2 = std::next(prev2);
      while (cur1 != last1 && cur2 != last2) {
        if (operator()(*cur1, *cur2)) {
          prev1 = cur1++;
          prev2 = cur2++;
        } else if (operator()(*cur1, *prev2)) {
          prev1 = cur1++;
        } else if (operator()(*cur2, *prev1)) {
          prev2 = cur2++;
        } else {
          return false;
        }
      }
      for (; cur1 != last1; ++cur1) {
        if (!operator()(*cur1, *prev2)) {
          return false;
        }
      }
      for (; cur2 != last2; ++cur2) {
        if (!operator()(*cur2, *prev1)) {
          return false;
        }
      }
      return true;
    } else {
      // clang-format off
      return std::ranges::all_of(set1, [this, &set2](auto const& s1) {
        return std::ranges::any_of(set2, [this, &s1](auto const& s2) {
          return operator()(s1, s2);
        });
      }) && std::ranges::all_of(set2, [this, &set1](auto const& s2) {
        return std::ranges::any_of(set1, [this, &s2](auto const& s1) {
          return operator()(s1, s2);
        });
      });
      // clang-format on
    }
  }
};

inline constexpr equivalent_fn equivalent;

struct weakly_dominates_fn {
 public:
  template <is_or_has_objective_vector V1, is_or_has_objective_vector V2>
  [[nodiscard]] constexpr auto operator()(V1 const& v1, V2 const& v2) const -> bool {
    auto const& ov1 = objective_vector(v1);
    auto const& ov2 = objective_vector(v2);
    assert(std::ranges::size(ov1) > 0);
    assert(std::ranges::size(ov2) > 0);
    assert(std::ranges::size(ov1) == std::ranges::size(ov2));
    auto first1 = std::ranges::begin(ov1);
    auto last1 = std::ranges::end(ov1);
    auto first2 = std::ranges::begin(ov2);
    for (; first1 != last1; ++first1, ++first2) {
      if (*first1 < *first2) {
        return false;
      }
    }
    return true;
  }

  template <is_or_has_objective_vector V, is_objective_vector_set S>
  [[nodiscard]] constexpr auto operator()(V const& v, S const& set) const -> bool {
    assert(std::ranges::size(set) > 0);
    return std::ranges::all_of(set, [this, &v](auto const& s) { return operator()(v, s); });
  }

  template <is_objective_vector_set S, is_or_has_objective_vector V>
  [[nodiscard]] constexpr auto operator()(S const& set, V const& v) const -> bool {
    assert(std::ranges::size(set) > 0);
    return std::ranges::any_of(set, [this, &v](auto const& s) { return operator()(s, v); });
  }

  template <is_objective_vector_set S1, is_objective_vector_set S2>
  [[nodiscard]] constexpr auto operator()(S1 const& set1, S2 const& set2) const -> bool {
    assert(std::ranges::size(set1) > 0);
    assert(std::ranges::size(set2) > 0);

    // clang-format off
    return std::ranges::all_of(set2, [this, &set1](auto const& s2) {
      return std::ranges::any_of(set1, [this, &s2](auto const& s1) {
        return operator()(s1, s2);
      });
    });
    // clang-format on
  }
};

inline constexpr weakly_dominates_fn weakly_dominates;

struct dominates_fn {
 public:
  template <is_or_has_objective_vector V1, is_or_has_objective_vector V2>
  [[nodiscard]] constexpr auto operator()(V1 const& v1, V2 const& v2) const -> bool {
    auto const& ov1 = objective_vector(v1);
    auto const& ov2 = objective_vector(v2);
    assert(std::ranges::size(ov1) > 0);
    assert(std::ranges::size(ov2) > 0);
    assert(std::ranges::size(ov1) == std::ranges::size(ov2));
    auto first1 = std::ranges::begin(ov1);
    auto last1 = std::ranges::end(ov1);
    auto first2 = std::ranges::begin(ov2);
    for (; first1 != last1; ++first1, ++first2) {
      if (*first1 > *first2) {  // one strict inequality guaranteed
        for (++first1, ++first2; first1 != last1; ++first1, ++first2) {
          if (*first1 < *first2) {
            return false;
          }
        }
        return true;
      } else if (*first1 < *first2) {
        return false;
      }
    }
    return false;
  }

  template <is_or_has_objective_vector V, is_objective_vector_set S>
  [[nodiscard]] constexpr auto operator()(V const& v, S const& set) const -> bool {
    assert(std::ranges::size(set) > 0);
    // This is the same as strictly dominates.
    return std::ranges::all_of(set, [this, &v](auto const& s) { return operator()(v, s); });
  }

  template <is_objective_vector_set S, is_or_has_objective_vector V>
  [[nodiscard]] constexpr auto operator()(S const& set, V const& v) const -> bool {
    assert(std::ranges::size(set) > 0);
    auto last = std::ranges::end(set);
    for (auto it = std::ranges::begin(set); it != last; ++it) {
      if (!weakly_dominates(v, *it)) {
        // `v` does not weakly dominated `set`, find if `set` weakly dominates `v`
        return weakly_dominates(std::ranges::subrange(it, last), v);
      }
      if (weakly_dominates(*it, v)) {
        // `set` weakly dominates `v`, find if `v` does not weakly
        // dominate set (note that, we already checked this and previous
        // elements)
        if (++it != last) {
          return !weakly_dominates(v, std::ranges::subrange(it, last));
        } else {
          return false;
        }
      }
    }
    return false;
  }

  template <is_objective_vector_set S1, is_objective_vector_set S2>
  [[nodiscard]] constexpr auto operator()(S1 const& set1, S2 const& set2) const -> bool {
    assert(std::ranges::size(set1) > 0);
    assert(std::ranges::size(set2) > 0);
    return weakly_dominates(set1, set2) && !weakly_dominates(set2, set1);
  }
};

inline constexpr dominates_fn dominates;

struct strictly_dominates_fn {
 public:
  template <is_or_has_objective_vector V1, is_or_has_objective_vector V2>
  [[nodiscard]] constexpr auto operator()(V1 const& v1, V2 const& v2) const -> bool {
    auto const& ov1 = objective_vector(v1);
    auto const& ov2 = objective_vector(v2);
    assert(std::ranges::size(ov1) > 0);
    assert(std::ranges::size(ov2) > 0);
    assert(std::ranges::size(ov1) == std::ranges::size(ov2));
    auto first1 = std::ranges::begin(ov1);
    auto last1 = std::ranges::end(ov1);
    auto first2 = std::ranges::begin(ov2);
    for (; first1 != last1; ++first1, ++first2) {
      if (*first1 <= *first2) {
        return false;
      }
    }
    return true;
  }

  template <is_or_has_objective_vector V, is_objective_vector_set S>
  [[nodiscard]] constexpr auto operator()(V const& v, S const& set) const -> bool {
    assert(std::ranges::size(set) > 0);
    return std::ranges::all_of(set, [&v](auto const& s) { return dominates(v, s); });
  }

  template <is_objective_vector_set S, is_or_has_objective_vector V>
  [[nodiscard]] constexpr auto operator()(S const& set, V const& v) const -> bool {
    assert(std::ranges::size(set) > 0);
    return std::ranges::any_of(set, [&v](auto const& s) { return dominates(s, v); });
  }

  template <is_objective_vector_set S1, is_objective_vector_set S2>
  [[nodiscard]] constexpr auto operator()(S1 const& set1, S2 const& set2) const -> bool {
    assert(std::ranges::size(set1) > 0);
    assert(std::ranges::size(set2) > 0);

    // clang-format off
    return std::ranges::all_of(set2, [&](auto const& s2) {
      return std::ranges::any_of(set1, [&](auto const& s1) {
        return dominates(s1, s2);
      });
    });
    // clang-format on
  }
};

inline constexpr strictly_dominates_fn strictly_dominates;

struct incomparable_fn {
 public:
  template <is_or_has_objective_vector V1, is_or_has_objective_vector V2>
  [[nodiscard]] constexpr auto operator()(V1 const& v1, V2 const& v2) const -> bool {
    auto const& ov1 = objective_vector(v1);
    auto const& ov2 = objective_vector(v2);
    assert(std::ranges::size(ov1) > 0);
    assert(std::ranges::size(ov2) > 0);
    assert(std::ranges::size(ov1) == std::ranges::size(ov2));
    auto first1 = std::ranges::begin(ov1);
    auto last1 = std::ranges::end(ov1);
    auto first2 = std::ranges::begin(ov2);
    for (; first1 != last1; ++first1, ++first2) {
      if (*first1 > *first2) {
        for (++first1, ++first2; first1 != last1; ++first1, ++first2) {
          if (*first2 > *first1) {
            return true;
          }
        }
        return false;
      } else if (*first2 > *first1) {
        for (++first1, ++first2; first1 != last1; ++first1, ++first2) {
          if (*first1 > *first2) {
            return true;
          }
        }
        return false;
      }
    }
    return false;
  }

  template <is_or_has_objective_vector V, is_objective_vector_set S>
  [[nodiscard]] constexpr auto operator()(V const& v, S const& set) const -> bool {
    assert(std::ranges::size(set) > 0);
    auto last = std::ranges::end(set);
    for (auto it = std::ranges::begin(set); it != last; ++it) {
      if (weakly_dominates(*it, v)) {
        return false;
      }
      if (operator()(v, *it)) {
        // `v` is incomparable to `*it`, so we just need to check if `set` does not weakly dominate `v`
        for (++it; it != last; ++it) {
          if (weakly_dominates(*it, v)) {
            return false;
          }
        }
        return true;
      }
    }
    // this should be unreachable
    return false;
  }

  template <is_objective_vector_set S, is_or_has_objective_vector V>
  [[nodiscard]] constexpr auto operator()(S const& set, V const& v) const -> bool {
    assert(std::ranges::size(set) > 0);
    return operator()(v, set);
  }

  template <is_objective_vector_set S1, is_objective_vector_set S2>
  [[nodiscard]] constexpr auto operator()(S1 const& set1, S2 const& set2) const -> bool {
    assert(std::ranges::size(set1) > 0);
    assert(std::ranges::size(set2) > 0);
    return !weakly_dominates(set1, set2) && !weakly_dominates(set2, set1);
  }
};

inline constexpr incomparable_fn incomparable;

struct lexicographically_less_fn {
  template <is_or_has_objective_vector V1, is_or_has_objective_vector V2>
  auto operator()(V1 const& v1, V2 const& v2) const -> bool {
    auto const& ov1 = objective_vector(v1);
    auto const& ov2 = objective_vector(v2);
    assert(ov1.size() == ov2.size());
    return std::ranges::lexicographical_compare(ov1, ov2);
  }
};

inline constexpr lexicographically_less_fn lexicographically_less;

struct lexicographically_greater_fn {
  template <is_or_has_objective_vector V1, is_or_has_objective_vector V2>
  auto operator()(V1 const& v1, V2 const& v2) const -> bool {
    auto const& ov1 = objective_vector(v1);
    auto const& ov2 = objective_vector(v2);
    assert(ov1.size() == ov2.size());
    return std::ranges::lexicographical_compare(ov2, ov1);
  }
};

inline constexpr lexicographically_greater_fn lexicographically_greater;

struct lexicographically_equivalent_fn {
  template <is_or_has_objective_vector V1, is_or_has_objective_vector V2>
  auto operator()(V1 const& v1, V2 const& v2) const -> bool {
    auto const& ov1 = objective_vector(v1);
    auto const& ov2 = objective_vector(v2);
    assert(ov1.size() == ov2.size());
    return std::ranges::equal(ov1, ov2);
  }
};

inline constexpr lexicographically_equivalent_fn lexicographically_equivalent;

}  // namespace mooutils

#endif
