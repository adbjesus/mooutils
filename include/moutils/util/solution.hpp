#pragma once

#include <moutils/util/concepts.hpp>

#include <ranges>
#include <utility>

namespace moutils {

template <moutils::decision_vector DVec, moutils::objective_vector OVec,
          moutils::constraint_vector CVec>
class solution {
 public:
  using decision_vector_type = DVec;
  using objective_vector_type = OVec;
  using constraint_vector_type = CVec;

  // template <typename Problem>
  // solution(decision_vector_type&& decision_vector, Problem const& problem)
  //     : m_decision_vector(std::move(decision_vector))
  //     , m_objective_vector(problem.eval_objectives(m_decision_vector))
  //     , m_constraint_vector(problem.eval_constraints(m_decision_vector))
  //     , m_feasible(problem.feasible(m_constraint_vector)) {}

  solution(decision_vector_type&& decision_vector, objective_vector_type&& objective_vector,
           constraint_vector_type&& constraint_vector, bool feasible)
      : m_decision_vector(std::move(decision_vector))
      , m_objective_vector(std::move(objective_vector))
      , m_constraint_vector(std::move(constraint_vector))
      , m_feasible(feasible) {}

  auto decision_vector() -> decision_vector_type& {
    return m_decision_vector;
  }

  auto objective_vector() -> objective_vector_type& {
    return m_objective_vector;
  }

  auto constraint_vector() -> constraint_vector_type& {
    return m_constraint_vector;
  }

  auto decision_vector() const -> decision_vector_type const& {
    return m_decision_vector;
  }

  auto objective_vector() const -> objective_vector_type const& {
    return m_objective_vector;
  }

  auto constraint_vector() const -> constraint_vector_type const& {
    return m_constraint_vector;
  }

  auto feasible() const {
    return m_feasible;
  }

  auto operator==(solution const& other) const {
    return m_decision_vector == other.m_decision_vector;
  }

 protected:
  decision_vector_type m_decision_vector;
  objective_vector_type m_objective_vector;
  constraint_vector_type m_constraint_vector;
  bool m_feasible;
};

template <typename T>
requires moutils::decision_vector<T>
auto get_decision_vector(T const& t) -> T const& {
  return t;
}

template <typename T>
requires moutils::has_decision_vector<T>
auto get_decision_vector(T const& t) -> decltype(t.decision_vector()) {
  return t.decision_vector();
}

template <typename T>
requires moutils::objective_vector<T>
auto get_objective_vector(T const& t) -> T const& {
  return t;
}

template <typename T>
requires moutils::has_objective_vector<T>
auto get_objective_vector(T const& t) -> decltype(t.objective_vector()) {
  return t.objective_vector();
}

template <typename T>
requires moutils::constraint_vector<T>
auto get_constraint_vector(T const& t) -> T const& {
  return t;
}

template <typename T>
requires moutils::has_constraint_vector<T>
auto get_constraint_vector(T const& t) -> decltype(t.constraint_vector()) {
  return t.constraint_vector();
}

// Takes a range of solutions and returns a view over the decision vectors
template <moutils::solution_set T>
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

}  // namespace moutils
