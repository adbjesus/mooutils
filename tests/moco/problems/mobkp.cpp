#include <catch2/catch.hpp>
#include <moco/problems/mobkp.hpp>
#include <sstream>

// TODO make a rapidcheck test or parametrize test
TEST_CASE("mobkp", "[problems][mobkp]") {
  using data_type = double;
  using problem_type = moco::problems::mobkp<data_type>;
  using size_type = typename problem_type::size_type;

  size_type ni = 100;
  size_type no = 3;
  size_type nc = 2;
  size_type n = nc + ni * (no + nc);

  std::stringstream aux;
  aux << ni << " " << no << " " << nc;
  for (size_type i = 0; i < n; ++i) {
    aux << " " << data_type(i);
  }

  auto p = problem_type(aux);
  auto const& pc = p;

  REQUIRE(pc.num_items() == ni);
  REQUIRE(pc.num_objectives() == no);
  REQUIRE(pc.num_constraints() == nc);

  for (size_type i = 0; i < nc; ++i) {
    REQUIRE(pc.constraint_rhs(i) == data_type(i));
    REQUIRE(*pc.constraint_rhs_it(i) == pc.constraint_rhs(i));
  }

  size_type val = nc;
  for (size_type i = 0; i < ni; ++i) {
    for (size_type j = 0; j < no; ++j) {
      REQUIRE(pc.item_value(i, j) == data_type(val++));
      REQUIRE(*pc.item_value_it(i, j) == pc.item_value(i, j));
    }
    for (size_type j = 0; j < nc; ++j) {
      REQUIRE(pc.item_weight(i, j) == data_type(val++));
      REQUIRE(*pc.item_weight_it(i, j) == pc.item_weight(i, j));
    }
  }
}
