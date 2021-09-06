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

template <typename Problem,   // noformat
          typename Solution,  // noformat
          typename SolutionQueue = moco::solution_queues::fifo<Solution>,
          typename SolutionSet =
              moco::solution_sets::multivector<std::reference_wrapper<const Solution>>>
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
    for (auto&& dvec : initial_dvecs) {
      solutions.insert(solution_type(std::move(dvec), p));
      unexplored.emplace_back(solutions.back());
    }
    while (!unexplored.empty()) {
      auto s = unexplored.pop();
      for (auto&& neighbor : solution.neighborhood(problem)) {
        auto it = solutions.insert(std::move(neighbor));
        if (it != solutions.end()) {
          unexplored.emplace_back(*it);
        }
      }
    }
    return solutions;
  }

 private:
  std::vector<dvec_type> initial_dvecs;
  queue_type unexplored;
  solution_set_type solutions;
};

}  // namespace moco::algorithms
