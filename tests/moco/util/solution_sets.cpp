#include <catch2/catch.hpp>

#include <moco/util/dominance.hpp>
#include <moco/util/solution.hpp>
#include <moco/util/solution_sets.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cmath>
#include <deque>
#include <list>
#include <random>
#include <ranges>
#include <set>
#include <typeinfo>
#include <vector>

template <std::floating_point T, typename Rng>
auto generate_nondominated_points(size_t n, size_t m, Rng &rng) {
  std::normal_distribution<T> rnorm(0.0, 1.0);
  std::vector<std::vector<T>> pointset;
  for (int i = 0; i < n; i++) {
    std::vector<T> point;
    T sum = 0.0;
    for (int j = 0; j < m; j++) {
      point.push_back(rnorm(rng));
      sum += point[j] * point[j];
    }
    std::ranges::for_each(point, [&sum](auto &c) { c = std::abs(c / sqrt(sum)); });
    pointset.push_back(std::move(point));
  }
  return pointset;
}

template <std::floating_point T, typename Rng>
auto generate_prob_nondominated_points(size_t n, size_t m, double prob, Rng &rng) {
  std::vector<std::vector<T>> pointset = generate_nondominated_points<T>(n, m, rng);
  std::uniform_real_distribution<T> runif(0.0, 1.0);
  std::bernoulli_distribution rbern(1.0 - prob);
  for (auto &point : pointset) {
    if (rbern(rng)) {
      auto r = runif(rng);
      std::ranges::for_each(point, [&r](auto &c) { c *= r; });
    }
  }
  return pointset;
}

template <typename R>
auto filter_nondominated_points(R const &r) {
  auto aux = std::vector<std::ranges::range_value_t<R>>();
  aux.reserve(r.size());
  auto first = r.begin();
  auto last = r.end();
  for (auto it = first; it != last; ++it) {
    auto dominated =
        std::any_of(first, it,
                    [it](auto const &p) { return moco::dominates(p, *it) || p == *it; }) ||
        std::any_of(std::next(it), last, [it](auto const &p) { return moco::dominates(p, *it); });
    if (!dominated) {
      aux.emplace_back(*it);
    }
  }
  return aux;
}

using data_type = double;
using dvec_type = std::array<size_t, 1>;
using ovec_type = std::vector<data_type>;
using cvec_type = std::array<size_t, 0>;
using solution_type = moco::solution<dvec_type, ovec_type, cvec_type>;
using nd_solution_multiset_types = std::tuple<moco::nd_solution_multivector<solution_type>,
                                              moco::nd_ordered_solution_multivector<solution_type>,
                                              moco::nd_ordered_solution_multilist<solution_type>,
                                              moco::nd_ordered_solution_multiset<solution_type>>;

TEMPLATE_LIST_TEST_CASE("nd_solution_set with random solutions", "[nd_solution_set][template]",
                        nd_solution_multiset_types) {
  std::random_device rd("/dev/urandom");
  std::mt19937 rng(rd());

  for (size_t n = 10; n <= 1000; n *= 10) {
    for (size_t m = 2; m <= 10; m += 1) {
      for (double p = 0.3; p < 0.71; p += 0.1) {
        auto points = generate_prob_nondominated_points<data_type>(n, m, p, rng);
        auto solutions = std::vector<solution_type>();
        solutions.reserve(n);
        for (size_t i = 0; i < n; ++i) {
          solutions.emplace_back(dvec_type{i}, std::move(points[i]), cvec_type{}, true);
        }

        auto ndom_solutions = filter_nondominated_points(solutions);

        auto set = TestType();
        for (auto const &s : solutions) {
          if (weakly_dominates(set, s)) {
            REQUIRE(set.insert(s) == false);
          } else {
            REQUIRE(set.insert(s) == true);
          }
        }

        REQUIRE(set.size() == ndom_solutions.size());

        auto cmp = [](auto const &lhs, auto const &rhs) {
          return lhs.decision_vector() < rhs.decision_vector();
        };

        auto aux = std::vector(set.begin(), set.end());
        std::ranges::sort(aux, cmp);
        std::ranges::sort(ndom_solutions, cmp);

        REQUIRE(std::ranges::equal(aux, ndom_solutions));

        // A second pass through should not change the set
        for (auto const &s : solutions) {
          REQUIRE(set.insert(s) == false);
        }

        REQUIRE(set.size() == ndom_solutions.size());

        // it may change the order but not the items
        aux = std::vector(set.begin(), set.end());
        std::ranges::sort(aux, cmp);
        REQUIRE(std::ranges::equal(aux, ndom_solutions) == true);
      }
    }
  }
}

// Equivalent solutions are unlikely to appear in the random case, so
// this is a test to force it.
TEMPLATE_LIST_TEST_CASE("nd_solution_set with equivalent", "[nd_solution_set][template]",
                        nd_solution_multiset_types) {
  for (size_t n = 10; n <= 1000; n *= 10) {
    for (size_t m = 2; m <= 10; m += 1) {
      for (double p = 0.3; p < 0.71; p += 0.1) {
        auto solutions = std::vector<solution_type>();
        solutions.reserve(n);
        for (size_t i = 0; i < n; ++i) {
          solutions.emplace_back(dvec_type{i}, ovec_type(m, 0), cvec_type{}, true);
        }
        auto ndom_solutions = filter_nondominated_points(solutions);

        auto set = TestType();
        // Every solution should be added in the first pass
        for (auto const &s : solutions) {
          REQUIRE(set.insert(s) == true);
        }

        REQUIRE(set.size() == ndom_solutions.size());

        auto cmp = [](auto const &lhs, auto const &rhs) {
          return lhs.decision_vector() < rhs.decision_vector();
        };

        auto aux = std::vector(set.begin(), set.end());
        std::ranges::sort(aux, cmp);
        std::ranges::sort(ndom_solutions, cmp);

        REQUIRE(std::ranges::equal(set, ndom_solutions) == true);

        // A second pass should not add any solution
        for (auto const &s : solutions) {
          REQUIRE(set.insert(s) == false);
        }

        REQUIRE(set.size() == ndom_solutions.size());

        aux = std::vector(set.begin(), set.end());
        std::ranges::sort(aux, cmp);
        REQUIRE(std::ranges::equal(aux, ndom_solutions) == true);
      }
    }
  }
}
