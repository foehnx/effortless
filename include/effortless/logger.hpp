#pragma once

#include <array>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

// Handle filesystem include an namespace for various stdlib versions.
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#define _fs_found_
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
#define _fs_found_
namespace fs = std::experimental::filesystem;
#else
#error "no filesystem support found"
#endif

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

class Logger {
 public:
  Logger(const std::string &name, const bool color = true)
    : name_(padName(name)), colored_(color), sink_(&std::cout) {
    sink_->precision(DEFAULT_PRECISION);
  }

  Logger() = delete;
  Logger(const Logger &) = delete;
  Logger(const Logger &&) = delete;
  ~Logger() = default;

  std::streamsize precision(const std::streamsize n) {
    return sink_->precision(n);
  }

  void scientific(const bool enable = true) {
    *sink_ << (enable ? std::scientific : std::fixed);
  }

  void color(const bool enable = true) { colored_ = enable; }

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
  std::ostream &debug() const { return *sink_ << name_; }
  constexpr void debug(const std::function<void(void)> &&lambda) const {
    lambda();
  }

  static constexpr bool debugEnabled() { return true; }
#else
  constexpr void debug(const char *, ...) const noexcept {}
  [[nodiscard]] constexpr NoPrint debug() const { return NoPrint(); }
  constexpr void debug(const std::function<void(void)> &&) const  //
    noexcept {}
  static constexpr bool debugEnabled() { return false; }
#endif

  template<typename T> std::ostream &operator<<(const T &printable) const {
    return *sink_ << name_ << printable;
  }

  std::ostream &operator<<(std::ostream &(*printable)(std::ostream &)) const {
    return *sink_ << name_ << printable;
  }

  void newline() { *sink_ << '\n'; }

  void newline(const int n) {
    for (int i = 0; i < n; ++i) *sink_ << '\n';
  }

  [[nodiscard]] const std::string &name() const { return name_; }

 protected:
  static std::string padName(const std::string &name) {
    if (name.empty()) return "";
    const std::string padded = "[" + name + "] ";
    const int extra = NAME_PADDING - (int)padded.size();
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
  bool colored_;
  std::ostream *sink_;
};

#ifdef _fs_found_
class FileLogger : public Logger {
 public:
  FileLogger(const std::string &name, const fs::path &file)
    : Logger(name, false) {
    ofs.open(file);
    if (ofs.is_open()) {
      sink_ = &ofs;
      sink_->precision(DEFAULT_PRECISION);
    } else {
      color(true);
      error("Could not open file \'%s\'!\nFallback to console logging!",
            file.c_str());
    }
  }

  FileLogger() = delete;
  FileLogger(const Logger &) = delete;
  FileLogger(const Logger &&) = delete;
  ~FileLogger() { ofs.flush(); }

 private:
  std::ofstream ofs;
};
#endif
#undef _fe_found_

}  // namespace effortless
