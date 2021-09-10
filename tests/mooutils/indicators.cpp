#include <catch2/catch.hpp>

#include <mooutils/indicators.hpp>

TEST_CASE("point hv", "[indicators][hv]") {
  using data_type = int64_t;

  // Number of dimensions
  auto m = GENERATE(range<size_t>(2, 10));

  // Point
  auto p = GENERATE_COPY(chunk(m, take(m + 99, random<data_type>(1, 100))));

  // Reference point
  auto r = GENERATE_COPY(std::vector<data_type>(m, 0),   // noformat
                         std::vector<data_type>(m, -1),  // noformat
                         chunk(m, take(m, random<data_type>(-10, 0))));

  data_type expected = 1;
  for (size_t i = 0; i < m; ++i) {
    expected *= p[i] - r[i];
  }

  REQUIRE(mooutils::indicators::point_hv(p, r) == expected);
}

TEST_CASE("point hv", "[indicators][hv]") {
  using data_type = int64_t;

  // Number of dimensions
  auto m = GENERATE(range<size_t>(2, 10));

  // Point
  auto p = GENERATE_COPY(chunk(m, take(m + 99, random<data_type>(1, 100))));

  // Reference point
  auto r = GENERATE_COPY(std::vector<data_type>(m, 0),   // noformat
                         std::vector<data_type>(m, -1),  // noformat
                         chunk(m, take(m, random<data_type>(-10, 0))));

  data_type expected = 1;
  for (size_t i = 0; i < m; ++i) {
    expected *= p[i] - r[i];
  }

  REQUIRE(mooutils::indicators::point_hv(p, r) == expected);
}
