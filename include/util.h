#ifndef UTIL_H
#define UTIL_H

#include <string_view>
#include <charconv>
#include <cstddef>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <string>

template<typename Type>
inline Type svtov(const std::string_view &sv, std::size_t &index) {
  Type value;
  int offset = 0;
  while (offset < sv.size() && std::isspace(sv[offset])) {
    ++offset;
  }
  std::from_chars_result result = std::from_chars(sv.data() + offset, sv.data() + sv.size(), value);
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
