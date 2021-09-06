#include <moco/problems/mobkp.hpp>
#include <moco/util/dominance.hpp>
#include <moco/util/solution.hpp>
#include <moco/util/solution_queues.hpp>
#include <moco/util/solution_sets.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <array>
#include <fstream>
#include <vector>

int main(int argc, char** argv) {
  auto is = std::ifstream(argv[1]);
  auto const problem = moco::problems::mobkp<double>(is);
  auto const n = problem.num_items();
  auto const m = problem.num_objectives();

  using dvec_type = std::vector<bool>;
  using ovec_type = std::vector<double>;
  using cvec_type = std::array<double, 1>;
  using solution_type = moco::solution<dvec_type, ovec_type, cvec_type>;

  auto solutions = moco::solution_sets::multivector<solution_type>();
  auto unexplored = moco::solution_queues::fifo<solution_type>();

  auto initial_dvec = dvec_type(n, false);
  auto initial_ovec = ovec_type(m, 0.0);
  auto initial_cvec = cvec_type{0.0};

  auto initial = solution_type(std::move(initial_dvec), std::move(initial_ovec),
                               std::move(initial_cvec), true);

  auto it = solutions.insert_unchecked(std::move(initial));
  unexplored.push(*it);

  while (!unexplored.empty()) {
    fmt::print("{}\n", unexplored.size());
    auto s = unexplored.pop();
    auto const& dvec = s.decision_vector();
    auto const& ovec = s.objective_vector();
    auto const& cvec = s.constraint_vector();

    // 1-flip neighborhood
    bool flip = false;
    for (size_t i = 0; i < n; ++i) {
      // Check if constraint is not broken.
      if (dvec[i] == true) {
        continue;
      }

      if (cvec[0] + problem.item_weight(i, 0) > problem.constraint_rhs(0)) {
        continue;
      }

      auto new_ovec = ovec;
      auto it = problem.item_value_it(i, 0);
      for (size_t j = 0; j < m; ++j, ++it) {
        new_ovec[j] += *it;
      }

      if (!moco::strictly_dominates(solutions, new_ovec)) {
        auto new_dvec = dvec;
        auto new_cvec = cvec;
        new_cvec[0] += problem.item_weight(i, 0);
        new_dvec[i] = true;
        auto it = solutions.insert(solution_type(std::move(new_dvec),  // noformat
                                                 std::move(new_ovec),  // noformat
                                                 std::move(new_cvec),  // noformat
                                                 true));
        if (it != solutions.end()) {
          flip = true;
          unexplored.push(*it);
        }
      }
    }

    // 1-swap neighborhood
  }

  for (auto const& s : solutions) {
    fmt::print("{} ", fmt::join(s.objective_vector(), " "));
    fmt::print("{} ", fmt::join(s.constraint_vector(), " "));
    fmt::print("{}\n", fmt::join(s.decision_vector(), " "));
  }

  return EXIT_SUCCESS;
}
