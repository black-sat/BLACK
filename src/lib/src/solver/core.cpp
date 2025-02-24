//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <black/solver/core.hpp>

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/solver/solver.hpp>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

namespace black_internal::core {

  struct K_data_t {
    size_t size;
    size_t n;
  };

  static size_t traverse_impl(
    formula f, tsl::hopscotch_map<formula, K_data_t> &ks, 
    size_t &next_placeholder
  ) {
    return f.match(
      [](boolean) -> size_t { return 1; },
      [&](proposition p) -> size_t {
        if(auto l = p.name().to<core_placeholder_t>(); l.has_value()) {
          if(l->n >= next_placeholder)
            next_placeholder = l->n + 1;
        }
        return 1;
      },
      [&](unary, formula arg) -> size_t {
        K_data_t data = ks.find(f) != ks.end() ? ks[f] : K_data_t{0, 0};
        if(data.size == 0)
          data.size = 1 + traverse_impl(arg, ks, next_placeholder);
        data.n += 1;
        ks[f] = data;

        return data.size;
      },
      [&](binary, formula left, formula right) -> size_t {
        K_data_t data = ks.find(f) != ks.end() ? ks[f] : K_data_t{0, 0};
        if(data.size == 0)
          data.size = 1 + traverse_impl(left, ks, next_placeholder) 
                        + traverse_impl(right, ks, next_placeholder);
        data.n += 1;
        ks[f] = data;

        return data.size;
      },
      // TODO: restrict types adequately
      [](otherwise) -> size_t { black_unreachable(); } // LCOV_EXCL_LINE
    );
  }

  static 
  std::pair<
    tsl::hopscotch_map<formula, K_data_t>,
    size_t
  >
  traverse(formula f) {
    tsl::hopscotch_map<formula, K_data_t> ks;
    size_t next_placeholder = 0;
    
    traverse_impl(f, ks, next_placeholder);
    
    return {ks, next_placeholder};
  }

  static
  std::vector<std::vector<formula>>
  group_by_K(formula f, tsl::hopscotch_map<formula, K_data_t> ks) {
    std::vector<std::vector<formula>> groups(ks[f].size + 1);

    for(auto [subf, k] : ks) {
      size_t K = k.size * k.n;
      black_assert(K < groups.size());
      groups[K].push_back(subf);
    }

    std::reverse(begin(groups), end(groups));

    return groups;
  }

  static size_t increment(std::vector<bool> &v) {
    size_t pos = 0;
    
    while(pos < v.size() && v[pos]) {
      v[pos] = false;
      ++pos;
    }
    if(pos < v.size())
      v[pos] = true;

    return pos;
  }

  static std::vector<std::vector<bool>> build_bitsets(size_t size) {
    std::vector<std::vector<bool>> result;
    
    std::vector<bool> v(size);
    while(increment(v) != size) {
      result.push_back(v);
    }

    std::sort(begin(result), end(result), [](auto const& a, auto const& b) {
      auto m = std::count_if(begin(a), end(a), [](bool x) { return x; });
      auto n = std::count_if(begin(b), end(b), [](bool x) { return x; });

      return m > n;
    });

    return result;
  }

  static 
  formula replace_impl(
    formula f, tsl::hopscotch_set<formula> const& dontcares,
    tsl::hopscotch_map<formula, size_t> &indexes,
    size_t &next_index
  ) {
    using namespace std::literals;

    if(dontcares.find(f) != dontcares.end()) {
      size_t index = 
        indexes.find(f) != indexes.end() ? indexes[f] : next_index++;
      indexes.insert_or_assign(f, index);

      return f.sigma()->proposition(core_placeholder_t{index, f});
    }
    
    return f.match(
      [&](boolean) { return f; },
      [&](proposition) { return f; },
      [&](unary u, formula arg) {
        return unary(
          u.node_type(), replace_impl(arg, dontcares, indexes, next_index)
        );
      },
      [&](binary b, formula left, formula right) {
        return binary(
          b.node_type(), 
          replace_impl(left, dontcares, indexes, next_index), 
          replace_impl(right, dontcares, indexes, next_index)
        );
      },
      [](otherwise) -> formula { black_unreachable(); } // LCOV_EXCL_LINE
    );
  }

  static
  tsl::hopscotch_set<formula> build_subset(
    std::vector<formula> const& universe, std::vector<bool> const&bitset
  ) {
    black_assert(universe.size() == bitset.size());
    tsl::hopscotch_set<formula> subset;
    for(size_t i = 0; i < bitset.size(); ++i) {
      if(bitset[i])
        subset.insert(universe[i]);
    }

    return subset;
  }

  static 
  formula replace(
    formula f, tsl::hopscotch_set<formula> const&dontcares,
    size_t next_index
  ) {
    tsl::hopscotch_map<formula, size_t> indexes;
    
    return replace_impl(f, dontcares, indexes, next_index);
  }

  static
  bool check_replacements_impl(
    formula f, tsl::hopscotch_map<size_t, formula> &formulas
  ) {
    return f.match(
      [](boolean) { return true; },
      [&](proposition p) {
        if(auto l = p.name().to<core_placeholder_t>(); l.has_value()) {
          if(formulas.find(l->n) != formulas.end() && formulas.at(l->n) != l->f)
            return false; // LCOV_EXCL_LINE
          formulas.insert({l->n, l->f});
        }
        return true;
      },
      [&](unary, formula arg) {
        return check_replacements_impl(arg, formulas);
      },
      [&](binary, formula left, formula right) {
        return check_replacements_impl(left, formulas) && 
               check_replacements_impl(right, formulas);
      },
      [](otherwise) -> bool { black_unreachable(); } // LCOV_EXCL_LINE
    );
  }

  [[maybe_unused]]
  static 
  bool check_replacements(formula f) {
    tsl::hopscotch_map<size_t, formula> formulas;
    return check_replacements_impl(f, formulas);
  }

  formula unsat_core(scope const& xi, formula f, bool finite) {
    auto [ks, next_placeholder] = traverse(f);
    auto groups = group_by_K(f, ks);

    for(auto &group : groups) {
      auto bitsets = build_bitsets(group.size());

      for(auto &bitset : bitsets) 
      { 
        auto subset = build_subset(group, bitset);
        formula candidate = replace(f, subset, next_placeholder);

        black::solver slv;
        
        if(slv.solve(xi, candidate, finite) == false)
          return unsat_core(xi, candidate, finite);
      }      
    }
    black_assert(check_replacements(f));
    return f;
  }

}
