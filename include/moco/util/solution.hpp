#pragma once

#include <moco/util/concepts.hpp>

#include <ranges>
#include <utility>

namespace moco::util {

template <typename Problem>
class solution {
 public:
  using problem_type = Problem;
  using decision_vector_type = typename problem_type::decision_vector_type;
  using objective_vector_type = typename problem_type::objective_vector_type;
  using constraint_vector_type = typename problem_type::constraint_vector_type;

  solution(decision_vector_type&& decision_vector, problem_type const& problem)
      : m_decision_vector(std::move(decision_vector))
      , m_objective_vector(problem.eval_objectives(m_decision_vector))
      , m_constraint_vector(problem.eval_objectives(m_decision_vector))
      , m_feasible(problem.feasible(m_constraint_vector)) {}

  solution(decision_vector_type&& decision_vector, objective_vector_type&& objective_vector,
           constraint_vector_type&& constraint_vector, bool feasible)
      : m_decision_vector(std::move(decision_vector))
      , m_objective_vector(std::move(objective_vector))
      , m_constraint_vector(std::move(constraint_vector))
      , m_feasible(feasible) {}

  auto decision_vector() const -> decision_vector_type const& {
    return decision_vector;
  }

  auto objective_vector() const -> objective_vector_type const& {
    return objective_vector;
  }

  auto constraint_vector() const -> constraint_vector_type const& {
    return constraint_vector;
  }

  auto feasible() const {
    return m_feasible;
  }

 private:
  decision_vector_type m_decision_vector;
  objective_vector_type m_objective_vector;
  constraint_vector_type m_constraint_vector;
  bool m_feasible;
};

// Takes a range of solutions and returns a view over the decision vectors
// TODO use concept instead of typename
template <moco::solution_set T>
constexpr auto decision_vectors(T const& t) {
  return std::views::transform(
      t, [](auto const& s) -> decltype(s.decision_vector()) const& { return s.decision_vector(); });
}

// Takes a range of solutions and returns a view over the objective vectors
// TODO use concept instead of typename
template <typename T>
constexpr auto objective_vectors(T const& t) {
  return std::views::transform(t, [](auto const& s) -> decltype(s.objective_vector()) const& {
    return s.objective_vector();
  });
}

// Takes a range of solutions and returns a view over the constraint vectors
// TODO use concept instead of typename
template <typename T>
constexpr auto contraint_vectors(T const& t) {
  return std::views::transform(t, [](auto const& s) -> decltype(s.contraint_vector()) const& {
    return s.constraint_vector();
  });
}

}  // namespace moco::util
