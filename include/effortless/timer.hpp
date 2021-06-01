#pragma once

#include <chrono>
#include <regex>

#include "effortless/logger.hpp"
#include "effortless/statistic.hpp"

namespace effortless {

/*
 * Timer class to perform runtime analytics.
 *
 * This timer class provides a simple solution to time code.
 * Simply construct a timer and call it's `tic()` and `toc()` functions to time
 * code. It is intended to be used to time multiple calls of a function and not
 * only reports the `last()` timing, but also statistics such as the `mean()`,
 * `min()`, `max()` time, the `count()` of calls to the timer , and even
 * standard deviation `std()`.
 *
 * The constructor can take a name for the timer (like "update") and a name for
 * the module (like "Filter").
 * After construction it can be `reset()` if needed.
 *
 * A simple way to get the timing and stats is `std::cout << timer;` which can
 * output to arbitrary streams, overloading the stream operator,
 * or `print()` which always prints to console.
 *
 */
template<typename T = double> class Timer : public Statistic<T> {
 public:
  Timer(const std::string name = "") : Statistic<T>("Timer " + name) {}
  Timer(const Timer &other) = default;

  /// Start the timer.
  inline void tic() { t_start_ = std::chrono::high_resolution_clock::now(); }

  /// Stops timer, calculates timing, also tics again.
  T toc() {
    // Calculate timing.
    const TimePoint t_end = std::chrono::high_resolution_clock::now();
    const T dt = 1e-9 * (T)std::chrono::duration_cast<std::chrono::nanoseconds>(
                          t_end - t_start_)
                          .count();

    t_start_ = t_end;
    return this->add(dt);
  }

  /// Reset saved timings and calls;
  void reset() {
    t_start_ = TimePoint();
    Statistic<T>::reset();
  }

  void nest(const Timer &timer) { nested_timers_.push_back(&timer); }

  /// Custom stream operator for outputs.
  friend std::ostream &operator<<(std::ostream &os, const Timer &timer) {
    os << timer.printNested();
    return os;
  }

  /// Print timing information to console.
  inline void print() const { std::cout << *this; }

 private:
  std::string printNested(const int level = 0,
                          const int percentage = -1) const {
    const int name_width = 30 - 2 * level - 3 * (int)(percentage >= 0);
    std::ostringstream ss;
    if (this->n_ < 1) {
      ss << std::left << std::setw(name_width) << this->name_
         << "has no sample yet." << std::endl;
      return ss.str();
    }

    ss.precision(3);

    ss << std::left << std::setw(name_width) << this->name_;
    if (percentage >= 0) ss << std::right << std::setw(2) << percentage << "%";
    ss << std::right << std::setw(5) << this->n_ << "  calls   "
       << "mean|std: ";
    ss << std::right << std::setw(8) << 1000 * this->mean_ << " | ";
    ss << std::left << std::setw(8) << 1000 * this->S_ << "  [min|max:  ";
    ss << std::right << std::setw(8) << 1000 * this->min_ << " | ";
    ss << std::left << std::setw(8) << 1000 * this->max_ << "]"
       << " in ms\n";

    for (const Timer *const nested : nested_timers_) {
      for (int i = 0; i < level; ++i) ss << "| ";

      ss << "|-"
         << nested->printNested(level + 1,
                                (int)(100 * nested->mean() / this->mean()));
    }
    return ss.str();
  }

  using TimePoint = std::chrono::high_resolution_clock::time_point;
  TimePoint t_start_;
  std::vector<const Timer *> nested_timers_;
};

/*
 * Helper Timer class to time scopes from Timer constructor to destructor.
 *
 * This effectively instantiates a timer and calls `tic()` in its constructor
 * and `toc()` and ` print()` in its destructor.
 */
template<typename T = double> class ScopedTimer : public Timer<T> {
 public:
  ScopedTimer(const std::string &name = "") : Timer<T>(name) { this->tic(); }
  ScopedTimer(const std::string &name, Logger &&logger)
    : Timer<T>(name), logger(&logger) {
    this->tic();
  }

  ~ScopedTimer() {
    this->toc();
    if (logger != nullptr) *logger << *this;
  }

 private:
  Logger *logger{nullptr};
};

template<typename T = double> class ScopedTicToc {
 public:
  ScopedTicToc(Timer<T> &timer) : timer(timer) { timer.tic(); }
  ~ScopedTicToc() { timer.toc(); }

 private:
  Timer<T> &timer;
};

/*
 * Helper Timer class to instantiate a static Timer that prints in descructor.
 *
 * Debugging slow code? Simply create this as a static object somewhere and
 * tic-toc it. Once the program ends, the destructor of StaticTimer will print
 * its stats.
 */
template<typename T = double> class StaticTimer : public Timer<T> {
 public:
  using Timer<T>::Timer;

  ~StaticTimer() { this->print(); }
};

}  // namespace effortless
