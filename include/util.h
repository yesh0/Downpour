#ifndef UTIL_H
#define UTIL_H

#include <string_view>
#include <charconv>
#include <cstddef>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <string>
#include <sstream>

/**
 * @brief Implementing C++17 std::from_chars now that GCC (10 or lower) does not
 * It may not have the performance of normal implementations of from_chars,
 * but it avoids most memory allocation by using an internal static string.
 */
template<typename Type>
std::from_chars_result from_chars(const char *begin, const char *end, Type &number) {
  static std::string FROM_CHARS_CACHE{};
  static std::istringstream SSTREAM;
  FROM_CHARS_CACHE = std::string_view(begin, end - begin);
  SSTREAM.str(FROM_CHARS_CACHE);
  SSTREAM.clear();
  try {
    SSTREAM >> number;
  } catch (...) {
    return {end, std::errc::invalid_argument};
  }
  if (SSTREAM.good() || SSTREAM.eof()) {
    return {begin + SSTREAM.tellg()};
  } else {
    return {end, std::errc::invalid_argument};
  }
}

template<typename Type>
inline Type svtov(const std::string_view &sv, std::size_t &index) {
  Type value;
  int offset = 0;
  while (offset < sv.size() && std::isspace(sv[offset])) {
    ++offset;
  }
  std::from_chars_result result = from_chars(sv.data() + offset, sv.data() + sv.size(), value);
  if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
    throw std::invalid_argument("Unable to convert");
  } else {
    index = result.ptr - sv.data();
    return value;
  }
}

template<typename Type>
inline Type svtov(const std::string_view &sv) {
  std::size_t ignored;
  return svtov<Type>(sv, ignored);
}

template<typename StringLike>
inline void split(const std::string_view &s, const char sep, std::size_t count, std::vector<StringLike> &results) {
  results.reserve(count + results.size());
  std::size_t index = 0;
  for (std::size_t i = 0; count == 0 || i != count; ++i) {
    size_t next = s.find(sep, index);
    if (next == std::string_view::npos) {
      results.emplace_back(s.substr(index));
      return;
    } else {
      results.emplace_back(s.substr(index, next - index));
      index = next + 1;
    }
  }
}

inline void split(const std::string_view &s, const char sep, std::vector<std::string_view> &results) {
  split<>(s, sep, 0, results);
}

#endif /* !UTIL_H */
