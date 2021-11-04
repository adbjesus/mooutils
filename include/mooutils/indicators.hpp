#ifndef MOOUTILS_INDICATORS_HPP_
#define MOOUTILS_INDICATORS_HPP_

#include "concepts.hpp"
#include "orders.hpp"
#include "sets.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <numeric>
#include <set>
#include <span>
#include <type_traits>
#include <variant>
#include <vector>

namespace mooutils {

// TODO should the hypervolume free form functions be overloaded to not
// require a type, and instead deduce it? I think so.

template <typename T>
struct hv2d_fn {
  template <is_or_has_objective_vector V, is_objective_vector R>
  [[nodiscard]] constexpr auto operator()(V const& v, R const& r) const -> T {
    auto const& ov = objective_vector(v);
    assert(ov[0] >= r[0]);
    assert(ov[1] >= r[1]);
    return T{(ov[0] - r[0])} * T{(ov[1] - r[1])};
  }

  template <is_objective_vector_set S, is_objective_vector R>
  [[nodiscard]] constexpr auto operator()(S const& set, R const& r, bool sorted = false) const -> T {
    if (sorted) {
      auto res = T{0};
      auto aux = std::array<T, 2>{r[0], r[1]};
      for (auto const& v : objective_vectors(set)) {
        res += (v[0] - aux[0]) * (v[1] - aux[1]);
        aux[1] = v[1];
      }
      return res;
    } else {
      auto ovs = objective_vectors(set);
      using ov_type = std::array<T, 2>;
      auto sorted_set = std::vector<ov_type>();
      sorted_set.reserve(set.size());
      for (auto const& v : ovs) {
        sorted_set.push_back(ov_type{v[0], v[1]});
      }
      std::ranges::sort(sorted_set, lexicographically_greater_fn{});
      return operator()(std::move(sorted_set), r, true);
    }
  }
};

template <typename T>
inline constexpr hv2d_fn<T> hv2d;

template <typename T>
struct hv3d_fn {
  template <is_or_has_objective_vector V, is_objective_vector R>
  [[nodiscard]] constexpr auto operator()(V const& v, R const& r) const -> T {
    auto const& ov = objective_vector(v);
    assert(ov[0] >= r[0]);
    assert(ov[1] >= r[1]);
    assert(ov[2] >= r[2]);
    return T{(ov[0] - r[0])} * T{(ov[1] - r[1])} * T{(ov[2] - r[2])};
  }

  template <is_objective_vector_set S, is_objective_vector R>
  [[nodiscard]] constexpr auto operator()(S const& set, R const& r, bool sorted = false) const -> T {
    if (sorted) {
      using array2_t = std::array<T, 2>;

      auto aux = std::vector<array2_t>{{r[1], std::numeric_limits<T>::max()}, {std::numeric_limits<T>::max(), r[2]}};

      auto v = T{0};
      auto a = T{0};
      auto z = T{0};

      for (auto const& p : objective_vectors(set)) {
        v += a * (z - p[0]);
        z = p[0];

        auto tmp = array2_t{p[1], p[2]};
        auto it = std::lower_bound(aux.begin(), aux.end(), tmp,
                                   [](auto const& lhs, auto const& rhs) { return lhs[1] > rhs[1]; });
        auto jt = it;

        auto ref = array2_t{(*std::prev(it))[0], tmp[1]};
        for (; (*it)[0] <= tmp[0]; ++it) {
          a += (tmp[0] - ref[0]) * (ref[1] - (*it)[1]);
          ref = *it;
        }
        a += (tmp[0] - ref[0]) * (ref[1] - (*it)[1]);
        if (jt != it) {
          *jt = tmp;
          aux.erase(++jt, it);
        } else {
          aux.insert(it, tmp);
        }
      }
      v += a * (z - r[0]);
      return v;
    } else {
      auto ovs = objective_vectors(set);
      using ov_type = std::array<T, 3>;
      auto sorted_set = std::vector<ov_type>();
      sorted_set.reserve(set.size());
      for (auto const& v : ovs) {
        sorted_set.push_back(ov_type{v[0], v[1], v[2]});
      }
      std::ranges::sort(sorted_set, lexicographically_greater_fn{});
      return operator()(std::move(sorted_set), r, true);
    }
  }
};

template <typename T>
inline constexpr hv3d_fn<T> hv3d;

template <typename T>
struct hvwfg_fn {
  template <is_or_has_objective_vector V, is_objective_vector R>
  [[nodiscard]] constexpr auto operator()(V const& v, R const& r) const -> T {
    auto const& ov = objective_vector(v);
    assert(mooutils::weakly_dominates(ov, r));
    assert(ov.size() > 1);
    assert(r.size() == ov.size());
    return std::transform_reduce(ov.begin(), ov.end(), r.begin(), T{1}, std::multiplies<T>{}, std::minus<T>{});
  }

  template <is_objective_vector_set S, is_objective_vector R>
  [[nodiscard]] constexpr auto operator()(S const& set, R const& r, bool sorted = false) const -> T {
    if (sorted) {
      return this->wfg(set, r, T{1});
    } else {
      auto ovs = objective_vectors(set);
      using ov_type = std::ranges::range_value_t<decltype(ovs)>;
      auto sorted_set = std::vector<ov_type>(ovs.begin(), ovs.end());
      std::ranges::sort(sorted_set, lexicographically_greater_fn{});
      return operator()(std::move(sorted_set), r, true);
    }
  }

 protected:
  template <is_objective_vector_set S, is_or_has_objective_vector V>
  [[nodiscard]] constexpr auto limitset(S const& set, V const& v) const {
    auto const& ov = objective_vector(v);
    auto const& ovs = objective_vectors(set);
    using ov_type = std::vector<T>;  // std::ranges::range_value_t<decltype(ovs)>;
    std::vector<ov_type> tmp;
    tmp.reserve(set.size());
    auto res = flat_minimal_set<ov_type>(std::move(tmp));
    for (auto const& p : ovs) {
      auto aux = ov_type();
      aux.reserve(p.size());
      std::transform(ov.begin(), ov.end(), p.begin(), std::back_inserter(aux),
                     [](auto const& in1, auto const& in2) { return in1 < in2 ? in1 : in2; });
      res.insert(std::move(aux));
    }
    return res;
  }

  template <is_objective_vector_set S, is_or_has_objective_vector V, is_objective_vector R>
  [[nodiscard]] constexpr auto exclhv(S const& set, V const& v, R const& r, T c) const -> T {
    return c * operator()(v, r) - this->wfg(limitset(set, v), r, c);
  }

  // TODO this currently does not support array like containers.
  template <is_objective_vector_set S, is_objective_vector R>
  [[nodiscard]] constexpr auto wfg(S const& set, R const& r, T c) const -> T {
    auto size = r.size();
    if (size == 2) {
      return c * hv2d<T>(set, r, true);
    } else if (size == 3) {
      return c * hv3d<T>(set, r, true);
    } else {
      auto ovs = objective_vectors(set);
      using ov_type = std::ranges::range_value_t<decltype(ovs)>;

      auto tmp = std::vector<ov_type>();
      tmp.reserve(set.size());
      auto newset = flat_minimal_set<ov_type>(std::move(tmp));

      auto newr = std::span(r.begin() + 1, r.end());

      auto v = T{0};
      for (auto const& p : ovs) {
        auto newc = c * (p[0] - r[0]);
        // TODO See if we can use a std::span to avoid copying
        auto newp = ov_type(p.begin() + 1, p.end());
        v += this->exclhv(newset, newp, newr, newc);
        newset.insert(std::move(newp));
      }

      return v;
    }
  }
};

template <typename T>
inline constexpr hvwfg_fn<T> hvwfg;

template <typename T>
struct hv_fn {
  template <is_or_has_objective_vector V, is_objective_vector R>
  [[nodiscard]] constexpr auto operator()(V const& v, R const& r) const -> T {
    auto const& ov = objective_vector(v);
    assert(mooutils::weakly_dominates(ov, r));
    assert(ov.size() > 1);
    assert(r.size() == ov.size());
    return std::transform_reduce(ov.begin(), ov.end(), r.begin(), T{1}, std::multiplies<T>{}, std::minus<T>{});
  }

  template <is_objective_vector_set S, is_objective_vector R>
  [[nodiscard]] constexpr auto operator()(S const& set, R const& r, bool sorted = false) const -> T {
    if (r.size() == 2) {
      return hv2d<T>(set, r, sorted);
    } else if (r.size() == 3) {
      return hv3d<T>(set, r, sorted);
    } else {
      return hvwfg<T>(set, r, sorted);
    }
  }
};

template <typename T>
inline constexpr hv_fn<T> hv;

// Pure virtual class for quality indicator structures that allow for
// incrementally updating a quality indicator with respect to the
// insertion of new solutions into the set.
//
// TODO allow initializing with a set of existing solutions (and
// optionally their hypervolume).
template <typename Value, typename Solution>
class increment_indicator {
 public:
  using value_type = Value;
  using solution_type = Solution;

  // Get the current indicator value.
  virtual auto value() const -> value_type = 0;

  // Find the contribution of a solution.
  virtual auto contribution(solution_type const& s) const -> value_type = 0;

  // Insert a new solution and return the contribution of that solution
  // w.r.t. to the indicator value.
  virtual auto insert(solution_type const& s) -> value_type = 0;
  virtual auto insert(solution_type&& s) -> value_type = 0;
};

template <typename Value, typename ObjectiveVector = std::vector<Value>>
class [[nodiscard]] incremental_hvwfg : hvwfg_fn<Value> {
 public:
  using value_type = Value;
  using objective_vector_type = ObjectiveVector;

  template <typename... ReferenceArgs>
  constexpr explicit incremental_hvwfg(ReferenceArgs&&... reference_args)
      : m_value(0)
      , m_reference(std::forward<ReferenceArgs>(reference_args)...)
      , m_solution_set() {}

  constexpr incremental_hvwfg(incremental_hvwfg const& other) = default;
  constexpr incremental_hvwfg(incremental_hvwfg&& other) = default;
  constexpr incremental_hvwfg& operator=(incremental_hvwfg const& other) = default;
  constexpr incremental_hvwfg& operator=(incremental_hvwfg&& other) = default;
  constexpr ~incremental_hvwfg() = default;

  [[nodiscard]] constexpr auto value() const -> value_type {
    return m_value;
  }

  template <typename S>
  [[nodiscard]] constexpr auto contribution(S const& s) const -> value_type {
    if (!mooutils::strictly_dominates(s, m_reference)) {
      return 0;
    } else {
      return this->exclhv(m_solution_set, s, m_reference, 1);
    }
  }

  template <typename S>
  constexpr auto insert(S&& s) -> value_type {
    auto c = contribution(s);
    if (c > 0) {
      m_solution_set.insert(std::forward<S>(mooutils::objective_vector(s)));
      m_value += c;
    }
    return c;
  }

 private:
  value_type m_value;
  objective_vector_type m_reference;
  unordered_minimal_set<objective_vector_type> m_solution_set;
};

template <typename Value>
class [[nodiscard]] incremental_hv2d {
  struct Cmp {
    template <typename Lhs, typename Rhs>
    [[nodiscard]] constexpr auto operator()(Rhs const& a, Lhs const& b) const -> bool {
      return a[0] > b[0];
    }
  };

 public:
  using value_type = Value;
  using objective_vector_type = std::array<value_type, 2>;
  using objective_vector_value_type = typename objective_vector_type::value_type;

  template <typename... ReferenceArgs>
  constexpr explicit incremental_hv2d(ReferenceArgs&&... reference_args)
      : m_value{0}
      , m_reference{std::forward<ReferenceArgs>(reference_args)...}
      , m_solution_set{{std::numeric_limits<objective_vector_value_type>::max(), m_reference[1]},
                       {m_reference[0], std::numeric_limits<objective_vector_value_type>::max()}} {}

  constexpr incremental_hv2d(incremental_hv2d&& other) = default;
  constexpr incremental_hv2d(incremental_hv2d const& other) = default;
  constexpr incremental_hv2d& operator=(incremental_hv2d&& other) = default;
  constexpr incremental_hv2d& operator=(incremental_hv2d const& other) = default;
  constexpr ~incremental_hv2d() = default;

  [[nodiscard]] constexpr auto value() const -> value_type {
    return m_value;
  }

  template <typename S>
  [[nodiscard]] constexpr auto contribution(S const& s) const -> value_type {
    if (!mooutils::strictly_dominates(s, m_reference)) {
      return 0;
    }

    auto const& ov = mooutils::objective_vector(s);
    auto it = std::prev(std::ranges::upper_bound(m_solution_set, ov, Cmp{}));
    // By definition objective_vector(*std::prev(it))[0] >= objective_vector(s)[0],
    // so we check weak dominance w.r.t. to [1]
    if ((*it)[1] >= s[1]) {
      return 0;
    }

    auto s0 = value_type{ov[0]};
    auto s1 = value_type{ov[1]};
    auto r1 = value_type{(*it)[1]};
    auto res = value_type{0};
    do {
      ++it;
      res += (s0 - (*it)[0]) * (s1 - r1);
      s0 = (*it)[0];
      r1 = (*it)[1];
    } while (s1 >= (*it)[1]);
    return res;
  }

  template <typename S>
  constexpr auto insert(S const& s) -> value_type {
    if (!mooutils::strictly_dominates(s, m_reference)) {
      return 0;
    }

    auto const& ov = mooutils::objective_vector(s);
    auto it = std::prev(std::ranges::upper_bound(m_solution_set, ov, Cmp{}));
    // By definition objective_vector(*std::prev(it))[0] >= objective_vector(s)[0],
    // so we check weak dominance w.r.t. to [1]
    if ((*it)[1] >= s[1]) {
      return 0;
    }

    auto first_erase = std::next(it);

    auto s0 = value_type{ov[0]};
    auto s1 = value_type{ov[1]};
    auto r1 = value_type{(*it)[1]};
    auto res = value_type{0};
    do {
      ++it;
      res += (s0 - (*it)[0]) * (s1 - r1);
      s0 = (*it)[0];
      r1 = (*it)[1];
    } while (s1 >= (*it)[1]);

    if (first_erase != it) {
      *first_erase = objective_vector_type{ov[0], ov[1]};
      m_solution_set.erase(++first_erase, it);
    } else {
      m_solution_set.insert(it, objective_vector_type{ov[0], ov[1]});
    }

    m_value += res;

    return res;
  }

  value_type m_value;
  objective_vector_type m_reference;
  std::vector<objective_vector_type> m_solution_set;
};

/// HV3D+ container based on "A. P. Guerreiro and C. M. Fonseca,
// "Computing and Updating Hypervolume Contributions in Up to Four
// Dimensions," in IEEE Transactions on Evolutionary Computation, vol.
// 22, no. 3, pp. 449-463, June 2018, doi: 10.1109/TEVC.2017.2729550."
//
// TODO Recheck the paper and implementation to see if it can be
// improved.
//
// TODO Clean up this class a bit and make it safer. Not urgent, since
// several passes with valgrind on different data sets revealed no
// leaks, so the implementation seems to be sound.
//
// TODO Allow for a custom allocator
//
// TODO Keep actual solutions instead of only the objective points (to
// be consistent with other indicator sets).
template <typename Value>
class [[nodiscard]] incremental_hv3dplus {
 public:
  using value_type = Value;
  using objective_vector_type = std::array<value_type, 3>;

 private:
  struct Point {
    Point(value_type _x, value_type _y, value_type _z)
        : x(_x)
        , y(_y)
        , z(_z)
        , prev(NULL)
        , next(NULL)
        , cprev(NULL)
        , cnext(NULL)
        , lprev(NULL)
        , lnext(NULL) {}

    value_type x;
    value_type y;
    value_type z;
    Point* prev;
    Point* next;
    Point* cprev;
    Point* cnext;
    Point* lprev;
    Point* lnext;
  };

 public:
  template <typename... ReferenceArgs>
  constexpr explicit incremental_hv3dplus(ReferenceArgs&&... reference_args)
      : m_value{0}
      , m_reference{std::forward<ReferenceArgs>(reference_args)...} {
    auto a = new Point(m_reference[0], std::numeric_limits<value_type>::max(), std::numeric_limits<value_type>::max());
    auto b = new Point(std::numeric_limits<value_type>::max(), m_reference[1], std::numeric_limits<value_type>::max());
    a->next = b;
    b->prev = a;
    b->cprev = a;
    m_solution_set = a;
  }

  // TODO Copy constructor is not trivial to implement, but also not
  // very important at the moment. I think there is the need to track
  // old and new pointers, to correctly copy all of prev, next, cprev,
  // cnext, lprev, lnext.
  constexpr incremental_hv3dplus(incremental_hv3dplus const& other) = delete;
  constexpr incremental_hv3dplus& operator=(incremental_hv3dplus const& other) = delete;

  constexpr incremental_hv3dplus(incremental_hv3dplus&& other) noexcept
      : m_value(std::move(other.m_value))
      , m_reference(std::move(other.m_reference))
      , m_solution_set(other.m_solution_set) {
    other.m_solution_set = NULL;
  }

  constexpr incremental_hv3dplus& operator=(incremental_hv3dplus&& other) noexcept {
    while (m_solution_set != NULL) {
      auto n = m_solution_set->next;
      delete m_solution_set;
      m_solution_set = n;
    }
    m_value = std::move(other.m_value);
    m_reference = std::move(other.m_reference);
    m_solution_set = std::move(other.m_solution_set);
    other.m_solution_set = NULL;
    return *this;
  }

  constexpr ~incremental_hv3dplus() {
    while (m_solution_set != NULL) {
      auto n = m_solution_set->next;
      delete m_solution_set;
      m_solution_set = n;
    }
  }

  // Get the current hypervolume value
  [[nodiscard]] constexpr auto value() const -> value_type {
    return m_value;
  }

  template <typename S>
  [[nodiscard]] constexpr auto contribution(S const& s) const -> value_type {
    auto const& u = mooutils::objective_vector(s);

    // Check if u is dominated by any point in q
    for (auto it = m_solution_set; it != NULL && it->z >= u[2]; it = it->next) {
      if (it->x >= u[0] && it->y >= u[1]) {
        return 0;
      }
    }

    // Utilities functions (TODO maybe move them to class functions?)
    auto compute_area_from_prev = [](value_type x, value_type y, Point* cprev) {
      value_type a = 0;
      auto rx = cprev->x;
      auto ry = y;
      auto it = cprev->lnext;
      for (; it->x <= x; it = it->lnext) {
        a += (x - rx) * (ry - it->y);
        rx = it->x;
        ry = it->y;
      }
      a += (x - rx) * (ry - it->y);
      return a;
    };

    auto compute_area_from_next = [](value_type x, value_type y, Point* cnext) {
      value_type a = 0;
      auto rx = x;
      auto ry = cnext->y;
      auto it = cnext->lprev;
      for (; it->y <= y; it = it->lprev) {
        a += (rx - it->x) * (y - ry);
        rx = it->x;
        ry = it->y;
      }
      a += (rx - it->x) * (y - ry);
      return a;
    };

    // Find outer delimeters
    Point* cprev = m_solution_set;
    Point* cnext = m_solution_set->next;

    cprev->lprev = NULL;
    cprev->lnext = cnext;
    cnext->lprev = cprev;
    cnext->lnext = NULL;

    auto p = m_solution_set->next->next;
    for (; p != NULL && p->z >= u[2]; p = p->next) {
      if (p->x < u[0] && p->y > u[1]) {
        if (p->x > cprev->x || (p->x == cprev->x && p->y > cprev->y)) {
          cprev = p;
        }
      }

      if (p->x > u[0] && p->y < u[1]) {
        if (p->y > cnext->y || (p->y == cnext->y && p->x > cnext->x)) {
          cnext = p;
        }
      }

      auto sp = p->cprev;
      auto sn = p->cnext;
      sp->lnext = p;
      sn->lprev = p;
      p->lprev = sp;
      p->lnext = sn;
    }

    // Find area contribution
    auto a = compute_area_from_prev(u[0], u[1], cprev);
    auto v = value_type{0};
    auto z = value_type{u[2]};
    for (; p != NULL && (p->x < u[0] || p->y < u[1]); p = p->next) {
      v += a * (z - p->z);
      z = p->z;
      auto ac = value_type{0};
      if (p->y >= u[1] && p->x >= cprev->x) {
        ac = compute_area_from_next(p->x, u[1], p->cnext);
        cprev = p;
      } else if (p->x >= u[0] && p->y >= cnext->y) {
        ac = compute_area_from_prev(u[0], p->y, p->cprev);
        cnext = p;
      } else if (p->x <= u[0] && p->y <= u[1]) {
        ac = compute_area_from_prev(p->x, p->y, p->cprev);
      } else {
        continue;
      }
      a -= ac;

      auto sp = p->cprev;
      auto sn = p->cnext;
      sp->lnext = p;
      p->lprev = sp;
      p->lnext = sn;
      sn->lprev = p;
    }

    if (p == NULL) {
      v += a * (z - m_reference[2]);
    } else {
      v += a * (z - p->z);
    }

    return v;
  }

  template <typename S>
  constexpr auto insert(S&& s) -> value_type {
    auto hvc = contribution(s);
    if (hvc == 0) {
      return 0;
    }
    m_value += hvc;

    auto const& p = mooutils::objective_vector(s);

    // Utility functions (TODO maybe move to class private methods)
    auto try_update_cprev = [](Point* u, Point* v) {
      if (v->x < u->x && v->y > u->y) {
        if (u->cprev == NULL) {
          u->cprev = v;
        } else if (v->x > u->cprev->x || (v->x == u->cprev->x && v->y > u->cprev->y)) {
          u->cprev = v;
        }
      }
    };

    auto try_update_cnext = [](Point* u, Point* v) {
      if (v->x > u->x && v->y < u->y) {
        if (u->cnext == NULL) {
          u->cnext = v;
        } else if (v->y > u->cnext->y || (v->y == u->cnext->y && v->x > u->cnext->x)) {
          u->cnext = v;
        }
      }
    };

    // Remove non-dominated points in q
    auto custom_weakly_dominates = [](Point* a, Point* b) {
      return a->x >= b->x && a->y >= b->y && a->z >= b->z;
    };

    auto u = new Point(p[0], p[1], p[2]);

    for (auto it = m_solution_set; it != NULL; it = it->next) {
      if (m_lex_ge(it, u)) {
        try_update_cnext(u, it);
        try_update_cprev(u, it);
      } else {
        try_update_cnext(it, u);
        try_update_cprev(it, u);
      }
    }

    for (auto it = m_solution_set->next->next; it != NULL;) {
      if (custom_weakly_dominates(u, it)) {
        if (it->next != NULL) {
          it->next->prev = it->prev;
        }
        if (it->prev != NULL) {
          it->prev->next = it->next;
        }

        auto aux = it->next;
        delete it;
        it = aux;
      } else {
        it = it->next;
      }
    }

    // Insert u in q
    auto prev = m_solution_set->next;
    for (auto it = m_solution_set->next->next; it != NULL; it = it->next) {
      if (m_lex_ge(u, it)) {
        u->next = it;
        u->prev = prev;
        it->prev = u;
        prev->next = u;
        break;
      }
      prev = it;
    }
    if (prev->next == NULL) {
      prev->next = u;
      u->prev = prev;
    }

    return hvc;
  }

  auto m_lex_ge(Point* a, Point* b) {
    return (a->z > b->z || (a->z == b->z && (a->y > b->y || (a->y == b->y && a->x >= b->x))));
  }

  value_type m_value;
  objective_vector_type m_reference;
  Point* m_solution_set;
};

template <typename Value, typename ObjectiveVector>
class [[nodiscard]] incremental_hv {
 public:
  using value_type = Value;
  using objective_vector_type = ObjectiveVector;

  template <typename S>
  incremental_hv(S&& s) {
    auto const& ov = mooutils::objective_vector(s);
    m_size = ov.size();
    if (m_size < 2) {
      throw("Size of objective vector must be at least 2");
    } else if (m_size == 2) {
      m_hv = decltype(m_hv)(std::in_place_index<1>, ov[0], ov[1]);
      // m_hv.emplace<1>(ov[0], ov[1]);
    } else if (m_size == 3) {
      m_hv = decltype(m_hv)(std::in_place_index<2>, ov[0], ov[1], ov[2]);
      // m_hv.emplace<2>(ov[0], ov[1], ov[2]);
    } else {
      m_hv = decltype(m_hv)(std::in_place_index<3>, ov);
      // m_hv.emplace<3>(ov);
    }
  }

  [[nodiscard]] constexpr auto value() const -> value_type {
    if (m_size == 2) {
      return std::get<1>(m_hv).value();
    } else if (m_size == 3) {
      return std::get<2>(m_hv).value();
    } else {
      return std::get<3>(m_hv).value();
    }
    return 0;
  }

  template <typename S>
  [[nodiscard]] constexpr auto contribution(S const& s) const -> value_type {
    if (m_size == 2) {
      return std::get<1>(m_hv).contribution(s);
    } else if (m_size == 3) {
      return std::get<2>(m_hv).contribution(s);
    } else {
      return std::get<3>(m_hv).contribution(s);
    }
    return 0;
  }

  template <typename S>
  constexpr auto insert(S&& s) -> value_type {
    if (m_size == 2) {
      return std::get<1>(m_hv).insert(std::forward<S>(s));
    } else if (m_size == 3) {
      return std::get<2>(m_hv).insert(std::forward<S>(s));
    } else {
      return std::get<3>(m_hv).insert(std::forward<S>(s));
    }
    return 0;
  }

 private:
  using hv2_type = incremental_hv2d<value_type>;
  using hv3_type = incremental_hv3dplus<value_type>;
  using hvd_type = incremental_hvwfg<value_type, objective_vector_type>;

  size_t m_size;
  std::variant<std::monostate, hv2_type, hv3_type, hvd_type> m_hv;
};

}  // namespace mooutils

#endif
