#include <catch2/catch.hpp>

#include <mooutils/indicators.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

// In the following tests, we use a small data_type and larger
// result_type. This is to test the case where the result does not fit
// in the data_type.
//
// If the domain for the reference points and set points is in the range
// [-10, 90] the difference between the coordinates of a point and those
// of the reference point is in the range [1,100]. Moreover, the
// hypervolume of a point or set will be at most 100^m, which fits in a
// 64 bit integer for `m <= 9`. The following min and max values are set
// to assure that not overflows are possible if the result_type is
// correctly used.
using data_type = int16_t;
using result_type = int64_t;
constexpr data_type min_p = 1;
constexpr data_type max_p = 90;
constexpr data_type min_r = -10;
constexpr data_type max_r = 0;
constexpr size_t min_m = 2;
constexpr size_t max_m = 9;

TEST_CASE("point hv", "[indicators][hv]") {
  // Number of dimensions
  auto m = GENERATE(range(min_m, max_m));

  // Point
  auto p = GENERATE_COPY(chunk(m, take(m + 99, random(min_p, max_p))));

  // Reference point
  auto r = GENERATE_COPY(std::vector(m, max_r),  // noformat
                         std::vector(m, min_r),  // noformat
                         chunk(m, take(m + 10, random(min_r, max_r))));

  auto expected = result_type{1};
  for (size_t i = 0; i < m; ++i) {
    expected *= result_type{p[i]} - result_type{r[i]};
  }

  REQUIRE(mooutils::hv<result_type>(p, r) == expected);
}

template <typename T, typename Rng>
auto generate_nondominated_points(size_t n, size_t m, Rng&& rng, T low, T high) {
  std::normal_distribution<double> rnorm(0.0, 1.0);
  std::vector<std::vector<T>> pointset;
  for (size_t i = 0; i < n; i++) {
    std::vector<double> point;
    double sum = 0.0;
    for (size_t j = 0; j < m; j++) {
      point.push_back(rnorm(rng));
      sum += point[j] * point[j];
    }
    std::ranges::for_each(point, [&sum](auto& c) { c = std::abs(c / sqrt(sum)); });
    std::vector<T> aux;
    for (auto& c : point) {
      aux.emplace_back(c * (high - low) + low);
    }
    pointset.push_back(std::move(aux));
  }
  return pointset;
}

TEST_CASE("set hv 2d properties", "[indicators][hv]") {
  // Dimension
  size_t m = 2;

  // Number of points
  auto n = GENERATE(size_t(2), 5, 10, 100, 1000);

  // Reference point
  auto r = GENERATE_COPY(std::vector(m, max_r),  // noformat
                         std::vector(m, min_r),  // noformat
                         chunk(m, take(m + 10, random(min_r, max_r))));

  // Seed
  using rng_type = std::mt19937_64;
  using rng_result_type = typename rng_type::result_type;
  auto min_seed = std::numeric_limits<rng_result_type>::min();
  auto max_seed = std::numeric_limits<rng_result_type>::max();
  auto seed = GENERATE_COPY(take(10, random(min_seed, max_seed)));

  auto const set = generate_nondominated_points<data_type, rng_type>(n, m, rng_type(seed), min_p, max_p);

  auto aux = set;
  std::ranges::sort(aux, mooutils::lexicographically_greater_fn{});
  auto const sorted_set = std::move(aux);

  // Non const equals same as const
  aux = set;
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hv<result_type>(aux, r));
  aux = set;
  REQUIRE(mooutils::hv2d<result_type>(set, r) == mooutils::hv2d<result_type>(aux, r));
  aux = sorted_set;
  REQUIRE(mooutils::hv2d<result_type>(sorted_set, r, true) == mooutils::hv2d<result_type>(aux, r, true));

  // hv equals hv2d
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hv2d<result_type>(set, r));

  // hv equals hv2d
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hv2d<result_type>(sorted_set, r, true));
}

TEST_CASE("set hv 3d properties", "[indicators][hv]") {
  // Dimension
  size_t m = 3;

  // Number of points
  auto n = GENERATE(size_t(2), 5, 10, 100, 1000);

  // Reference point
  auto r = GENERATE_COPY(std::vector(m, max_r),  // noformat
                         std::vector(m, min_r),  // noformat
                         chunk(m, take(m + 10, random(min_r, max_r))));

  // Seed
  using rng_type = std::mt19937_64;
  using rng_result_type = typename rng_type::result_type;
  auto min_seed = std::numeric_limits<rng_result_type>::min();
  auto max_seed = std::numeric_limits<rng_result_type>::max();
  auto seed = GENERATE_COPY(take(10, random(min_seed, max_seed)));

  auto const set = generate_nondominated_points<data_type, rng_type>(n, m, rng_type(seed), min_p, max_p);

  auto aux = set;
  std::ranges::sort(aux, mooutils::lexicographically_greater_fn{});
  auto const sorted_set = std::move(aux);

  // Non const equals same as const
  aux = set;
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hv<result_type>(aux, r));
  aux = set;
  REQUIRE(mooutils::hv3d<result_type>(set, r) == mooutils::hv3d<result_type>(aux, r));
  aux = sorted_set;
  REQUIRE(mooutils::hv3d<result_type>(sorted_set, r, true) == mooutils::hv3d<result_type>(aux, r, true));

  // hv equals hv3d
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hv3d<result_type>(set, r));

  // hv equals hv3d
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hv3d<result_type>(sorted_set, r, true));
}

TEST_CASE("set hv wfg properties", "[indicators][hv]") {
  // Dimension
  auto m = GENERATE(range(min_m, max_m));

  // Number of points
  auto n = GENERATE(size_t(2), 5, 10, 50);

  // Reference point
  auto r = GENERATE_COPY(std::vector(m, max_r),  // noformat
                         std::vector(m, min_r),  // noformat
                         chunk(m, take(m + 5, random(min_r, max_r))));

  // Seed
  using rng_type = std::mt19937_64;
  using rng_result_type = typename rng_type::result_type;
  auto min_seed = std::numeric_limits<rng_result_type>::min();
  auto max_seed = std::numeric_limits<rng_result_type>::max();
  auto seed = GENERATE_COPY(take(5, random(min_seed, max_seed)));

  auto const set = generate_nondominated_points<data_type, rng_type>(n, m, rng_type(seed), min_p, max_p);

  auto aux = set;
  std::ranges::sort(aux, mooutils::lexicographically_greater_fn{});
  auto const sorted_set = std::move(aux);

  // Non const equals same as const
  aux = set;
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hv<result_type>(aux, r));
  aux = set;
  REQUIRE(mooutils::hvwfg<result_type>(set, r) == mooutils::hvwfg<result_type>(aux, r));
  aux = sorted_set;
  REQUIRE(mooutils::hvwfg<result_type>(sorted_set, r, true) == mooutils::hvwfg<result_type>(aux, r, true));

  // hv equals hvwfg
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hvwfg<result_type>(set, r));

  // hv equals hvwfg
  REQUIRE(mooutils::hv<result_type>(set, r) == mooutils::hvwfg<result_type>(sorted_set, r, true));
}

struct HypervolumeDataset {
  using ovec_type = std::vector<data_type>;
  using set_type = std::vector<ovec_type>;

  size_t n;         // Number of points in the front
  size_t m;         // Dimensions
  ovec_type refp;   // Reference point
  set_type points;  // Points
  result_type hv;   // Correct hypervolume value

  explicit HypervolumeDataset(std::filesystem::path const& path) {
    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
      throw("Can't open file");
    }

    std::ifstream is(path);
    is >> n >> m >> hv;
    std::copy_n(std::istream_iterator<data_type>(is), m, std::back_inserter(refp));

    for (size_t i = 0; i < n; ++i) {
      ovec_type aux;
      std::copy_n(std::istream_iterator<data_type>(is), m, std::back_inserter(aux));
      points.emplace_back(std::move(aux));
    }
  }
};

template <typename HVStruct, typename HVData>
[[nodiscard]] constexpr auto test_hv_struct(HVStruct&& hvs, HVData const& hvdata) -> bool {
  auto sum_contributions = result_type{0};
  auto sum_inserts = result_type{0};
  for (auto const& point : hvdata.points) {
    sum_contributions += hvs.contribution(point);
    sum_inserts += hvs.insert(point);
  }
  if (sum_contributions != hvdata.hv) {
    return false;
  } else if (sum_inserts != hvdata.hv) {
    return false;
  } else if (hvs.value() != hvdata.hv) {
    return false;
  } else {
    return true;
  }
}

// Note, the test cases in the TEST_RESOURCES_DIR should be such that
// the individual values of the points fit in a 16-bit signed integer,
// and that the hypervolume fits in a 64-bit signed integer.
TEST_CASE("set hv results", "[indicators][hv]") {
  auto datadir = std::filesystem::path(TEST_RESOURCES_DIR) / "hvdata" / "data";

  if (!std::filesystem::exists(datadir) || !std::filesystem::is_directory(datadir)) {
    throw("Can't read directory");
  }

  for (auto const& p : std::filesystem::directory_iterator(datadir)) {
    auto hvdata = HypervolumeDataset(p);
    // Base hv function
    REQUIRE(mooutils::hv<result_type>(hvdata.points, hvdata.refp) == hvdata.hv);
    // Dynamic hv structs
    auto hvs = mooutils::hvwfg_container<result_type>(hvdata.refp);
    REQUIRE(test_hv_struct(hvs, hvdata) == true);
    if (hvdata.m == 2) {
      auto hv2d = mooutils::hv2d_container<result_type>(hvdata.refp);
      REQUIRE(test_hv_struct(hv2d, hvdata) == true);
    }
    if (hvdata.m == 3) {
      auto hv3d = mooutils::hv3dplus_container<result_type>(hvdata.refp);
      REQUIRE(test_hv_struct(hv3d, hvdata) == true);
    }
  }
}
