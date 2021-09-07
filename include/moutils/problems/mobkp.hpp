#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <istream>
#include <iterator>
#include <utility>
#include <vector>

namespace moutils::problems {

template <typename T, typename C = std::vector<T>>
class mobkp {
 public:
  using data_type = T;
  using container_type = C;
  using size_type = typename C::size_type;

  mobkp(size_t ni, size_t no, size_t nc, data_type&& data)
      : m_ni(ni)
      , m_no(no)
      , m_nc(nc)
      , m_data(std::move(data)) {}

  // Assumes data is in the form (whitespace irrelevant):
  //   n
  //   no
  //   nc
  //   W_1 ... W_nc
  //   v^1_1 ... v^no_1 w^1_1 ... w^nc_1
  //   ...
  //   v^1_n ... v^no_n w^1_n ... w^nc_n
  //
  // TODO protect this functions against ioerrors
  explicit mobkp(std::istream& is) {
    is >> m_ni >> m_no >> m_nc;

    auto n = m_nc + m_ni * (m_nc + m_no);

    m_data.reserve(n);

    // Read data to array
    std::copy_n(std::istream_iterator<data_type>(is), n, std::back_inserter(m_data));
  }

  constexpr auto num_items() const noexcept {
    return m_ni;
  }

  constexpr auto num_objectives() const noexcept {
    return m_no;
  }

  constexpr auto num_constraints() const noexcept {
    return m_nc;
  }

  // Get an iterator to value $j$ of item $i$.
  [[nodiscard]] constexpr auto item_value_it(size_type i, size_type j) const {
    assert(i >= 0 && i < m_ni);
    assert(j >= 0 && j < m_no);
    // skip the first nc constraints: nc
    // skip the first (i-1) items: i * (nc + no);
    // return the value: j
    return m_data.begin() + (m_nc + i * (m_nc + m_no) + j);
  }

  // Get value $j$ for item $i$.
  [[nodiscard]] constexpr auto item_value(size_type i, size_type j) const {
    return *item_value_it(i, j);
  }

  // Get an iterator to constraint $j$ of item $i$.
  [[nodiscard]] constexpr auto item_weight_it(size_type i, size_type j) const {
    assert(i >= 0 && i < m_ni);
    assert(j >= 0 && j < m_nc);
    // skip the first nc constraints: nc
    // skip the first (i-1) items: i * (nc + no)
    // skip the values of the $i$th item: no
    // return the constraint: j
    return m_data.begin() + (m_nc + i * (m_no + m_nc) + m_no + j);
  }

  // Get constraint $j$ of item $i$.
  [[nodiscard]] constexpr auto item_weight(size_type i, size_type j) const {
    return *item_weight_it(i, j);
  }

  // Get an iterator to the right hand side of constraint $i$.
  [[nodiscard]] constexpr auto constraint_rhs_it(size_type i) const {
    assert(i >= 0 && i < m_nc);
    return m_data.begin() + i;
  }

  // Get the value of right hand side of constraint $i$.
  [[nodiscard]] constexpr auto constraint_rhs(size_type i) const {
    return *constraint_rhs_it(i);
  }

 private:
  size_type m_ni;         // number of items
  size_type m_no;         // number of values per item
  size_type m_nc;         // number of constraints per item
  container_type m_data;  // data container (constraints, and items)
};

}  // namespace moutils::problems
