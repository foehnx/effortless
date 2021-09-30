#pragma once

#include <array>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

namespace effortless {

namespace {
struct NoPrint {
  template<typename T> constexpr NoPrint operator<<(const T &) const noexcept {
    return NoPrint();
  }
  constexpr NoPrint operator<<(std::ostream &(*)(std::ostream &)) const  //
    noexcept {
    return NoPrint();
  }
};
}  // namespace

template<typename OutputStream> class LoggerBase {
 public:
  LoggerBase(const std::string &name, const bool color = true)
    : name_(padName(name)), colored_(color) {
    sink_ = std::make_unique<std::ostream>(std::cout.rdbuf());
    sink_->precision(DEFAULT_PRECISION);
  }

  inline std::streamsize precision(const std::streamsize n) {
    return sink_->precision(n);
  }
  inline void scientific(const bool on = true) {
    *sink_ << (on ? std::scientific : std::fixed);
  }

  static constexpr int MAX_CHARS = 256;

  void info(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    std::array<char, MAX_CHARS> buf;
    const int n = std::vsnprintf(buf.data(), MAX_CHARS, msg, args);
    va_end(args);
    if (n < 0 || n >= MAX_CHARS)
      *sink_ << name_ << "=== Logging error ===" << std::endl;
    if (colored_)
      *sink_ << name_ << buf.data() << std::endl;
    else
      *sink_ << name_ << INFO << buf.data() << std::endl;
  }

  void warn(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    std::array<char, MAX_CHARS> buf;
    const int n = std::vsnprintf(buf.data(), MAX_CHARS, msg, args);
    va_end(args);
    if (n < 0 || n >= MAX_CHARS)
      *sink_ << name_ << "=== Logging error ===" << std::endl;
    if (colored_)
      *sink_ << YELLOW << name_ << buf.data() << RESET << std::endl;
    else
      *sink_ << name_ << WARN << buf.data() << std::endl;
  }

  void error(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    std::array<char, MAX_CHARS> buf;
    const int n = std::vsnprintf(buf.data(), MAX_CHARS, msg, args);
    va_end(args);
    if (n < 0 || n >= MAX_CHARS)
      *sink_ << name_ << "=== Logging error ===" << std::endl;
    if (colored_)
      *sink_ << RED << name_ << buf.data() << RESET << std::endl;
    else
      *sink_ << name_ << ERROR << buf.data() << std::endl;
  }

  void fatal(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    std::array<char, MAX_CHARS> buf;
    const int n = std::vsnprintf(buf.data(), MAX_CHARS, msg, args);
    va_end(args);
    if (n < 0 || n >= MAX_CHARS)
      *sink_ << name_ << "=== Logging error ===" << std::endl;
    if (colored_)
      *sink_ << RED << name_ << buf.data() << RESET << std::endl;
    else
      *sink_ << name_ << FATAL << buf.data() << std::endl;
    throw std::runtime_error(name_ + buf.data());
  }

#ifdef DEBUG_LOG
  void debug(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    std::array<char, MAX_CHARS> buf;
    const int n = std::vsnprintf(buf.data(), MAX_CHARS, msg, args);
    va_end(args);
    if (n < 0 || n >= MAX_CHARS)
      *sink_ << name_ << "=== Logging error ===" << std::endl;
    *sink_ << name_ << buf.data() << std::endl;
  }
  OutputStream &debug() const { return *sink_ << name_; }
  constexpr void debug(const std::function<void(void)> &&lambda) const {
    lambda();
  }

  static constexpr bool debugEnabled() { return true; }
#else
  inline constexpr void debug(const char *, ...) const noexcept {}
  inline constexpr NoPrint debug() const { return NoPrint(); }
  inline constexpr void debug(const std::function<void(void)> &&) const  //
    noexcept {}
  static constexpr bool debugEnabled() { return false; }
#endif

  template<typename T> OutputStream &operator<<(const T &printable) const {
    return *sink_ << name_ << printable;
  }

  OutputStream &operator<<(OutputStream &(*printable)(OutputStream &)) const {
    return *sink_ << name_ << printable;
  }

  inline void newline(const int n = 1) {
    *sink_ << std::string((size_t)n, '\n');
  }

  [[nodiscard]] inline const std::string &name() const { return name_; }

 protected:
  static std::string padName(const std::string &name) {
    if (name.empty()) return "";
    const std::string padded = "[" + name + "] ";
    const int extra = LoggerBase::NAME_PADDING - (int)padded.size();
    return extra > 0 ? padded + std::string((size_t)extra, ' ') : padded;
  }

  static constexpr int DEFAULT_PRECISION = 3;
  static constexpr int NAME_PADDING = 20;
  static constexpr char RESET[] = "\033[0m";
  static constexpr char RED[] = "\033[31m";
  static constexpr char YELLOW[] = "\033[33m";
  static constexpr char INFO[] = "Info:    ";
  static constexpr char WARN[] = "Warning: ";
  static constexpr char ERROR[] = "Error:   ";
  static constexpr char FATAL[] = "Fatal:   ";

  const std::string name_;
  const bool colored_;
  std::shared_ptr<OutputStream> sink_;
};

using Logger = LoggerBase<std::ostream>;

}  // namespace effortless
