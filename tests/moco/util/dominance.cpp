#include <catch2/catch.hpp>

#include <moco/util/dominance.hpp>

// TODO rapidcheck (or similar) for random/property test cases

TEST_CASE("dominance for equivalent vectors", "[util][dominance]") {
  auto v1 = std::vector<int>{1, 2, 3, 4};
  auto v2 = v1;

  REQUIRE(moco::equivalent(v1, v2) == true);
  REQUIRE(moco::weakly_dominates(v1, v2) == true);
  REQUIRE(moco::dominates(v1, v2) == false);
  REQUIRE(moco::strictly_dominates(v1, v2) == false);
  REQUIRE(moco::incomparable(v1, v2) == false);

  REQUIRE(moco::equivalent(v2, v1) == true);
  REQUIRE(moco::weakly_dominates(v2, v1) == true);
  REQUIRE(moco::dominates(v2, v1) == false);
  REQUIRE(moco::strictly_dominates(v2, v1) == false);
  REQUIRE(moco::incomparable(v2, v1) == false);
}

TEST_CASE("dominance for dominant vector", "[dominance]") {
  // difference on the first element
  auto v1 = std::vector<int>{1, 2, 3, 4};
  auto v2 = std::vector<int>{0, 2, 3, 4};

  REQUIRE(moco::equivalent(v1, v2) == false);
  REQUIRE(moco::weakly_dominates(v1, v2) == true);
  REQUIRE(moco::dominates(v1, v2) == true);
  REQUIRE(moco::strictly_dominates(v1, v2) == false);
  REQUIRE(moco::incomparable(v1, v2) == false);

  REQUIRE(moco::equivalent(v2, v1) == false);
  REQUIRE(moco::weakly_dominates(v2, v1) == false);
  REQUIRE(moco::dominates(v2, v1) == false);
  REQUIRE(moco::strictly_dominates(v2, v1) == false);
  REQUIRE(moco::incomparable(v2, v1) == false);

  // difference on the last element
  v1 = std::vector<int>{1, 2, 3, 4};
  v2 = std::vector<int>{1, 2, 3, 3};

  REQUIRE(moco::equivalent(v1, v2) == false);
  REQUIRE(moco::weakly_dominates(v1, v2) == true);
  REQUIRE(moco::dominates(v1, v2) == true);
  REQUIRE(moco::strictly_dominates(v1, v2) == false);
  REQUIRE(moco::incomparable(v1, v2) == false);

  REQUIRE(moco::equivalent(v2, v1) == false);
  REQUIRE(moco::weakly_dominates(v2, v1) == false);
  REQUIRE(moco::dominates(v2, v1) == false);
  REQUIRE(moco::strictly_dominates(v2, v1) == false);
  REQUIRE(moco::incomparable(v2, v1) == false);
}

TEST_CASE("dominance for strictly dominant vector", "[dominance]") {
  // difference on the first element
  auto v1 = std::vector<int>{1, 2, 3, 4};
  auto v2 = std::vector<int>{0, 1, 2, 3};

  REQUIRE(moco::equivalent(v1, v2) == false);
  REQUIRE(moco::weakly_dominates(v1, v2) == true);
  REQUIRE(moco::dominates(v1, v2) == true);
  REQUIRE(moco::strictly_dominates(v1, v2) == true);
  REQUIRE(moco::incomparable(v1, v2) == false);

  REQUIRE(moco::equivalent(v2, v1) == false);
  REQUIRE(moco::weakly_dominates(v2, v1) == false);
  REQUIRE(moco::dominates(v2, v1) == false);
  REQUIRE(moco::strictly_dominates(v2, v1) == false);
  REQUIRE(moco::incomparable(v2, v1) == false);
}

TEST_CASE("dominance for incomparable vectors", "[dominance]") {
  // difference on the first element
  auto v1 = std::vector<int>{1, 2};
  auto v2 = std::vector<int>{0, 3};

  REQUIRE(moco::equivalent(v1, v2) == false);
  REQUIRE(moco::weakly_dominates(v1, v2) == false);
  REQUIRE(moco::dominates(v1, v2) == false);
  REQUIRE(moco::strictly_dominates(v1, v2) == false);
  REQUIRE(moco::incomparable(v1, v2) == true);

  REQUIRE(moco::equivalent(v2, v1) == false);
  REQUIRE(moco::weakly_dominates(v2, v1) == false);
  REQUIRE(moco::dominates(v2, v1) == false);
  REQUIRE(moco::strictly_dominates(v2, v1) == false);
  REQUIRE(moco::incomparable(v2, v1) == true);
}

TEST_CASE("mutually weakly dominated vector and set", "[dominance]") {
  auto v = std::vector<int>{1, 2, 3, 4};
  auto s = std::vector<std::vector<int>>{v, v, v};

  REQUIRE(moco::weakly_dominates(v, s) == true);
  REQUIRE(moco::dominates(v, s) == false);
  REQUIRE(moco::strictly_dominates(v, s) == false);

  REQUIRE(moco::weakly_dominates(s, v) == true);
  REQUIRE(moco::dominates(s, v) == false);
  REQUIRE(moco::strictly_dominates(s, v) == false);
}

TEST_CASE("vector dominates set", "[dominance]") {
  auto v = std::vector<int>{1, 2, 3, 4};
  auto s = std::vector<std::vector<int>>{v, v, std::vector<int>{1, 2, 3, 3}};

  REQUIRE(moco::weakly_dominates(v, s) == true);
  REQUIRE(moco::dominates(v, s) == true);
  REQUIRE(moco::strictly_dominates(v, s) == false);

  REQUIRE(moco::weakly_dominates(s, v) == true);
  REQUIRE(moco::dominates(s, v) == false);
  REQUIRE(moco::strictly_dominates(s, v) == false);
}

TEST_CASE("vector strictly dominates set", "[dominance]") {
  auto v = std::vector<int>{1, 2, 3, 4};
  auto s1 = std::vector<int>{0, 2, 3, 4};
  auto s2 = std::vector<int>{1, 1, 3, 4};
  auto s3 = std::vector<int>{1, 2, 2, 4};
  auto s4 = std::vector<int>{1, 2, 3, 3};
  auto s = std::vector<std::vector<int>>{s1, s2, s3, s4};

  REQUIRE(moco::weakly_dominates(v, s) == true);
  REQUIRE(moco::dominates(v, s) == true);
  REQUIRE(moco::strictly_dominates(v, s) == true);

  REQUIRE(moco::weakly_dominates(s, v) == false);
  REQUIRE(moco::dominates(s, v) == false);
  REQUIRE(moco::strictly_dominates(s, v) == false);
}

TEST_CASE("set dominates vector", "[dominance]") {
  auto v = std::vector<int>{1, 2, 3, 4};
  auto s = std::vector<std::vector<int>>{v, v, std::vector<int>{0, 2, 3, 5}};

  REQUIRE(moco::weakly_dominates(v, s) == false);
  REQUIRE(moco::dominates(v, s) == false);
  REQUIRE(moco::strictly_dominates(v, s) == false);

  REQUIRE(moco::weakly_dominates(s, v) == true);
  REQUIRE(moco::dominates(s, v) == true);
  REQUIRE(moco::strictly_dominates(s, v) == false);
}

TEST_CASE("set strictly dominates vector", "[dominance]") {
  auto v = std::vector<int>{1, 2, 3, 4};
  auto s1 = std::vector<int>{2, 2, 3, 4};
  auto s2 = std::vector<int>{1, 3, 3, 4};
  auto s3 = std::vector<int>{1, 2, 4, 4};
  auto s4 = std::vector<int>{1, 2, 3, 5};
  auto s5 = std::vector<int>{0, 2, 3, 6};
  auto s = std::vector<std::vector<int>>{s1, s2, s3, s4};

  REQUIRE(moco::weakly_dominates(v, s) == false);
  REQUIRE(moco::dominates(v, s) == false);
  REQUIRE(moco::strictly_dominates(v, s) == false);

  REQUIRE(moco::weakly_dominates(s, v) == true);
  REQUIRE(moco::dominates(s, v) == true);
  REQUIRE(moco::strictly_dominates(s, v) == true);
}
