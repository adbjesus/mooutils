#pragma once

#include <algorithm>
#include <cassert>
#include <list>
#include <set>
#include <type_traits>
#include <vector>

namespace mooutils::indicators {

// Class to keep the up to date hypervolume of a set of non-dominated
// objective vectors using the WFG algorithm. There are
// specializations for 2 and 3 objectives for faster calculations.
//
// For now this class assumes an initially empty set, and it allows
// inserting new non-dominated objective vectors. In the future, it
// may allow initialization with a set of points, as well as, the
// removal of points.
//
// Also it currently assumes that the objective vector size is known
// at compile time.
template <typename T, size_t N>
class [[nodiscard]] hypervolume {
  static_assert(N > 1);

 public:
  using value_type = T;
  using ovec_type = std::array<value_type, N>;
  using set_type = std::vector<ovec_type>;

  constexpr explicit hypervolume(ovec_type const& r)
      : m_hv(0)
      , m_set()
      , m_ref(r) {}

  constexpr hypervolume(hypervolume const& other) = default;
  constexpr hypervolume(hypervolume&& other) noexcept = default;

  // Get the current hypervolume value
  [[nodiscard]] constexpr auto value() const {
    return m_hv;
  }

  // Get the contribution of a new vector w.r.t. to the current set
  template <typename V>
  [[nodiscard]] constexpr auto contribution(V const& v) const {
    return m_point_hv(v, m_ref) - m_set_hv<N>(m_limit_set(m_set, v), m_ref);
  }

  // Inserts a new objective vector and returns its contribution
  template <typename V>
  constexpr auto insert(V&& v) {
    auto hvc = contribution(v);
    if (hvc != 0) {
      m_insert_non_dominated(std::forward<V>(v), m_set);
      m_hv += hvc;
    }
    return hvc;
  }

 private:
  // Check if a[1..] >= b[1..]
  template <typename V>
  [[nodiscard]] constexpr auto m_weakly_dominates(V const& a, V const& b) const {
    for (size_t i = 1; i < a.size(); ++i) {
      if (a[i] < b[i]) {
        return false;
      }
    }
    return true;
  }

  template <typename V>
  void m_insert_non_dominated(V&& v, std::vector<std::remove_cvref_t<V>>& set) const {
    auto it = set.begin();

    for (; it != set.end() && (*it)[0] > v[0]; ++it) {
      if (m_weakly_dominates(*it, v)) {
        return;
      }
    }

    for (; it != set.end() && (*it)[0] == v[0]; ++it) {
      if (m_weakly_dominates(*it, v)) {
        return;
      } else if (m_weakly_dominates(v, *it)) {
        *it = std::forward<V>(v);
        set.erase(std::remove_if(std::next(it), set.end(),
                                 [this, it](auto const& a) { return m_weakly_dominates(*it, a); }),
                  set.end());
        return;
      }
    }

    if (it == set.end()) {
      set.push_back(std::forward<V>(v));
    } else {
      auto aux = std::forward<V>(v);
      std::swap(aux, *it);
      for (auto jt = std::next(it); jt != set.end(); ++jt) {
        if (m_weakly_dominates(*it, aux)) {
          set.erase(
              std::remove_if(jt, set.end(),
                             [this, it](auto const& a) { return m_weakly_dominates(*it, a); }),
              set.end());
          return;
        } else {
          std::swap(aux, *jt);
        }
      }
      if (!m_weakly_dominates(*it, aux)) {
        set.push_back(std::move(aux));
      }
    }
  }

  template <typename S, typename V>
  auto m_limit_set(S const& s, V const& v) const {
    S res;
    res.reserve(s.size());
    for (auto const& p : s) {
      auto aux = p;
      for (size_t i = 0; i < aux.size(); ++i) {
        aux[i] = std::min(aux[i], v[i]);
      }
      m_insert_non_dominated(std::move(aux), res);
    }
    return res;
  }

  template <typename V, typename R>
  auto m_point_hv(V const& v, R const& r) const {
    value_type res{1};
    for (size_t i = 0; i < v.size(); ++i) {
      res *= v[i] - r[i];
    }
    return res;
  }

  template <typename S, typename R>
  auto m_set_hv3d(S const& s, R const& r) const {
    using array2_t = std::array<value_type, 2>;

    auto aux = std::vector<array2_t>{{r[1], std::numeric_limits<value_type>::max()},
                                     {std::numeric_limits<value_type>::max(), r[2]}};

    value_type v = 0;
    value_type a = 0;
    value_type z = 0;

    for (auto const& p : s) {
      v += a * (z - p[0]);
      z = p[0];

      auto tmp = array2_t{p[1], p[2]};
      auto it = std::lower_bound(aux.begin(), aux.end(), tmp,
                                 [](auto const& a, auto const& b) { return a[1] > b[1]; });
      auto jt = it;

      auto r0 = (*std::prev(it))[0];
      auto r1 = tmp[1];
      for (; (*it)[0] <= tmp[0]; ++it) {
        a += (tmp[0] - r0) * (r1 - (*it)[1]);
        r0 = (*it)[0];
        r1 = (*it)[1];
      }
      a += (tmp[0] - r0) * (r1 - (*it)[1]);
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

  // Assumes set is sorted by increasing i
  template <std::size_t I, typename S, typename R>
  auto m_set_hv(S const& s, R const& r, value_type c = 1) const -> value_type {
    value_type v = 0;

    if (s.size() == 0) {
      return 0;
    }

    if constexpr (I == 2) {
      value_type r1 = r[1];
      for (auto const& p : s) {
        v += (p[1] - r1) * (p[0] - r[0]);
        r1 = p[1];
      }
      v *= c;
    } else if constexpr (I == 3) {
      v = c * m_set_hv3d(s, r);
    } else {
      auto newr = std::array<value_type, I - 1>{};
      for (size_t i = 1; i < I; ++i) {
        newr[i - 1] = r[i];
      }
      auto newl = std::vector<std::array<value_type, I - 1>>();
      newl.reserve(s.size());
      for (auto&& p : s) {
        auto newc = c * (p[0] - r[0]);
        auto newp = std::array<value_type, I - 1>{};
        for (size_t i = 1; i < I; ++i) {
          newp[i - 1] = p[i];
        }

        v += newc * m_point_hv(newp, newr) - m_set_hv<I - 1>(m_limit_set(newl, newp), newr, newc);

        m_insert_non_dominated(std::move(newp), newl);
      }
    }
    return v;
  }

  value_type m_hv;
  set_type m_set;
  ovec_type m_ref;
};

template <typename T>
class [[nodiscard]] hypervolume<T, 2> {
  struct Cmp {
    [[nodiscard]] constexpr auto operator()(auto const& a, auto const& b) const -> bool {
      return a[1] > b[1];
    }
  };

 public:
  using value_type = T;
  using ovec_type = std::array<value_type, 2>;
  using set_type = std::set<ovec_type, Cmp>;

  constexpr explicit hypervolume(ovec_type const& r)
      : m_hv(0)
      , m_set()
      , m_ref(r) {}

  constexpr hypervolume(hypervolume const& other) = default;
  constexpr hypervolume(hypervolume&& other) noexcept = default;

  // Get the current hypervolume value
  [[nodiscard]] constexpr auto value() const {
    return m_hv;
  }

  // Get the contribution of a new vector w.r.t. to the current set
  template <typename V>
  [[nodiscard]] constexpr auto contribution(V const& v) const -> value_type {
    assert(v.size() == 2);
    value_type res = 0;
    auto it = m_set.lower_bound(v);
    value_type r0 = it != m_set.begin() ? (*std::prev(it))[0] : m_ref[0];

    if ((v[1] == (*it)[1] && v[0] <= (*it)[0]) || v[0] <= r0) {
      return 0;
    }

    value_type v1 = v[1];
    for (; it != m_set.end() && (*it)[0] <= v[0]; ++it) {
      res += (v[0] - r0) * (v1 - (*it)[1]);
      r0 = (*it)[0];
      v1 = (*it)[1];
    }
    if (it != m_set.end()) {
      res += (v[0] - r0) * (v1 - (*it)[1]);
    } else {
      res += (v[0] - r0) * (v1 - m_ref[1]);
    }
    return res;
  }

  // Inserts a new objective vector and returns its contribution
  template <typename V>
  constexpr auto insert(V&& v) -> value_type {
    assert(v.size() == 2);
    value_type hvc = 0;

    auto it = m_set.lower_bound(v);
    value_type r0 = it != m_set.begin() ? (*std::prev(it))[0] : m_ref[0];

    if ((v[1] == (*it)[1] && v[0] <= (*it)[0]) || v[0] <= r0) {
      return 0;
    }

    value_type v1 = v[1];
    auto jt = it;
    for (; jt != m_set.end() && (*jt)[0] <= v[0]; ++jt) {
      hvc += (v[0] - r0) * (v1 - (*jt)[1]);
      r0 = (*jt)[0];
      v1 = (*jt)[1];
    }

    if (jt != m_set.end()) {
      hvc += (v[0] - r0) * (v1 - (*jt)[1]);
    } else {
      hvc += (v[0] - r0) * (v1 - m_ref[1]);
    }

    m_set.insert(m_set.erase(it, jt), std::forward<V>(v));

    m_hv += hvc;

    return hvc;
  }

 private:
  value_type m_hv;
  set_type m_set;
  ovec_type m_ref;
};

// Uses hv3d plus
template <typename T>
class [[nodiscard]] hypervolume<T, 3> {
  struct Point {
    Point(T x, T y, T z)
        : x(x)
        , y(y)
        , z(z)
        , prev(NULL)
        , next(NULL)
        , cprev(NULL)
        , cnext(NULL)
        , lprev(NULL)
        , lnext(NULL) {}

    T x;
    T y;
    T z;
    Point* prev;
    Point* next;
    Point* cprev;
    Point* cnext;
    Point* lprev;
    Point* lnext;
  };

 public:
  using value_type = T;
  using ovec_type = std::array<value_type, 3>;
  using set_type = Point*;

  constexpr explicit hypervolume(ovec_type const& r)
      : m_hv(0)
      , m_ref(r) {
    auto a = new Point(r[0], std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
    auto b = new Point(std::numeric_limits<T>::max(), r[1], std::numeric_limits<T>::max());
    a->next = b;
    b->prev = a;
    b->cprev = a;
    m_set = a;
  }

  constexpr hypervolume(hypervolume const& other) = delete;
  constexpr hypervolume(hypervolume&& other) noexcept
      : m_hv(std::move(other.m_hv))
      , m_set(other.m_set)
      , m_ref(std::move(other.m_ref)) {
    other.m_set = NULL;
  }

  constexpr ~hypervolume() {
    while (m_set != NULL) {
      auto n = m_set->next;
      delete m_set;
      m_set = n;
    }
  }

  // Get the current hypervolume value
  [[nodiscard]] constexpr auto value() const {
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
  ovec_type m_ref;
};

// Assumptions:
//   - Maximizing objective values
//   - v.size() == r.size()
//   - v[i] >= r[i]
//   - r.size() > 1
template <typename V, typename R>
auto point_hv(V const& v, R const& r) {
  using result_t = std::common_type_t<typename V::value_type, typename R::value_type>;
  result_t res = 1;
  for (size_t i = 0; i < v.size(); ++i) {
    res *= v[i] - r[i];
  }
  return res;
}

// Assumptions:
//   - Maximizing objective values
//   - Set `s` contains only mutually non-dominated points
//   - r.size() > 1
//   - s[i].size() == r.size()
template <typename S, typename R>
auto set_hv(S const& s, R const& r) {
  auto v = std::vector(s.begin(), s.end());
  sort(v.begin(), v.end(), [](auto const& a, auto const& b) { return a[0] > b[0]; });
  return sorted_set_hv(v, r);
}

// Assumptions:
//   - Maximizing objective values
//   - Set `s` contains only mutually non-dominated points
//   - Set `s` is sorted in worsening order oif the first objective value
//   - r.size() > 1
//   - s[i].size() == r.size()
template <typename S, typename R>
auto sorted_set_hv(S const& s, R const& r) {
  if constexpr (r.size() == 2) {
    return sorted_set_hv2(s, r);
  } else if constexpr (r.size() == 3) {
    return sorted_set_hv3d(s, r);
  } else {
    return sorted_set_hv_wfg(s, r);
  }
}

// Assumptions:
//   - Maximizing objective values
//   - Set `s` contains only mutually non-dominated points
//   - Set `s` is sorted in worsening order of the first objective value
//   - r.size() == s[i].size() == 2
template <typename S, typename R>
auto sorted_set_hv2(S const& s, R const& r) {
  using result_t = std::common_type_t<typename S::value_type::value_type, typename R::value_type>;
  result_t res = 0;
  result_t aux = r[1];
  for (auto const& v : s) {
    res += (v[1] - aux) * (v[0] - r[0]);
    aux = v[1];
  }
  return res;
}

// Assumptions:
//   - Maximizing objective values
//   - Set `s` contains only mutually non-dominated points
//   - Set `s` is sorted in worsening order of the first objective value
//   - r.size() == s[i].size() == 3
template <typename S, typename R>
auto sorted_set_hv3d(S const& s, R const& r) {
  using result_t = std::common_type_t<typename S::value_type::value_type, typename R::value_type>;
  using array2_t = std::array<result_t, 2>;

  auto aux = std::vector<array2_t>{{r[1], std::numeric_limits<result_t>::max()},
                                   {std::numeric_limits<result_t>::max(), r[2]}};

  result_t vol = 0;
  result_t area = 0;
  result_t r0 = 0;

  for (auto const& v : s) {
    vol += area * (r0 - v[0]);
    r0 = v[0];

    auto tmp = array2_t{v[1], v[2]};
    auto it = std::lower_bound(aux.begin(), aux.end(), tmp,
                               [](auto const& a, auto const& b) { return a[1] > b[1]; });
    auto jt = it;

    auto r0 = (*std::prev(it))[0];
    auto r1 = tmp[1];
    for (; (*it)[0] <= tmp[0]; ++it) {
      area += (tmp[0] - r0) * (r1 - (*it)[1]);
      r0 = (*it)[0];
      r1 = (*it)[1];
    }
    area += (tmp[0] - r0) * (r1 - (*it)[1]);
    if (jt != it) {
      *jt = tmp;
      aux.erase(++jt, it);
    } else {
      aux.insert(it, tmp);
    }
  }
  vol += area * (r0 - r[0]);

  return vol;
}

}  // namespace mooutils::indicators
