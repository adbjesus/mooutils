#include <moco/problems/mobkp.hpp>
#include <moco/util/dominance.hpp>
#include <moco/util/indicators/hypervolume.hpp>
#include <moco/util/solution.hpp>
#include <moco/util/solution_queues.hpp>
#include <moco/util/solution_sets.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <array>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  auto is = std::ifstream(argv[1]);
  auto timeout = std::stod(argv[2]);

  using data_type = double;

  auto const problem = moco::problems::mobkp<data_type>(is);
  auto const n = problem.num_items();
  auto const m = problem.num_objectives();

  if (m != 2) {
    throw("Invalid number of objectives!");
  }

  auto start = std::chrono::high_resolution_clock::now();

  auto refp = std::array<data_type, 2>{0.0, 0.0};
  auto hv = moco::indicators::hypervolume<data_type, 2>(refp);
  auto iter = 0;

  auto elapsed = [&start]() {
    return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
  };

  auto register_time_and_hv = [&iter, &elapsed, &hv]() {
    fmt::print("{},{},{}\n", iter, elapsed(), hv.value());
  };

  using dvec_type = std::vector<bool>;
  using ovec_type = std::array<data_type, 2>;
  using cvec_type = std::array<data_type, 1>;
  using solution_type = moco::solution<dvec_type, ovec_type, cvec_type>;

  auto solutions = moco::solution_sets::multivector<solution_type>();

  using rng_type = std::mt19937_64;
  auto rng = rng_type(2);  // std::random_device()());
  auto unexplored = moco::solution_queues::random<solution_type, rng_type>(std::move(rng));

  auto initial_dvec = dvec_type(n, false);
  auto initial_ovec = ovec_type{0.0, 0.0};
  auto initial_cvec = cvec_type{0.0};

  auto initial = solution_type(std::move(initial_dvec),  // noformat
                               std::move(initial_ovec),  // noformat
                               std::move(initial_cvec),  // noformat
                               true);

  auto it = solutions.insert_unchecked(std::move(initial));
  unexplored.push(*it);

  while (elapsed() < timeout && !unexplored.empty()) {
    auto s = unexplored.pop();
    auto const& dvec = s.decision_vector();
    auto const& ovec = s.objective_vector();
    auto const& cvec = s.constraint_vector();

    if (strictly_dominates(solutions, s)) {
      continue;
    }

    iter += 1;

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

          hv.insert(it->objective_vector());
          register_time_and_hv();
        }
      }
    }

    if (flip) {
      continue;
    }

    // 1-swap neighborhood
    for (size_t i = 0; i < n; ++i) {
      for (size_t j = i + 1; j < n; ++j) {
        // Check if constraint is not broken.
        if (dvec[i] == dvec[j]) {
          continue;
        }

        auto new_cvec = cvec;
        if (dvec[i]) {
          new_cvec[0] -= problem.item_weight(i, 0);
          new_cvec[0] += problem.item_weight(j, 0);
        } else {
          new_cvec[0] += problem.item_weight(i, 0);
          new_cvec[0] -= problem.item_weight(j, 0);
        }
        if (new_cvec[0] > problem.constraint_rhs(0)) {
          continue;
        }

        auto new_ovec = ovec;
        auto it = problem.item_value_it(i, 0);
        auto jt = problem.item_value_it(j, 0);
        if (dvec[i]) {
          for (size_t k = 0; k < m; ++k, ++it, ++jt) {
            new_ovec[k] -= *it;
            new_ovec[k] += *jt;
          }
        } else {
          for (size_t k = 0; k < m; ++k, ++it, ++jt) {
            new_ovec[k] += *it;
            new_ovec[k] -= *jt;
          }
        }

        if (!moco::strictly_dominates(solutions, new_ovec)) {
          auto new_dvec = dvec;
          new_dvec[i] = !dvec[i];
          new_dvec[j] = !dvec[j];
          auto it = solutions.insert(solution_type(std::move(new_dvec),  // noformat
                                                   std::move(new_ovec),  // noformat
                                                   std::move(new_cvec),  // noformat
                                                   true));
          if (it != solutions.end()) {
            unexplored.push(*it);

            hv.insert(it->objective_vector());
            register_time_and_hv();
          }
        }
      }
    }
  }

  // for (auto const& s : solutions) {
  //   fmt::print("{} ", fmt::join(s.objective_vector(), " "));
  //   fmt::print("{} ", fmt::join(s.constraint_vector(), " "));
  //   fmt::print("{}\n", fmt::join(s.decision_vector(), " "));
  // }

  return EXIT_SUCCESS;
}
