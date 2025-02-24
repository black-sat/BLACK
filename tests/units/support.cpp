//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#include <catch.hpp>

#include <black/support/identifier.hpp>
#include <black/support/tribool.hpp>

#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <sstream>

TEST_CASE("Testing black::tribool")
{
  black::tribool tb1 = black::tribool::undef;
  black::tribool tb2 = true;
  black::tribool tb3 = false;
  
  SECTION("Output stream") {
    std::vector<std::pair<black::tribool, std::string>> tests = {
      {tb1, "tribool::undef"},
      {tb2, "true"},
      {tb3, "false"}
    };
    
    for(auto [b, res] : tests) {
      std::stringstream s;
      s << b;

      REQUIRE(s.str() == res);
    }
  }

  SECTION("Equalities") {
    REQUIRE(tb1 == tb1);
    REQUIRE(tb1 != tb2);
    REQUIRE(tb1 != tb3);

    REQUIRE(tb1 != true);
    REQUIRE(true != tb1);
    REQUIRE(tb1 != false);
    REQUIRE(false != tb1);

    REQUIRE(tb2 == true);
    REQUIRE(tb2 != false);
    REQUIRE(tb3 == false);
    REQUIRE(tb3 != true);
  }

  SECTION("operator!") {
    REQUIRE(!tb1);
    REQUIRE(tb2);
    REQUIRE(!tb3);
  }
}

TEST_CASE("identifier class")
{
  using namespace black_internal;

  STATIC_REQUIRE(stringable<int>);

  SECTION("Base types") {
    int i = 42;
    identifier h{i};

    std::hash<int> int_hash;
    std::hash<identifier> any_hash;

    REQUIRE(any_hash(h) == int_hash(i));

    REQUIRE(to_string(h) == "42");

    std::optional opt = h.to<int>();
    std::optional c = h.to<char>();

    REQUIRE(opt.has_value());
    REQUIRE(!c.has_value());
    REQUIRE(*opt == 42);

    h = 3.14f; // reassign with different type
    std::optional opt2 = h.to<float>();

    REQUIRE(opt2.has_value());
    REQUIRE(*opt2 == 3.14f);
  }

  SECTION("C strings") {
    char const* str = "hello";

    identifier h{str};

    std::optional opt = h.to<std::string>();

    REQUIRE(opt.has_value());
    REQUIRE(*opt == str);
  }

  SECTION("String views") {
    std::string_view view = "hello";

    identifier h{view};

    std::optional opt = h.to<std::string>();

    REQUIRE(opt.has_value());
    REQUIRE((*opt == view));
  }

  SECTION("Tuples") {
    using namespace black_internal;

    std::tuple<int, double> t = {42, 3.14};

    identifier h1{t};
    identifier h2{t};

    std::optional opt = h1.to<std::tuple<int, double>>();

    REQUIRE(opt.has_value());
    REQUIRE((*opt == t));
    REQUIRE(h1 == h2);
  }

  SECTION("Class types") {
    std::string s1 = "hello";
    std::string s2 = "goodbye";

    SECTION("By value") {
      identifier h{s1};

      std::optional opt = h.to<std::string>();

      REQUIRE(opt.has_value());
      REQUIRE(*opt == s1);

      h = s2; // reassignment
      std::optional opt2 = h.to<std::string>();

      REQUIRE(opt2.has_value());
      REQUIRE(*opt2 == s2);
    }

    SECTION("By move") {
      identifier h{std::move(s1)};

      std::optional opt = h.to<std::string>();

      REQUIRE(opt.has_value());
      REQUIRE(*opt == "hello");

      h = std::move(s2); // reassignment
      std::optional opt2 = h.to<std::string>();

      REQUIRE(opt2.has_value());
      REQUIRE(*opt2 == "goodbye");
    }
  }
}
