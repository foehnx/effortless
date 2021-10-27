#pragma once

#include <array>
#include <cstdarg>
#include <cstddef>
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

struct LoggerSettings {
  bool colored = true;
  bool timed = false;
  bool relative_time = false;

  bool scientific = false;
  int initial_precision = 3;

  int name_padding = 20;
  time_t time_since;
  std::string time_format = "%H:%M:%S";
};

class Logger {
 public:
  Logger(const std::string &name,
         const LoggerSettings &settings = LoggerSettings())
    : sink_(&std::cout),
      settings_(settings),
      name_(padName(name, settings.name_padding)) {
    if (settings_.relative_time) settings_.time_since = time(nullptr);
    precision(settings_.initial_precision);
    scientific(settings.scientific);
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

  void color(const bool enable = true) { settings_.colored = enable; }

  static constexpr int MAX_CHARS = 51;

  void info(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    print(INFO, NOCOLOR, msg, args);
    va_end(args);
  }

  void warn(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    print(WARN, YELLOW, msg, args);
    va_end(args);
  }

  void error(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    print(ERROR, RED, msg, args);
    va_end(args);
  }

  void fatal(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    print(FATAL, RED, msg, args);
    va_end(args);
    throw std::runtime_error(name_);
  }

#ifdef DEBUG_LOG
  void debug(const char *msg, ...) const {
    std::va_list args;
    va_start(args, msg);
    print("", NOCOLOR, msg, args);
    va_end(args);
  }

  [[nodiscard]] std::ostream &debug() const { return *sink_ << name_; }
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
  void print(const char *prefix, const char *color, const char *msg,
             const std::va_list &args) const {
    std::array<char, MAX_CHARS> buf;
    std::vsnprintf(buf.data(), MAX_CHARS, msg, args);

    if (settings_.colored) *sink_ << color;

    *sink_ << name_;

    if (!settings_.colored) *sink_ << prefix;

    if (settings_.timed) {
      const time_t now = std::time(nullptr) - settings_.time_since;
      if (settings_.relative_time) {
        *sink_ << now << "s  ";
      } else {
        auto tm = *std::localtime(&now);
        *sink_ << std::put_time(&tm, settings_.time_format.c_str()) << "  ";
      }
    }

    *sink_ << buf.data();

    *sink_ << '\n';
  }

  static std::string padName(const std::string &name, const int padding) {
    if (name.empty()) return "";
    const std::string padded = "[" + name + "] ";
    const int extra = padding - (int)padded.size();
    return extra > 0 ? padded + std::string((size_t)extra, ' ') : padded;
  }

  static constexpr char NOCOLOR[] = "\033[0m";
  static constexpr char RED[] = "\033[31m";
  static constexpr char YELLOW[] = "\033[33m";
  static constexpr char INFO[] = "Info:    ";
  static constexpr char WARN[] = "Warning: ";
  static constexpr char ERROR[] = "Error:   ";
  static constexpr char FATAL[] = "Fatal:   ";

  std::ostream *sink_;
  LoggerSettings settings_;
  const std::string name_;
};

#ifdef _fs_found_
class FileLogger : public Logger {
 public:
  FileLogger(const std::string &name, const fs::path &file,
             const LoggerSettings &settings = LoggerSettings())
    : Logger(name, settings) {
    ofs.open(file);
    if (ofs.is_open()) {
      sink_ = &ofs;
      sink_->precision(settings.initial_precision);
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
#undef _fs_found_

}  // namespace effortless
