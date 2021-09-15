#include <catch2/catch.hpp>

#include <mooutils/solution.hpp>

#include <array>
#include <span>
#include <vector>

TEST_CASE("unconstrained solution", "[solutions][classes]") {
  using dvec_type = std::vector<bool>;
  using ovec_type = std::array<int, 2>;

  auto dvec = dvec_type{true, false, true};
  auto ovec = ovec_type{0, 1};

  auto sol = mooutils::unconstrained_solution<dvec_type, ovec_type>(dvec, ovec);
  REQUIRE(sol.decision_vector() == dvec);
  REQUIRE(sol.objective_vector() == ovec);

  auto sol2 = std::move(sol);
  REQUIRE(sol2.decision_vector() == dvec);
  REQUIRE(sol2.objective_vector() == ovec);
}

TEST_CASE("constrained solution", "[solutions][classes]") {
  using dvec_type = std::vector<bool>;
  using ovec_type = std::array<int, 2>;
  using cvec_type = std::vector<int>;

  auto dvec = dvec_type{true, false, true};
  auto ovec = ovec_type{0, 1};
  auto cvec = cvec_type{2};

  auto sol = mooutils::constrained_solution<dvec_type, ovec_type, cvec_type>(dvec, ovec, cvec);
  REQUIRE(sol.decision_vector() == dvec);
  REQUIRE(sol.objective_vector() == ovec);
  REQUIRE(sol.constraint_vector() == cvec);

  auto sol2 = std::move(sol);
  REQUIRE(sol2.decision_vector() == dvec);
  REQUIRE(sol2.objective_vector() == ovec);
  REQUIRE(sol2.constraint_vector() == cvec);
}

TEST_CASE("decision vector view", "[solutions][view]") {
  using dvec_type = std::vector<bool>;
  auto dvec = dvec_type{true, false, true};

  auto& dvec_lvalue_ref = mooutils::decision_vector(dvec);
  REQUIRE(std::addressof(dvec) == std::addressof(dvec_lvalue_ref));

  auto const& dvec_const_ref = mooutils::decision_vector(dvec);
  REQUIRE(std::addressof(dvec) == std::addressof(dvec_const_ref));

  auto&& dvec_rvalue_ref = mooutils::decision_vector(std::move(dvec));
  REQUIRE(std::addressof(dvec) == std::addressof(dvec_rvalue_ref));

  auto dvec2 = mooutils::base_decision_vector<dvec_type>(std::move(dvec));

  auto& dvec2_lvalue_ref = mooutils::decision_vector(dvec2);
  REQUIRE(std::addressof(dvec2.decision_vector()) == std::addressof(dvec2_lvalue_ref));

  auto const& dvec2_const_ref = mooutils::decision_vector(dvec2);
  REQUIRE(std::addressof(dvec2.decision_vector()) == std::addressof(dvec2_const_ref));

  auto&& dvec2_rvalue_ref = mooutils::decision_vector(std::move(dvec2));
  REQUIRE(std::addressof(dvec2.decision_vector()) == std::addressof(dvec2_rvalue_ref));
}

TEST_CASE("objective vector view", "[solutions][view]") {
  using ovec_type_aux = std::vector<int>;
  using ovec_type = std::span<int>;
  auto ovec_aux = ovec_type_aux{1, 2, 3};
  auto ovec = ovec_type{ovec_aux};

  auto& ovec_lvalue_ref = mooutils::objective_vector(ovec);
  REQUIRE(std::addressof(ovec) == std::addressof(ovec_lvalue_ref));

  auto const& ovec_const_ref = mooutils::objective_vector(ovec);
  REQUIRE(std::addressof(ovec) == std::addressof(ovec_const_ref));

  auto&& ovec_rvalue_ref = mooutils::objective_vector(std::move(ovec));
  REQUIRE(std::addressof(ovec) == std::addressof(ovec_rvalue_ref));

  auto ovec2 = mooutils::base_objective_vector<ovec_type>(std::move(ovec));

  auto& ovec2_lvalue_ref = mooutils::objective_vector(ovec2);
  REQUIRE(std::addressof(ovec2.objective_vector()) == std::addressof(ovec2_lvalue_ref));

  auto const& ovec2_const_ref = mooutils::objective_vector(ovec2);
  REQUIRE(std::addressof(ovec2.objective_vector()) == std::addressof(ovec2_const_ref));

  auto&& ovec2_rvalue_ref = mooutils::objective_vector(std::move(ovec2));
  REQUIRE(std::addressof(ovec2.objective_vector()) == std::addressof(ovec2_rvalue_ref));
}

TEST_CASE("constraint vector view", "[solutions][view]") {
  using cvec_type = std::array<int, 2>;
  auto cvec = cvec_type{4, 5};

  auto& cvec_lvalue_ref = mooutils::constraint_vector(cvec);
  REQUIRE(std::addressof(cvec) == std::addressof(cvec_lvalue_ref));

  auto const& cvec_const_ref = mooutils::constraint_vector(cvec);
  REQUIRE(std::addressof(cvec) == std::addressof(cvec_const_ref));

  auto&& cvec_rvalue_ref = mooutils::constraint_vector(std::move(cvec));
  REQUIRE(std::addressof(cvec) == std::addressof(cvec_rvalue_ref));

  auto cvec2 = mooutils::base_constraint_vector<cvec_type>(std::move(cvec));

  auto& cvec2_lvalue_ref = mooutils::constraint_vector(cvec2);
  REQUIRE(std::addressof(cvec2.constraint_vector()) == std::addressof(cvec2_lvalue_ref));

  auto const& cvec2_const_ref = mooutils::constraint_vector(cvec2);
  REQUIRE(std::addressof(cvec2.constraint_vector()) == std::addressof(cvec2_const_ref));

  auto&& cvec2_rvalue_ref = mooutils::constraint_vector(std::move(cvec2));
  REQUIRE(std::addressof(cvec2.constraint_vector()) == std::addressof(cvec2_rvalue_ref));
}

TEST_CASE("decision vectors view", "[solutions][view]") {
  using dvec_type = std::array<int, 2>;
  auto dvec1 = dvec_type{1, 2};
  auto dvec2 = dvec_type{3, 4};
  auto dvec3 = dvec_type{5, 6};
  auto set = std::vector<dvec_type>{dvec1, dvec2, dvec3};

  auto view = mooutils::decision_vectors(set);
  REQUIRE(std::addressof(*view.begin()) == std::addressof(*set.begin()));
  REQUIRE(std::addressof(*(view.begin() + 1)) == std::addressof(*(set.begin() + 1)));
  REQUIRE(std::addressof(*(view.begin() + 2)) == std::addressof(*(set.begin() + 2)));

  auto const cset = std::vector<dvec_type>{dvec1, dvec2, dvec3};
  auto cview = mooutils::decision_vectors(cset);
  REQUIRE(std::addressof(*cview.begin()) == std::addressof(*cset.begin()));
  REQUIRE(std::addressof(*(cview.begin() + 1)) == std::addressof(*(cset.begin() + 1)));
  REQUIRE(std::addressof(*(cview.begin() + 2)) == std::addressof(*(cset.begin() + 2)));
}

TEST_CASE("objective vectors view", "[solutions][view]") {
  using ovec_type = std::array<int, 2>;
  auto ovec1 = ovec_type{1, 2};
  auto ovec2 = ovec_type{3, 4};
  auto ovec3 = ovec_type{5, 6};
  auto set = std::vector<ovec_type>{ovec1, ovec2, ovec3};

  auto view = mooutils::objective_vectors(set);
  REQUIRE(std::addressof(*view.begin()) == std::addressof(*set.begin()));
  REQUIRE(std::addressof(*(view.begin() + 1)) == std::addressof(*(set.begin() + 1)));
  REQUIRE(std::addressof(*(view.begin() + 2)) == std::addressof(*(set.begin() + 2)));

  auto const cset = std::vector<ovec_type>{ovec1, ovec2, ovec3};
  auto cview = mooutils::objective_vectors(cset);
  REQUIRE(std::addressof(*cview.begin()) == std::addressof(*cset.begin()));
  REQUIRE(std::addressof(*(cview.begin() + 1)) == std::addressof(*(cset.begin() + 1)));
  REQUIRE(std::addressof(*(cview.begin() + 2)) == std::addressof(*(cset.begin() + 2)));
}

TEST_CASE("constraint vectors view", "[solutions][view]") {
  using cvec_type = std::array<int, 2>;
  auto cvec1 = cvec_type{1, 2};
  auto cvec2 = cvec_type{3, 4};
  auto cvec3 = cvec_type{5, 6};
  auto set = std::vector<cvec_type>{cvec1, cvec2, cvec3};

  auto view = mooutils::constraint_vectors(set);
  REQUIRE(std::addressof(*view.begin()) == std::addressof(*set.begin()));
  REQUIRE(std::addressof(*(view.begin() + 1)) == std::addressof(*(set.begin() + 1)));
  REQUIRE(std::addressof(*(view.begin() + 2)) == std::addressof(*(set.begin() + 2)));

  auto const cset = std::vector<cvec_type>{cvec1, cvec2, cvec3};
  auto cview = mooutils::constraint_vectors(cset);
  REQUIRE(std::addressof(*cview.begin()) == std::addressof(*cset.begin()));
  REQUIRE(std::addressof(*(cview.begin() + 1)) == std::addressof(*(cset.begin() + 1)));
  REQUIRE(std::addressof(*(cview.begin() + 2)) == std::addressof(*(cset.begin() + 2)));
}

TEST_CASE("solutions vectors view", "[solutions][view]") {
  using dvec_type = std::vector<bool>;
  using ovec_type = std::array<int, 2>;
  using cvec_type = std::vector<int>;

  auto dvec1 = dvec_type{true, false, true};
  auto ovec1 = ovec_type{0, 1};
  auto cvec1 = cvec_type{2};
  auto dvec2 = dvec_type{false, true, false};
  auto ovec2 = ovec_type{3, 4};
  auto cvec2 = cvec_type{5};

  using solution_type = mooutils::constrained_solution<dvec_type, ovec_type, cvec_type>;
  auto sol1 = solution_type(dvec1, ovec1, cvec1);
  auto sol2 = solution_type(dvec2, ovec2, cvec2);

  auto set = std::vector<solution_type>{sol1, sol2};

  auto dview = mooutils::decision_vectors(set);
  auto oview = mooutils::objective_vectors(set);
  auto cview = mooutils::constraint_vectors(set);

  REQUIRE(std::addressof(*dview.begin()) == std::addressof(set.begin()->decision_vector()));
  REQUIRE(std::addressof(*oview.begin()) == std::addressof(set.begin()->objective_vector()));
  REQUIRE(std::addressof(*cview.begin()) == std::addressof(set.begin()->constraint_vector()));

  REQUIRE(std::addressof(*(dview.begin() + 1)) == std::addressof((set.begin() + 1)->decision_vector()));
  REQUIRE(std::addressof(*(oview.begin() + 1)) == std::addressof((set.begin() + 1)->objective_vector()));
  REQUIRE(std::addressof(*(cview.begin() + 1)) == std::addressof((set.begin() + 1)->constraint_vector()));
}
