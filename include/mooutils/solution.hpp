#pragma once

#include <mooutils/concepts.hpp>

#include <vector>

namespace mooutils {

template <is_decision_vector T>
class base_decision_vector {
 public:
  using decision_vector_type = T;

  template <typename... Args>
  explicit base_decision_vector(Args&&... args)
      : dvec(std::forward<Args>(args)...) {}

  [[nodiscard]] constexpr auto decision_vector() -> decision_vector_type& {
    return dvec;
  }

  [[nodiscard]] constexpr auto decision_vector() const -> decision_vector_type const& {
    return dvec;
  }

 protected:
  decision_vector_type dvec;
};

template <is_objective_vector T>
class base_objective_vector {
 public:
  using objective_vector_type = T;

  template <typename... Args>
  explicit base_objective_vector(Args&&... args)
      : ovec(std::forward<Args>(args)...) {}

  [[nodiscard]] constexpr auto objective_vector() -> objective_vector_type& {
    return this->ovec;
  }

  [[nodiscard]] constexpr auto objective_vector() const -> objective_vector_type const& {
    return this->ovec;
  }

 protected:
  objective_vector_type ovec;
};

template <is_constraint_vector T>
class base_constraint_vector {
 public:
  using constraint_vector_type = T;

  template <typename... Args>
  explicit base_constraint_vector(Args&&... args)
      : cvec(std::forward<Args>(args)...) {}

  [[nodiscard]] constexpr auto constraint_vector() -> constraint_vector_type& {
    return this->cvec;
  }

  [[nodiscard]] constexpr auto constraint_vector() const -> constraint_vector_type const& {
    return this->cvec;
  }

 protected:
  constraint_vector_type cvec;
};

template <typename DVec, typename OVec>
class unconstrained_solution : public base_decision_vector<DVec>, public base_objective_vector<OVec> {
 private:
  using base_decision_vector_type = base_decision_vector<DVec>;
  using base_objective_vector_type = base_objective_vector<OVec>;

 public:
  template <typename DV, typename OV>
  unconstrained_solution(DV&& dv, OV&& ov)
      : base_decision_vector_type(std::forward<DV>(dv))
      , base_objective_vector_type(std::forward<OV>(ov)) {}

  auto operator==(unconstrained_solution const& other) const {
    return std::equal(this->dvec.begin(), this->dvec.end(), other.dvec.begin(), other.dvec.end());
  }
};

template <typename DVec, typename OVec, typename CVec>
class constrained_solution : public base_decision_vector<DVec>,
                             public base_objective_vector<OVec>,
                             public base_constraint_vector<CVec> {
 private:
  using base_decision_vector_type = base_decision_vector<DVec>;
  using base_objective_vector_type = base_objective_vector<OVec>;
  using base_constraint_vector_type = base_constraint_vector<CVec>;

 public:
  template <typename DV, typename OV, typename CV>
  constrained_solution(DV&& dv, OV&& ov, CV&& cv)
      : base_decision_vector_type(std::forward<DV>(dv))
      , base_objective_vector_type(std::forward<OV>(ov))
      , base_constraint_vector_type(std::forward<CV>(cv)) {}

  auto operator==(constrained_solution const& other) const {
    return std::equal(this->dvec.begin(), this->dvec.end(), other.dvec.begin(), other.dvec.end());
  }
};

// The following are views to get a particular vector of a solution.
struct decision_vector_view {
  template <is_decision_vector T>
  [[nodiscard]] constexpr auto operator()(T&& t) const -> T&& {
    return std::forward<T>(t);
  }

  template <has_decision_vector T>
  [[nodiscard]] constexpr auto operator()(T&& t) const -> decltype(t.decision_vector()) {
    return t.decision_vector();
  }
};

inline constexpr decision_vector_view decision_vector;

struct objective_vector_view {
  template <is_objective_vector T>
  [[nodiscard]] constexpr auto operator()(T&& t) const -> T&& {
    return std::forward<T>(t);
  }

  template <has_objective_vector T>
  [[nodiscard]] constexpr auto operator()(T&& t) const -> decltype(t.objective_vector()) {
    return t.objective_vector();
  }
};

inline constexpr objective_vector_view objective_vector;

struct constraint_vector_view {
  template <is_constraint_vector T>
  [[nodiscard]] constexpr auto operator()(T&& t) const -> T&& {
    return std::forward<T>(t);
  }

  template <has_constraint_vector T>
  [[nodiscard]] constexpr auto operator()(T&& t) const -> decltype(t.constraint_vector()) {
    return std::forward<T>(t).constraint_vector();
  }
};

inline constexpr constraint_vector_view constraint_vector;

// The following are views to get the vectors over a ranges. They take
// the range of solutions, and a projection to get the corresponding
// vector, which defaults to the ones previously described.
struct decision_vectors_view {
  template <typename Range, typename DVecView = decision_vector_view>
  [[nodiscard]] constexpr auto operator()(Range&& r, DVecView&& dvec_view = {}) const {
    return std::ranges::transform_view(std::forward<Range>(r), std::forward<DVecView>(dvec_view));
  }
};

inline constexpr decision_vectors_view decision_vectors;

struct objective_vectors_view {
  template <typename Range, typename OVecView = objective_vector_view>
  [[nodiscard]] constexpr auto operator()(Range&& r, OVecView&& ovec_view = {}) const {
    return std::ranges::transform_view(std::forward<Range>(r), std::forward<OVecView>(ovec_view));
  }
};

inline constexpr objective_vectors_view objective_vectors;

struct constraint_vectors_view {
  template <typename Range, typename CVecView = constraint_vector_view>
  [[nodiscard]] constexpr auto operator()(Range&& r, CVecView&& cvec_view = {}) const {
    return std::ranges::transform_view(std::forward<Range>(r), std::forward<CVecView>(cvec_view));
  }
};

inline constexpr constraint_vectors_view constraint_vectors;

}  // namespace mooutils
