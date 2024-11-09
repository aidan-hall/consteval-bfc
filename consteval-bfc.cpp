#include <algorithm>
#include <array>
#include <compare>
#include <functional>
#include <ranges>
#include <span>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <string_view>
#include <tuple>

using namespace std;

// https://www.quora.com/How-do-you-pass-a-string-literal-as-a-parameter-to-a-C-template-class
template <int N> struct FixedString {
  constexpr static int len = N - 1;
  constexpr FixedString(char const (&s)[N]) { std::copy_n(s, N, this->elems); }
  constexpr FixedString(char const *s, size_t offset) {
    std::copy_n(s + offset, N, this->elems);
  }

  constexpr std::strong_ordering
  operator<=>(FixedString const &) const = default;
  char elems[N];
};
template <int N> FixedString(char const (&)[N]) -> FixedString<N>;

template <int N, int M>
consteval FixedString<N> subFixedString(FixedString<M> s, int64_t offset) {
  return FixedString<N>(s.elems, offset);
}

// using FixedString = pair<const char *const, size_t>;

template <FixedString src> consteval bool program_parses() {
  size_t depth = 0;
  for (size_t i = 0; i < decltype(src)::len; ++i) {
    char c = src.elems[i];
    if (c == '[') {
      depth++;
    } else if (c == ']') {
      depth--;
    }
  }

  return depth == 0;
}

// Find the index of the first closing bracket at the current depth after start.
template <FixedString src>
consteval size_t find_matching_bracket(size_t start) {
  size_t depth = 0;
  size_t i = start;
  while (i < decltype(src)::len) {
    if (src.elems[i] == '[')
      depth++;
    else if (src.elems[i] == ']')
      depth--;
    if (depth == 0)
      return i;
    i++;
  }

  return i;
}

struct Machine {
  array<uint8_t, 30000> memory;
  size_t sc = 0;
};

template <FixedString source, int64_t start, int64_t end>
consteval auto compile() {

  if constexpr (start >= end || start >= decltype(source)::len) {
    return [](Machine &mch) {
      // printf("finished at %ld %ld %c\n", start, end, source.elems[start]);
    };
  } else if constexpr (source.elems[start] == '+') {
    return [](Machine &mch) {
      // printf("+ %ld %ld\n", start, end);
      mch.memory[mch.sc]++;
      compile<source, start + 1, end>()(mch);
    };
  } else if constexpr (source.elems[start] == '-') {
    return [](Machine &mch) {
      // printf("+ %ld %ld\n", start, end);
      mch.memory[mch.sc]--;
      compile<source, start + 1, end>()(mch);
    };
  } else if constexpr (source.elems[start] == '<') {
    return [](Machine &mch) {
      // printf("< %ld %ld\n", start, end);
      mch.sc--;
      compile<source, start + 1, end>()(mch);
    };
  } else if constexpr (source.elems[start] == '>') {
    return [](Machine &mch) {
      // printf("> %ld %ld\n", start, end);
      mch.sc++;
      compile<source, start + 1, end>()(mch);
    };
  } else if constexpr (source.elems[start] == '.') {
    return [](Machine &mch) {
      putchar(mch.memory[mch.sc]);
      // printf("output %ld %ld: %d\n", start, end, mch.memory[mch.sc]);
      compile<source, start + 1, end>()(mch);
    };
  } else if constexpr (source.elems[start] == ',') {
    return [](Machine &mch) {
      // printf("input %ld %ld:\n", start, end);
      mch.memory[mch.sc] = getchar();
      compile<source, start + 1, end>()(mch);
    };
  } else if constexpr (source.elems[start] == '[') {
    return [](Machine &mch) {
      constexpr size_t match = find_matching_bracket<source>(start);
      // printf("start loop %ld %ld -> %ld\n", start, end, match);
      // constexpr auto loop_body = subFixedString<match-1,
      // decltype(source)::len>(source, 1); constexpr auto remainder =
      // subFixedString<decltype(source)::len - match - 1,
      // decltype(source)::len>(source, match + 1);
      while (mch.memory[mch.sc]) {
        compile<source, start + 1, match>()(mch);
      }
      compile<source, match + 1, end>()(mch);
    };
  } else {
    return [](Machine &mch) {
      // printf("other %ld %ld:  %d\n", start, end, source.elems[start]);
      compile<source, start + 1, end>()(mch);
    };
  }
}

template <FixedString source> consteval auto brainfuck() {
  static_assert(program_parses<source>(), "unmatched bracket");

  return []() {
    Machine mch;
    compile<source, 0, decltype(source)::len>()(mch);
  };
}

int main() {
  brainfuck<R"(
	 >++++++++[<+++++++++>-]<.	 H
	 >++++[<+++++++>-]<+.		 e
	 +++++++..			 ll
	 +++.				 o
	 >>++++++[<+++++++>-]<++.	 comma
	 ------------.			 space
	 >++++++[<+++++++++>-]<+.	 W
	 <.				 o
	 +++.				 r
	 ------.			 l	 
	 --------.			 d
	 >>>++++[<++++++++>-]<+.	 !
)">()();
  putchar('\n');
}
