#ifndef MOOUTILS_QUEUES_HPP_
#define MOOUTILS_QUEUES_HPP_

#include <cassert>
#include <deque>
#include <random>
#include <type_traits>
#include <vector>

namespace mooutils {

// CRTP Base class for queues based on an stl (or equivalent) container.
template <typename Derived, typename Container>
class base_queue {
 private:
  friend Derived;
  using container_type = Container;

 public:
  using value_type = typename container_type::value_type;
  using size_type = typename container_type::size_type;

  base_queue()
      : c() {}

  base_queue(base_queue const& other) = default;

  base_queue(base_queue&& other) = default;

  explicit base_queue(container_type const& cont)
      : c(cont) {}

  explicit base_queue(container_type&& cont)
      : c(std::move(cont)) {}

  template <typename InputIt>
  base_queue(InputIt first, InputIt last)
      : c(first, last) {}

  constexpr auto push(value_type const& value) {
    return static_cast<Derived&>(*this).push_impl(value);
  }

  constexpr auto push(value_type&& value) {
    return static_cast<Derived&>(*this).push_impl(std::move(value));
  }

  constexpr auto pop() -> value_type {
    assert(this->empty() == false);
    return static_cast<Derived&>(*this).pop_impl();
  }

  [[nodiscard]] constexpr auto empty() -> bool {
    return this->c.empty();
  }

  [[nodiscard]] constexpr auto size() -> size_type {
    return this->c.size();
  }

 private:
  container_type c;
};

template <typename Solution, typename Container = std::deque<Solution>>
class fifo_queue : public base_queue<fifo_queue<Solution, Container>, Container> {
 public:
  template <typename... Args>
  explicit fifo_queue(Args&&... args)
      : base_class_type(std::forward<Args>(args)...) {}

 private:
  using base_class_type = base_queue<fifo_queue<Solution, Container>, Container>;
  using typename base_class_type::value_type;

  friend base_class_type;

  template <typename S>
  constexpr auto push_impl(S&& s) {
    this->c.push_back(std::forward<S>(s));
  }

  constexpr auto pop_impl() -> value_type {
    auto res = std::move(this->c.front());
    this->c.pop_front();
    return res;
  }
};

template <typename Solution, typename Container = std::vector<Solution>>
class lifo_queue : public base_queue<lifo_queue<Solution, Container>, Container> {
 public:
  template <typename... Args>
  explicit lifo_queue(Args&&... args)
      : base_class_type(std::forward<Args>(args)...) {}

 private:
  using base_class_type = base_queue<lifo_queue<Solution, Container>, Container>;
  using typename base_class_type::value_type;

  friend base_class_type;

  template <typename S>
  constexpr auto push_impl(S&& s) {
    this->c.push_back(std::forward<S>(s));
  }

  constexpr auto pop_impl() -> value_type {
    auto res = std::move(this->c.back());
    this->c.pop_back();
    return res;
  }
};

template <typename Solution, typename Rng, typename Container = std::vector<Solution>>
class random_queue : public base_queue<random_queue<Solution, Rng, Container>, Container> {
 public:
  using rng_type = Rng;

  template <typename... Args>
  explicit random_queue(Args&&... args, Rng&& prng)
      : base_class_type(std::forward<Args>(args)...)
      , rng(std::move(prng)) {}

 private:
  using base_class_type = base_queue<random_queue<Solution, Rng, Container>, Container>;
  using typename base_class_type::size_type;
  using typename base_class_type::value_type;

  friend base_class_type;

  // Push a new solution to the queue
  template <typename S>
  constexpr auto push_impl(S&& s) {
    this->c.emplace_back(std::forward<S>(s));
  }

  constexpr auto pop_impl() -> value_type {
    auto size_minus_1 = this->c.size() - 1;
    auto runif = std::uniform_int_distribution<size_type>(0, size_minus_1);
    auto ind = runif(this->rng);
    auto ret = std::move(this->c[ind]);
    if (ind != size_minus_1) {
      this->c[ind] = std::move(this->c.back());
    }
    this->c.pop_back();
    return ret;
  }

  rng_type rng;
};

}  // namespace mooutils

#endif
