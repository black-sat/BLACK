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

#ifndef BLACK_SAT_BACKEND_FRACTIONALS_HPP
#define BLACK_SAT_BACKEND_FRACTIONALS_HPP

#include <limits>
#include <tuple>
#include <cmath>
#include <cstdint>

namespace black_internal {
  //
  // Thanks to Leonardo Taglialegne
  //
  inline std::pair<int, int> double_to_fraction(double n) {
    uint64_t a = (uint64_t)floor(n), b = 1;
    uint64_t c = (uint64_t)ceil(n), d = 1;

    uint64_t num = 1;
    uint64_t denum = 1;
    while(
      a + c <= (uint64_t)std::numeric_limits<int>::max() &&
      b + d <= (uint64_t)std::numeric_limits<int>::max() &&
      ((double)num/(double)denum != n)
    ) {
      num = a + c;
      denum = b + d;

      if((double)num/(double)denum > n) {
        c = num;
        d = denum;
      } else {
        a = num;
        b = denum;
      }
    }

    return {static_cast<int>(num), static_cast<int>(denum)};
  }
}

#endif
