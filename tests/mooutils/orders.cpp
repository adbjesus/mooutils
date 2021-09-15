#include <catch2/catch.hpp>

#include <mooutils/orders.hpp>

// TODO improve sets/vectors generation

constexpr size_t min_m{2};
constexpr size_t max_m{10};  // exclusive
constexpr size_t repeats{50};
constexpr int min_v{0};
constexpr int max_v{100};

TEST_CASE("equivalent vectors", "[orders][dominance]") {
  auto m = GENERATE(range(min_m, max_m));
  auto v1 = GENERATE_COPY(chunk(m, take(m + repeats, random(min_v, max_v))));
  auto v2 = v1;

  REQUIRE(mooutils::equivalent(v1, v2) == true);
  REQUIRE(mooutils::weakly_dominates(v1, v2) == true);
  REQUIRE(mooutils::dominates(v1, v2) == false);
  REQUIRE(mooutils::strictly_dominates(v1, v2) == false);
  REQUIRE(mooutils::incomparable(v1, v2) == false);

  REQUIRE(mooutils::equivalent(v2, v1) == true);
  REQUIRE(mooutils::weakly_dominates(v2, v1) == true);
  REQUIRE(mooutils::dominates(v2, v1) == false);
  REQUIRE(mooutils::strictly_dominates(v2, v1) == false);
  REQUIRE(mooutils::incomparable(v2, v1) == false);
}

TEST_CASE("vector1 dominates vector2", "[orders][dominance]") {
  auto m = GENERATE(range(min_m, max_m));
  auto v1 = GENERATE_COPY(chunk(m, take(m + repeats, random(min_v, max_v))));
  auto ind = GENERATE_COPY(range(size_t(0), m));
  auto v2 = v1;
  v2[ind] -= 1;

  REQUIRE(mooutils::equivalent(v1, v2) == false);
  REQUIRE(mooutils::weakly_dominates(v1, v2) == true);
  REQUIRE(mooutils::dominates(v1, v2) == true);
  REQUIRE(mooutils::strictly_dominates(v1, v2) == false);
  REQUIRE(mooutils::incomparable(v1, v2) == false);

  REQUIRE(mooutils::equivalent(v2, v1) == false);
  REQUIRE(mooutils::weakly_dominates(v2, v1) == false);
  REQUIRE(mooutils::dominates(v2, v1) == false);
  REQUIRE(mooutils::strictly_dominates(v2, v1) == false);
  REQUIRE(mooutils::incomparable(v2, v1) == false);
}

TEST_CASE("vector1 strictly dominates vector2", "[orders][dominance]") {
  auto m = GENERATE(range(min_m, max_m));
  auto v1 = GENERATE_COPY(chunk(m, take(m + repeats, random(min_v, max_v))));
  auto v2 = v1;
  std::for_each(v2.begin(), v2.end(), [](auto &v) { return v = v - 1; });

  REQUIRE(mooutils::equivalent(v1, v2) == false);
  REQUIRE(mooutils::weakly_dominates(v1, v2) == true);
  REQUIRE(mooutils::dominates(v1, v2) == true);
  REQUIRE(mooutils::strictly_dominates(v1, v2) == true);
  REQUIRE(mooutils::incomparable(v1, v2) == false);

  REQUIRE(mooutils::equivalent(v2, v1) == false);
  REQUIRE(mooutils::weakly_dominates(v2, v1) == false);
  REQUIRE(mooutils::dominates(v2, v1) == false);
  REQUIRE(mooutils::strictly_dominates(v2, v1) == false);
  REQUIRE(mooutils::incomparable(v2, v1) == false);
}

TEST_CASE("incomparable vectors", "[orders][dominance]") {
  auto m = GENERATE(range(min_m, max_m));
  auto v1 = GENERATE_COPY(chunk(m, take(m + repeats, random(min_v, max_v))));
  auto ind1 = GENERATE_COPY(range(size_t(0), m));
  auto ind2 = GENERATE_COPY(range(size_t(0), m));

  if (ind1 != ind2) {
    auto v2 = v1;
    v2[ind1] += 1;
    v2[ind2] -= 1;

    REQUIRE(mooutils::equivalent(v1, v2) == false);
    REQUIRE(mooutils::weakly_dominates(v1, v2) == false);
    REQUIRE(mooutils::dominates(v1, v2) == false);
    REQUIRE(mooutils::strictly_dominates(v1, v2) == false);
    REQUIRE(mooutils::incomparable(v1, v2) == true);

    REQUIRE(mooutils::equivalent(v2, v1) == false);
    REQUIRE(mooutils::weakly_dominates(v2, v1) == false);
    REQUIRE(mooutils::dominates(v2, v1) == false);
    REQUIRE(mooutils::strictly_dominates(v2, v1) == false);
    REQUIRE(mooutils::incomparable(v2, v1) == true);
  }
}

TEST_CASE("equivalent vector and set", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto v = GENERATE_COPY(chunk(m, take(m + repeats, random(min_v, max_v))));

  auto s = std::vector<decltype(v)>(n, v);

  REQUIRE(mooutils::equivalent(v, s) == true);
  REQUIRE(mooutils::weakly_dominates(v, s) == true);
  REQUIRE(mooutils::dominates(v, s) == false);
  REQUIRE(mooutils::strictly_dominates(v, s) == false);
  REQUIRE(mooutils::incomparable(v, s) == false);

  REQUIRE(mooutils::equivalent(s, v) == true);
  REQUIRE(mooutils::weakly_dominates(s, v) == true);
  REQUIRE(mooutils::dominates(s, v) == false);
  REQUIRE(mooutils::strictly_dominates(s, v) == false);
  REQUIRE(mooutils::incomparable(s, v) == false);
}

TEST_CASE("vector (strictly) dominates set", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 5, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto v = GENERATE_COPY(chunk(m, take(m + repeats, random(min_v, max_v))));
  auto inds = GENERATE_COPY(chunk(n, take(n, random(size_t(0), m - 1))));

  auto s = std::vector<decltype(v)>();
  s.reserve(n);
  for (auto ind : inds) {
    auto aux = v;
    aux[ind] -= 1;
    s.push_back(std::move(aux));
  }

  REQUIRE(mooutils::equivalent(v, s) == false);
  REQUIRE(mooutils::weakly_dominates(v, s) == true);
  REQUIRE(mooutils::dominates(v, s) == true);
  REQUIRE(mooutils::strictly_dominates(v, s) == true);
  REQUIRE(mooutils::incomparable(v, s) == false);

  REQUIRE(mooutils::equivalent(s, v) == false);
  REQUIRE(mooutils::weakly_dominates(s, v) == false);
  REQUIRE(mooutils::dominates(s, v) == false);
  REQUIRE(mooutils::strictly_dominates(s, v) == false);
  REQUIRE(mooutils::incomparable(s, v) == false);
}

TEST_CASE("set dominates vector", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 5, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto v = GENERATE_COPY(chunk(m, take(m + repeats, random(min_v, max_v))));
  auto s = std::vector<decltype(v)>(n, v);
  auto ind1 = GENERATE_COPY(take(1, random(size_t(0), m - 2)));
  auto ind2 = GENERATE_COPY(take(1, random(ind1 + 1, m - 1)));
  auto aux = v;
  aux[ind1] -= 1;
  aux[ind2] += 1;
  s.push_back(std::move(aux));

  REQUIRE(mooutils::equivalent(v, s) == false);
  REQUIRE(mooutils::weakly_dominates(v, s) == false);
  REQUIRE(mooutils::dominates(v, s) == false);
  REQUIRE(mooutils::strictly_dominates(v, s) == false);
  REQUIRE(mooutils::incomparable(v, s) == false);

  REQUIRE(mooutils::equivalent(s, v) == false);
  REQUIRE(mooutils::weakly_dominates(s, v) == true);
  REQUIRE(mooutils::dominates(s, v) == true);
  REQUIRE(mooutils::strictly_dominates(s, v) == false);
  REQUIRE(mooutils::incomparable(s, v) == false);
}

TEST_CASE("set strictly dominates vector", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 5, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto v = GENERATE_COPY(chunk(m, take(m + repeats, random(min_v, max_v))));
  auto s = std::vector<decltype(v)>(n, v);
  auto ind = GENERATE_COPY(take(1, random(size_t(0), m - 1)));
  auto aux = v;
  aux[ind] += 1;
  s.push_back(std::move(aux));

  REQUIRE(mooutils::equivalent(v, s) == false);
  REQUIRE(mooutils::weakly_dominates(v, s) == false);
  REQUIRE(mooutils::dominates(v, s) == false);
  REQUIRE(mooutils::strictly_dominates(v, s) == false);
  REQUIRE(mooutils::incomparable(v, s) == false);

  REQUIRE(mooutils::equivalent(s, v) == false);
  REQUIRE(mooutils::weakly_dominates(s, v) == true);
  REQUIRE(mooutils::dominates(s, v) == true);
  REQUIRE(mooutils::strictly_dominates(s, v) == true);
  REQUIRE(mooutils::incomparable(s, v) == false);
}

TEST_CASE("incomparable vector and set", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 5, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto aux = GENERATE_COPY(chunk(m * n, take(m * n + repeats, random(min_v, max_v))));
  auto s = std::vector<decltype(aux)>();
  auto first = aux.begin();
  for (size_t i = 0; i < n; ++i) {
    auto next = first + static_cast<int>(m);
    s.emplace_back(first, next);
    first = next;
  }
  auto v = GENERATE_COPY(chunk(m, take(m, random(min_v, max_v))));
  v[0] = min_v - 1;
  v[m - 1] = max_v + 1;

  REQUIRE(mooutils::equivalent(v, s) == false);
  REQUIRE(mooutils::weakly_dominates(v, s) == false);
  REQUIRE(mooutils::dominates(v, s) == false);
  REQUIRE(mooutils::strictly_dominates(v, s) == false);
  REQUIRE(mooutils::incomparable(v, s) == true);

  REQUIRE(mooutils::equivalent(s, v) == false);
  REQUIRE(mooutils::weakly_dominates(s, v) == false);
  REQUIRE(mooutils::dominates(s, v) == false);
  REQUIRE(mooutils::strictly_dominates(s, v) == false);
  REQUIRE(mooutils::incomparable(s, v) == true);
}

TEST_CASE("equivalent sets", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto aux = GENERATE_COPY(chunk(m * n, take(m * n + repeats, random(min_v, max_v))));
  auto s1 = std::vector<decltype(aux)>();
  auto first = aux.begin();
  for (size_t i = 0; i < n; ++i) {
    auto next = first + static_cast<int>(m);
    s1.emplace_back(first, next);
    first = next;
  }
  auto s2 = s1;

  REQUIRE(mooutils::equivalent(s1, s2) == true);
  REQUIRE(mooutils::weakly_dominates(s1, s2) == true);
  REQUIRE(mooutils::dominates(s1, s2) == false);
  REQUIRE(mooutils::strictly_dominates(s1, s2) == false);
  REQUIRE(mooutils::incomparable(s1, s2) == false);

  REQUIRE(mooutils::equivalent(s2, s1) == true);
  REQUIRE(mooutils::weakly_dominates(s2, s1) == true);
  REQUIRE(mooutils::dominates(s2, s1) == false);
  REQUIRE(mooutils::strictly_dominates(s2, s1) == false);
  REQUIRE(mooutils::incomparable(s2, s1) == false);
}

TEST_CASE("set1 dominates set2", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto aux = GENERATE_COPY(chunk(m * n, take(m * n + repeats, random(min_v, max_v))));
  auto s1 = std::vector<decltype(aux)>();
  auto first = aux.begin();
  for (size_t i = 0; i < n; ++i) {
    auto next = first + static_cast<int>(m);
    s1.emplace_back(first, next);
    first = next;
  }
  auto s2 = s1;

  auto v1 = GENERATE_COPY(chunk(m, take(m, random(min_v, max_v))));
  v1[0] = min_v - 1;
  v1[m - 1] = max_v + 1;
  s1.push_back(std::move(v1));

  REQUIRE(mooutils::equivalent(s1, s2) == false);
  REQUIRE(mooutils::weakly_dominates(s1, s2) == true);
  REQUIRE(mooutils::dominates(s1, s2) == true);
  REQUIRE(mooutils::strictly_dominates(s1, s2) == false);
  REQUIRE(mooutils::incomparable(s1, s2) == false);

  REQUIRE(mooutils::equivalent(s2, s1) == false);
  REQUIRE(mooutils::weakly_dominates(s2, s1) == false);
  REQUIRE(mooutils::dominates(s2, s1) == false);
  REQUIRE(mooutils::strictly_dominates(s2, s1) == false);
  REQUIRE(mooutils::incomparable(s2, s1) == false);
}

TEST_CASE("set1 strictly dominates set2", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto aux = GENERATE_COPY(chunk(m * n, take(m * n + repeats, random(min_v, max_v))));
  auto inds = GENERATE_COPY(chunk(n, take(n, random(size_t(0), m - 1))));

  auto s1 = std::vector<decltype(aux)>();
  auto first = aux.begin();
  for (size_t i = 0; i < n; ++i) {
    auto next = first + static_cast<int>(m);
    s1.emplace_back(first, next);
    first = next;
  }

  auto s2 = s1;
  for (size_t i = 0; i < n; ++i) {
    s1[i][inds[i]] += 1;
  }

  CAPTURE(s1, s2);

  REQUIRE(mooutils::equivalent(s1, s2) == false);
  REQUIRE(mooutils::weakly_dominates(s1, s2) == true);
  REQUIRE(mooutils::dominates(s1, s2) == true);
  REQUIRE(mooutils::strictly_dominates(s1, s2) == true);
  REQUIRE(mooutils::incomparable(s1, s2) == false);

  REQUIRE(mooutils::equivalent(s2, s1) == false);
  REQUIRE(mooutils::weakly_dominates(s2, s1) == false);
  REQUIRE(mooutils::dominates(s2, s1) == false);
  REQUIRE(mooutils::strictly_dominates(s2, s1) == false);
  REQUIRE(mooutils::incomparable(s2, s1) == false);
}

TEST_CASE("incomparable sets", "[orders][dominance]") {
  auto n = GENERATE(size_t(1), 2, 5, 10);
  auto m = GENERATE(range(min_m, max_m));
  auto aux1 = GENERATE_COPY(chunk(m * n, take(m * n + repeats, random(min_v, max_v))));
  auto s1 = std::vector<decltype(aux1)>();
  auto first = aux1.begin();
  for (size_t i = 0; i < n; ++i) {
    auto next = first + static_cast<int>(m);
    s1.emplace_back(first, next);
    first = next;
  }
  auto s2 = s1;

  auto v1 = GENERATE_COPY(chunk(m, take(m, random(min_v, max_v))));
  v1[0] = min_v - 1;
  v1[m - 1] = max_v + 1;
  auto v2 = GENERATE_COPY(chunk(m, take(m, random(min_v, max_v))));
  v2[0] = max_v + 1;
  v2[m - 1] = min_v - 1;

  s1.push_back(std::move(v1));
  s2.push_back(std::move(v2));

  REQUIRE(mooutils::equivalent(s1, s2) == false);
  REQUIRE(mooutils::weakly_dominates(s1, s2) == false);
  REQUIRE(mooutils::dominates(s1, s2) == false);
  REQUIRE(mooutils::strictly_dominates(s1, s2) == false);
  REQUIRE(mooutils::incomparable(s1, s2) == true);

  REQUIRE(mooutils::equivalent(s2, s1) == false);
  REQUIRE(mooutils::weakly_dominates(s2, s1) == false);
  REQUIRE(mooutils::dominates(s1, s2) == false);
  REQUIRE(mooutils::strictly_dominates(s1, s2) == false);
  REQUIRE(mooutils::incomparable(s1, s2) == true);
}
