#ifndef UTIL_H
#define UTIL_H

#include <string_view>
#include <charconv>
#include <cstddef>
#include <stdexcept>
#include <cctype>

#include <iostream>

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

#endif /* !UTIL_H */