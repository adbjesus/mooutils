#pragma once

#include <mooutils/concepts.hpp>
#include <mooutils/dominance.hpp>
#include <mooutils/sets.hpp>

#include <algorithm>
#include <cassert>
#include <list>
#include <numeric>
#include <set>
#include <span>
#include <type_traits>
#include <vector>

namespace mooutils::indicators {

// TODO should the hypervolume free form functions be overloaded to not
// require a type, and instead deduce it? I think so.

// Computes the hypervolume of a single point.
//
// Assumptions:
//   - Maximizing objective values
//   - v.size() == r.size()
//   - v[i] >= r[i]
//   - r.size() > 1
template <typename T, typename P, typename R>
requires is_or_has_objective_vector<P> && is_or_has_objective_vector<R>
auto point_hv(P const& p, R const& r) -> T {
  return std::transform_reduce(p.begin(), p.end(),    // noformat
                               r.begin(),             // noformat
                               T{1},                  // noformat
                               std::multiplies<T>{},  // noformat
                               std::minus<T>{});
}

// Assumes that the set is sorted by mooutils::lexicographically_greater
// (or equivalent), and non-dominated, e.g. only contains mutually
// non-dominated solutions.
template <typename T, typename S, typename R>
requires solution_set<S> && is_or_has_objective_vector<std::ranges::range_value_t<S>> &&
    is_or_has_objective_vector<R>
auto sorted_set_hv2d(S&& set, R&& r) -> T {
  auto res = T{0};
  auto aux = std::array<T, 2>{r[0], r[1]};
  for (auto const& s : set) {
    auto const& ov = get_objective_vector(s);
    res += (ov[0] - aux[0]) * (ov[1] - aux[1]);
    aux[1] = ov[1];
  }
  return res;
}

template <typename T, typename S, typename R>
requires solution_set<S> && is_or_has_objective_vector<std::ranges::range_value_t<S>> &&
    is_or_has_objective_vector<R>
auto set_hv2d(S&& set, R&& r) -> T {
  if constexpr (std::is_const_v<std::remove_reference_t<S>>) {
    // If this is a const reference we make a copy with the objective
    // vectors, and sort it.
    auto ovs = objective_vectors(set);
    using ov_type = std::remove_cvref_t<std::ranges::range_value_t<decltype(ovs)>>;
    auto sorted_set = std::vector<ov_type>(ovs.begin(), ovs.end());
    std::ranges::sort(sorted_set, mooutils::lexicographically_greater<ov_type>{});
    return sorted_set_hv2d<T>(std::move(sorted_set), std::forward<R>(r));
  } else {
    // otherwise we sort the set itself
    std::ranges::sort(set, mooutils::lexicographically_greater<std::ranges::range_value_t<S>>{});
    return sorted_set_hv2d<T>(std::forward<S>(set), std::forward<R>(r));
  }
}

template <typename T, typename S, typename R>
requires solution_set<S> && is_or_has_objective_vector<R>
auto sorted_set_hv3d(S&& set, R&& r) -> T {
  using array2_t = std::array<T, 2>;

  auto aux = std::vector<array2_t>{{r[1], std::numeric_limits<T>::max()},
                                   {std::numeric_limits<T>::max(), r[2]}};

  auto v = T{0};
  auto a = T{0};
  auto z = T{0};

  for (auto const& p : set) {
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
}

template <typename T, typename S, typename R>
requires solution_set<S> && is_or_has_objective_vector<R>
auto set_hv3d(S&& set, R&& r) -> T {
  if constexpr (std::is_const_v<std::remove_reference_t<S>>) {
    // If this is a const reference we make a copy with the objective
    // vectors, and sort it.
    auto ovs = objective_vectors(set);
    using ov_type = std::remove_cvref_t<std::ranges::range_value_t<decltype(ovs)>>;
    auto sorted_set = std::vector<ov_type>(ovs.begin(), ovs.end());
    std::ranges::sort(sorted_set, mooutils::lexicographically_greater<ov_type>{});
    return sorted_set_hv3d<T>(std::move(sorted_set), std::forward<R>(r));
  } else {
    // otherwise we sort the set itself
    std::ranges::sort(set, mooutils::lexicographically_greater<std::ranges::range_value_t<S>>{});
    return sorted_set_hv3d<T>(std::forward<S>(set), std::forward<R>(r));
  }
}

template <typename T>
struct sorted_set_hvwfg_helper_fn {
 protected:
  template <typename S, typename V>
  [[nodiscard]] constexpr auto limitset(S const& set, V const& v) const {
    auto res = mooutils::sets::sorted_multivector<typename S::value_type>();
    res.reserve(set.size());
    for (auto const& p : set) {
      auto aux = p;
      std::transform(v.begin(), v.end(), aux.begin(), aux.begin(),
                     [](auto const& in1, auto const& in2) { return in1 < in2 ? in1 : in2; });
      res.insert(std::move(aux));
    }
    return res;
  }

  template <typename V, typename R>
  [[nodiscard]] constexpr auto inclhv(V const& v, R const& r) const -> T {
    return point_hv<T>(v, r);
  }

  template <typename S, typename V, typename R>
  [[nodiscard]] constexpr auto exclhv(S&& set, V&& v, R&& r, T c) const -> T {
    return c * inclhv(v, r) - wfg(limitset(set, get_objective_vector(v)), r, c);
  }

  template <typename S, typename R>
  [[nodiscard]] constexpr auto wfg(S&& set, R&& r, T c) const -> T {
    auto size = r.size();
    if (size == 2) {
      return c * sorted_set_hv2d<T>(std::forward<S>(set), std::forward<R>(r));
    } else if (size == 3) {
      return c * sorted_set_hv3d<T>(std::forward<S>(set), std::forward<R>(r));
    } else {
      auto newr = std::vector(r.begin() + 1, r.end());
      auto newset = mooutils::sets::sorted_multivector<std::vector<T>>();
      newset.reserve(set.size());
      auto v = T{0};
      for (auto const& p : set) {
        auto newc = c * (p[0] - r[0]);
        auto newp = std::vector<T>(p.begin() + 1, p.end());
        v += exclhv(newset, newp, newr, newc);
        newset.insert(std::move(newp));
      }
      return v;
    }
  }
};

template <typename T>
struct sorted_set_hvwfg_fn : public sorted_set_hvwfg_helper_fn<T> {
 public:
  template <typename S, typename R>
  requires solution_set<S> && is_or_has_objective_vector<R>
  [[nodiscard]] constexpr auto operator()(S&& set, R&& r) const -> T {
    return this->wfg(std::forward<S>(set), std::forward<R>(r), T{1});
  }
};

template <typename T>
inline constexpr sorted_set_hvwfg_fn<T> sorted_set_hvwfg;

template <typename T, typename S, typename R>
requires solution_set<S> && is_or_has_objective_vector<R>
[[nodiscard]] constexpr auto set_hvwfg(S&& set, R&& r) -> T {
  if constexpr (std::is_const_v<std::remove_reference_t<S>>) {
    // If this is a const reference we make a copy with the objective
    // vectors, and sort it.
    auto ovs = objective_vectors(set);
    using ov_type = std::remove_cvref_t<std::ranges::range_value_t<decltype(ovs)>>;
    auto sorted_set = std::vector<ov_type>(ovs.begin(), ovs.end());
    std::ranges::sort(sorted_set, mooutils::lexicographically_greater<ov_type>{});
    return sorted_set_hvwfg<T>(std::move(sorted_set), std::forward<R>(r));
  } else {
    // otherwise we sort the set itself
    std::ranges::sort(set, mooutils::lexicographically_greater<std::ranges::range_value_t<S>>{});
    return sorted_set_hvwfg<T>(std::forward<S>(set), std::forward<R>(r));
    // return aux<T>(std::forward<S>(set), std::forward<R>(r));
  }
}

// Computes the hypervolume of a set. It call different algorithms
// depending on the number of dimensions:
//   - 2d: calls set_hv2d
//   - 3d: calls set_hv3d
//   - nd: calls set_hvwfg
//
// Assumptions:
//   - Maximizing objective values
//   - r.size() > 1
//   - s[i].size() == r.size()
template <typename T, typename S, typename R>
requires solution_set<S> && is_or_has_objective_vector<R>
[[nodiscard]] constexpr auto set_hv(S&& set, R&& r) -> T {
  if (r.size() == 2) {
    return set_hv2d<T>(std::forward<S>(set), std::forward<R>(r));
  } else if (r.size() == 3) {
    return set_hv3d<T>(std::forward<S>(set), std::forward<R>(r));
  } else {
    return set_hvwfg<T>(std::forward<S>(set), std::forward<R>(r));
  }
}

// Class to keep the up to date hypervolume of a set of non-dominated
// objective vectors using the WFG algorithm.
//
// TODO Allow removing points
// TODO Allow initializing with multiple points at once
template <typename Value, typename ObjectiveVector = std::vector<Value>>
class [[nodiscard]] hypervolume_wfg : public sorted_set_hvwfg_helper_fn<Value> {
 public:
  using value_type = Value;
  using objective_vector_type = ObjectiveVector;
  using set_type = mooutils::sets::sorted_multivector<objective_vector_type>;

  template <typename R>
  constexpr explicit hypervolume_wfg(R&& r)
      : m_hv(0)
      , m_set()
      , m_ref(r.begin(), r.end()) {}

  // Get the current hypervolume value
  [[nodiscard]] constexpr auto value() const -> value_type {
    return m_hv;
  }

  // Get the contribution of a new vector w.r.t. to the current set
  template <typename V>
  requires mooutils::is_or_has_objective_vector<V>
  [[nodiscard]] constexpr auto contribution(V const& v) const -> value_type {
    if (v[0] <= m_ref[0] || v[1] <= m_ref[1]) {
      return 0;
    } else {
      return this->exclhv(m_set, get_objective_vector(v), m_ref, 1);
    }
  }

  // Inserts a new objective vector and returns its contribution
  template <typename V>
  requires mooutils::is_or_has_objective_vector<V>
  constexpr auto insert(V&& v) -> value_type {
    auto hvc = contribution(get_objective_vector(v));
    if (hvc > 0) {
      if constexpr (std::constructible_from<typename set_type::value_type, V>) {
        m_set.insert(get_objective_vector(std::forward<V>(v)));
      } else {
        auto&& ov = get_objective_vector(std::forward<V>(v));
        m_set.insert(objective_vector_type(ov.begin(), ov.end()));
      }
      m_hv += hvc;
    }
    return hvc;
  }

 private:
  value_type m_hv;
  set_type m_set;
  objective_vector_type m_ref;
};

template <typename Value, typename ObjectiveVector = std::array<Value, 2>>
class [[nodiscard]] hypervolume_2d {
  struct Cmp {
    template <typename Lhs, typename Rhs>
    [[nodiscard]] constexpr auto operator()(Rhs const& a, Lhs const& b) const -> bool {
      return a[0] > b[0];
    }
  };

 public:
  using value_type = Value;
  using objective_vector_type = ObjectiveVector;
  using set_type = std::vector<objective_vector_type>;

  template <typename R>
  constexpr explicit hypervolume_2d(R&& r)
      : m_hv(0)
      , m_set{{std::numeric_limits<value_type>::max(), r[1]},
              {r[0], std::numeric_limits<value_type>::max()}}
      , m_ref{r[0], r[1]} {}

  // Get the current hypervolume value
  [[nodiscard]] constexpr auto value() const {
    return m_hv;
  }

  // Get the contribution of a new vector w.r.t. to the current set
  template <typename V>
  requires is_or_has_objective_vector<V>
  [[nodiscard]] constexpr auto contribution(V const& v) const -> value_type {
    assert(v.size() == 2);
    if (v[0] <= m_ref[0] || v[1] <= m_ref[1]) {
      return 0;
    }

    auto it = std::prev(std::ranges::upper_bound(m_set, v, Cmp{}));
    // By definition v[0] <= (*it)[0], so we just need to check v[1]
    if (v[1] <= (*it)[1]) {
      return 0;
    }

    auto v0 = value_type{v[0]};
    auto r1 = value_type{(*it)[1]};
    auto res = value_type{0};
    do {
      ++it;
      res += (v0 - (*it)[0]) * (v[1] - r1);
      v0 = (*it)[0];
      r1 = (*it)[1];
    } while (v[1] >= (*it)[1]);
    return res;
  }

  // Get the contribution of a new vector w.r.t. to the current set
  template <typename V>
  requires is_or_has_objective_vector<V>
  constexpr auto insert(V&& v) -> value_type {
    assert(v.size() == 2);
    if (v[0] <= m_ref[0] || v[1] <= m_ref[1]) {
      return 0;
    }

    auto it = std::prev(std::ranges::upper_bound(m_set, v, Cmp{}));
    // By definition v[0] <= (*it)[0], so we just need to check v[1]
    if (v[1] <= (*it)[1]) {
      return 0;
    }

    auto first_erase = std::next(it);

    auto v0 = value_type{v[0]};
    auto r1 = value_type{(*it)[1]};
    auto res = value_type{0};
    do {
      ++it;
      res += (v0 - (*it)[0]) * (v[1] - r1);
      v0 = (*it)[0];
      r1 = (*it)[1];
    } while (v[1] >= (*it)[1]);

    auto last_erase = it;
    if (first_erase != last_erase) {
      (*first_erase)[0] = v[0];
      (*first_erase)[1] = v[1];
      m_set.erase(++first_erase, last_erase);
    } else {
      m_set.insert(last_erase, objective_vector_type{v[0], v[1]});
    }

    m_hv += res;

    return res;
  }

 private:
  value_type m_hv;
  set_type m_set;
  objective_vector_type m_ref;
};

// TODO Clean up this class a bit and make it safer. Not urgent, since
// several passes with valgrind on different data sets revealed no
// leaks, so the implementation seems to be sound.
template <typename Value, typename ObjectiveVector = std::array<Value, 3>>
class [[nodiscard]] hypervolume_3dplus {
 public:
  using value_type = Value;
  using objective_vector_type = ObjectiveVector;
  using objective_vector_value_type = typename ObjectiveVector::value_type;

 private:
  struct Point {
    Point(objective_vector_value_type _x, objective_vector_value_type _y,
          objective_vector_value_type _z)
        : x(_x)
        , y(_y)
        , z(_z)
        , prev(NULL)
        , next(NULL)
        , cprev(NULL)
        , cnext(NULL)
        , lprev(NULL)
        , lnext(NULL) {}

    objective_vector_value_type x;
    objective_vector_value_type y;
    objective_vector_value_type z;
    Point* prev;
    Point* next;
    Point* cprev;
    Point* cnext;
    Point* lprev;
    Point* lnext;
  };

 public:
  using set_type = Point*;

  template <typename V>
  constexpr explicit hypervolume_3dplus(V&& r)
      : m_hv(0)
      , m_ref{r[0], r[1], r[2]} {
    auto a = new Point(r[0], std::numeric_limits<objective_vector_value_type>::max(),
                       std::numeric_limits<objective_vector_value_type>::max());
    auto b = new Point(std::numeric_limits<objective_vector_value_type>::max(), r[1],
                       std::numeric_limits<objective_vector_value_type>::max());
    a->next = b;
    b->prev = a;
    b->cprev = a;
    m_set = a;
  }

  // TODO add copy constructor
  constexpr hypervolume_3dplus(hypervolume_3dplus const& other) = delete;

  constexpr hypervolume_3dplus(hypervolume_3dplus&& other) noexcept
      : m_hv(std::move(other.m_hv))
      , m_set(other.m_set)
      , m_ref(std::move(other.m_ref)) {
    other.m_set = NULL;
  }

  constexpr ~hypervolume_3dplus() {
    while (m_set != NULL) {
      auto n = m_set->next;
      delete m_set;
      m_set = n;
    }
  }

  // Get the current hypervolume value
  [[nodiscard]] constexpr auto value() const -> value_type {
    return m_hv;
  }

  // Get the contribution of a new vector w.r.t. to the current set
  template <typename V>
  [[nodiscard]] constexpr auto contribution(V const& u) const -> value_type {
    // Check if u is dominated by any point in q
    for (auto it = m_set; it != NULL && it->z >= u[2]; it = it->next) {
      if (it->x >= u[0] && it->y >= u[1]) {
        return 0;
      }
    }

    // Find outer delimeters
    Point* cprev = m_set;
    Point* cnext = m_set->next;

    cprev->lprev = NULL;
    cprev->lnext = cnext;
    cnext->lprev = cprev;
    cnext->lnext = NULL;

    auto p = m_set->next->next;
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

    // Find area contribution
    value_type a = compute_area_from_prev(u[0], u[1], cprev);

    value_type v = 0;
    auto z = u[2];
    for (; p != NULL && (p->x < u[0] || p->y < u[1]); p = p->next) {
      v += a * (z - p->z);
      z = p->z;
      value_type ac = 0;
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
      v += a * (z - m_ref[2]);
    } else {
      v += a * (z - p->z);
    }

    return v;
  }

  // Inserts a new objective vector and returns its contribution
  template <typename V>
  constexpr auto insert(V const& p) -> value_type {
    auto hvc = contribution(p);
    if (hvc == 0) {
      return 0;
    }
    m_hv += hvc;

    auto u = new Point(p[0], p[1], p[2]);

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

    for (auto it = m_set; it != NULL; it = it->next) {
      if (m_lex_ge(it, u)) {
        try_update_cnext(u, it);
        try_update_cprev(u, it);
      } else {
        try_update_cnext(it, u);
        try_update_cprev(it, u);
      }
    }

    // Remove non-dominated points in q
    auto weakly_dominates = [](Point* a, Point* b) {
      return a->x >= b->x && a->y >= b->y && a->z >= b->z;
    };

    for (auto it = m_set->next->next; it != NULL;) {
      if (weakly_dominates(u, it)) {
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
    auto prev = m_set->next;
    for (auto it = m_set->next->next; it != NULL; it = it->next) {
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

 private:
  auto m_lex_ge(Point* a, Point* b) {
    return (a->z > b->z || (a->z == b->z && (a->y > b->y || (a->y == b->y && a->x >= b->x))));
  }

  value_type m_hv;
  set_type m_set;
  objective_vector_type m_ref;
};

}  // namespace mooutils::indicators
