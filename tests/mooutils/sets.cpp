#include <catch2/catch.hpp>

#include <mooutils/orders.hpp>
#include <mooutils/sets.hpp>
#include <mooutils/solution.hpp>

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
  for (size_t i = 0; i < n; i++) {
    std::vector<T> point;
    T sum = 0.0;
    for (size_t j = 0; j < m; j++) {
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
auto filter_nondominated_points(R const &r, bool remove_equivalent) {
  auto aux = std::vector<std::ranges::range_value_t<R>>();
  aux.reserve(r.size());
  auto first = r.begin();
  auto last = r.end();
  for (auto it = first; it != last; ++it) {
    bool dominated;
    if (remove_equivalent) {
      dominated = std::any_of(first, it, [it](auto const &p) { return mooutils::weakly_dominates(p, *it); }) ||
                  std::any_of(std::next(it), last, [it](auto const &p) { return mooutils::dominates(p, *it); });
    } else {
      dominated = std::any_of(first, it, [it](auto const &p) { return mooutils::dominates(p, *it) || p == *it; }) ||
                  std::any_of(std::next(it), last, [it](auto const &p) { return mooutils::dominates(p, *it); });
    }
    if (!dominated) {
      aux.emplace_back(*it);
    }
  }
  return aux;
}

using data_type = double;
using dvec_type = std::array<size_t, 1>;
using ovec_type = std::vector<data_type>;
using solution_type = mooutils::unconstrained_solution<dvec_type, ovec_type>;
using multisets_types = std::tuple<mooutils::unordered_set<solution_type>,  // noformat
                                   mooutils::flat_set<solution_type>,       // noformat
                                   mooutils::set<solution_type>>;

TEMPLATE_LIST_TEST_CASE("multisets with random solutions", "[sets][template]", multisets_types) {
  std::random_device rd("/dev/urandom");
  std::mt19937 rng(rd());

  size_t n = GENERATE(10, 100, 1000);
  size_t m = GENERATE(2, 3, 5, 7);
  double p = GENERATE(0.3, 0.5, 0.7);

  auto points = generate_prob_nondominated_points<data_type>(n, m, p, rng);
  auto solutions = std::vector<solution_type>();
  solutions.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    solutions.emplace_back(dvec_type{i}, std::move(points[i]));
  }

  auto ndom_solutions = filter_nondominated_points(solutions, false);

  auto set = TestType();
  for (auto const &s : solutions) {
    if (!set.empty() && mooutils::strictly_dominates(set, s)) {
      REQUIRE(set.insert(s) == set.end());
    } else {
      REQUIRE(*set.insert(s) == s);
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
    REQUIRE(set.insert(s) == set.end());
  }

  REQUIRE(set.size() == ndom_solutions.size());

  // it may change the order but not the items
  aux = std::vector(set.begin(), set.end());
  std::ranges::sort(aux, cmp);
  REQUIRE(std::ranges::equal(aux, ndom_solutions) == true);
}

// Equivalent solutions are unlikely to appear in the random case, so
// this is a test to force it.
TEMPLATE_LIST_TEST_CASE("multisets with equivalent solutions", "[sets][template]", multisets_types) {
  size_t n = GENERATE(size_t(10), 100, 1000);
  size_t m = GENERATE(size_t(2), 3, 5, 7);
  auto solutions = std::vector<solution_type>();
  solutions.reserve(n);
  // Consider a list of equivalent (but not equal) solutions.
  for (size_t i = 0; i < n; ++i) {
    solutions.emplace_back(dvec_type{i}, ovec_type(m, 0));
  }
  auto ndom_solutions = filter_nondominated_points(solutions, false);

  auto set = TestType();
  // Every solution should be added in the first pass
  for (auto const &s : solutions) {
    REQUIRE(*set.insert(s) == s);
  }

  REQUIRE(set.size() == ndom_solutions.size());

  auto cmp = [](auto const &lhs, auto const &rhs) {
    return lhs.decision_vector() < rhs.decision_vector();
  };

  auto aux = std::vector(set.begin(), set.end());
  std::ranges::sort(aux, cmp);
  std::ranges::sort(ndom_solutions, cmp);

  REQUIRE(std::ranges::equal(aux, ndom_solutions) == true);

  // A second pass should not add any solution
  for (auto const &s : solutions) {
    REQUIRE(set.insert(s) == set.end());
  }

  REQUIRE(set.size() == ndom_solutions.size());

  aux = std::vector(set.begin(), set.end());
  std::ranges::sort(aux, cmp);
  REQUIRE(std::ranges::equal(aux, ndom_solutions) == true);
}

using sets_types = std::tuple<mooutils::unordered_minimal_set<solution_type>,  // noformat
                              mooutils::flat_minimal_set<solution_type>,       // noformat
                              mooutils::minimal_set<solution_type>>;

TEMPLATE_LIST_TEST_CASE("sets with random solutions", "[sets][template]", sets_types) {
  std::random_device rd("/dev/urandom");
  std::mt19937 rng(rd());

  size_t n = GENERATE(10, 100, 1000);
  size_t m = GENERATE(2, 3, 5, 7);
  double p = GENERATE(0.3, 0.5, 0.7);

  auto points = generate_prob_nondominated_points<data_type>(n, m, p, rng);
  auto solutions = std::vector<solution_type>();
  solutions.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    solutions.emplace_back(dvec_type{i}, std::move(points[i]));
  }

  auto ndom_solutions = filter_nondominated_points(solutions, true);

  auto set = TestType();
  for (auto const &s : solutions) {
    if (!set.empty() && mooutils::weakly_dominates(set, s)) {
      REQUIRE(set.insert(s) == set.end());
    } else {
      REQUIRE(*set.insert(s) == s);
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
    REQUIRE(set.insert(s) == set.end());
  }

  REQUIRE(set.size() == ndom_solutions.size());

  // it may change the order but not the items
  aux = std::vector(set.begin(), set.end());
  std::ranges::sort(aux, cmp);
  REQUIRE(std::ranges::equal(aux, ndom_solutions) == true);
}

// Equivalent solutions are unlikely to appear in the random case, so
// this is a test to force it.
TEMPLATE_LIST_TEST_CASE("sets with equivalent solutions", "[sets][template]", sets_types) {
  size_t n = GENERATE(size_t(10), 100, 1000);
  size_t m = GENERATE(size_t(2), 3, 5, 7);
  auto solutions = std::vector<solution_type>();
  solutions.reserve(n);
  // Consider a list of equivalent (but not equal) solutions.
  for (size_t i = 0; i < n; ++i) {
    solutions.emplace_back(dvec_type{i}, ovec_type(m, 0));
  }
  auto ndom_solutions = filter_nondominated_points(solutions, true);

  auto set = TestType();
  // The first solution should be added
  REQUIRE(*set.insert(*solutions.begin()) == *solutions.begin());
  // The first and remaining solutions should not be added
  for (auto const &s : solutions) {
    REQUIRE(set.insert(s) == set.end());
  }

  REQUIRE(set.size() == 1);
  REQUIRE(std::ranges::equal(set, ndom_solutions) == true);
}
