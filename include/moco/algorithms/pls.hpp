#pragma once

#include <moco/util/solution.hpp>
#include <moco/util/solution_queues.hpp>
#include <moco/util/solution_sets.hpp>

#include <deque>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

namespace moco::algorithms {

template <typename Problem,  // noformat
          typename Solution = moco::util::solution<Problem>,
          typename SolutionQueue = moco::sq_fifo<Solution>,
          typename SolutionSet = moco::ss_unordered_vec<Solution>>
class pls {
 public:
  using problem_type = Problem;
  using solution_type = Solution;
  using queue_type = Queue;
  using dvec_type = typename problem_type::dvec_type;

  template <typename D>
  auto add_initial_dvec(D&& dvec) {
    initial_dvecs.emplace_back(std::forward<D>(dvec));
  }

  template <typename S>
  auto add_initial_solution(S&& solution) {
    solutions.insert(std::forward<S>(solution));
    unexplored.emplace_back(solutions.back());
  }

  auto solve(problem_type const& p) {
    // Evaluate all initial dvecs
    for (auto&& dvec : initial_dvecs) {
      solutions.emplace(std::move(dvec), p);
      unexplored.emplace_back(solutions.back());
    }
    // PLS loop
    while (!unexplored.empty()) {
      // choose a solution from unexplored
      // loop over neighborhood
    }
  }

 private:
  std::vector<dvec_type> initial_dvecs;
  queue_type unexplored;
  solution_set_type solutions;
};

}  // namespace moco::algorithms
