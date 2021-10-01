#pragma once

#include <chrono>
#include <memory>
#include <regex>
#include <sstream>

#include "effortless/logger.hpp"
#include "effortless/statistic.hpp"

namespace effortless {

using Scalar = double;

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
class Timer : public Statistic {
 public:
  Timer(const std::string &name = "") : Statistic("Timer " + name) {}
  Timer(const Timer &other) = default;

  /// Start the timer.
  void tic() { t_start_ = std::chrono::high_resolution_clock::now(); }

  /// Stops timer, calculates timing, also tics again.
  Scalar toc() {
    // Calculate timing.
    const TimePoint t_end = std::chrono::high_resolution_clock::now();
    const Scalar dt =
      1e-9 * (Scalar)std::chrono::duration_cast<std::chrono::nanoseconds>(
               t_end - t_start_)
               .count();

    t_start_ = t_end;
    return this->add(dt);
  }

  /// Reset saved timings and calls;
  void reset() {
    t_start_ = TimePoint();
    Statistic::reset();
  }

  std::shared_ptr<Timer> nest(const std::string &nested_name) {
    nested_timers_.emplace_back(std::make_shared<Timer>(nested_name));
    return nested_timers_.back();
  }

  /// Custom stream operator for outputs.
  friend std::ostream &operator<<(std::ostream &os, const Timer &timer) {
    os << timer.printNested();
    return os;
  }

  /// Print timing information to console.
  void print() const { std::cout << *this; }

 private:
  [[nodiscard]] std::string printNested(const int level = 0,
                                        const Scalar parent_sum = 0.0) const {
    const int name_width = 30 - 2 * level;
    std::ostringstream ss;
    if (this->n_ < 1) {
      ss << std::left << std::setw(name_width) << this->name_
         << "has no sample yet." << std::endl;
      return ss.str();
    }

    ss.precision(3);

    ss << std::left << std::setw(name_width) << this->name_;
    ss << std::right << std::setw(8) << this->sum_ << "s  ";
    if (parent_sum != 0.0)
      ss << std::right << std::setw(3) << (int)(100.0 * this->sum_ / parent_sum)
         << "% ";
    else
      ss << std::string(5, ' ');
    ss << std::right << std::setw(8) << this->n_ << "  calls   "
       << "mean|std: ";
    ss << std::right << std::setw(8) << 1000 * this->mean() << " | ";
    ss << std::left << std::setw(8) << 1000 * this->std() << "  [min|max:  ";
    ss << std::right << std::setw(8) << 1000 * this->min_ << " | ";
    ss << std::left << std::setw(8) << 1000 * this->max_ << "]"
       << " in ms\n";

    for (const std::shared_ptr<Timer> &nested : nested_timers_) {
      for (int i = 0; i < level; ++i) ss << "| ";
      ss << "|-" << nested->printNested(level + 1, this->sum_);
    }
    return ss.str();
  }

  using TimePoint = std::chrono::high_resolution_clock::time_point;
  TimePoint t_start_;
  std::vector<std::shared_ptr<Timer>> nested_timers_;
};

using NestedTimer = std::shared_ptr<Timer>;

/*
 * Helper Timer class to time scopes from Timer constructor to destructor.
 *
 * This effectively instantiates a timer and calls `tic()` in its constructor
 * and `toc()` and ` print()` in its destructor.
 */
class ScopedTimer : public Timer {
 public:
  ScopedTimer(const std::string &name = "") : Timer(name) { this->tic(); }
  ScopedTimer(const std::string &name, Logger &&logger)
    : Timer(name), logger(&logger) {
    this->tic();
  }

  ~ScopedTimer() {
    this->toc();
    if (logger != nullptr)
      *logger << *this;
    else
      std::cout << *this;
  }

 private:
  Logger *logger{nullptr};
};

class ScopedTicToc {
 public:
  ScopedTicToc(Timer &timer) : timer(timer) { timer.tic(); }
  ~ScopedTicToc() { timer.toc(); }

 private:
  Timer &timer;
};

/*
 * Helper Timer class to instantiate a static Timer that prints in descructor.
 *
 * Debugging slow code? Simply create this as a static object somewhere and
 * tic-toc it. Once the program ends, the destructor of StaticTimer will print
 * its stats.
 */
template<typename T = double> class StaticTimer : public Timer {
 public:
  using Timer::Timer;

  ~StaticTimer() { this->print(); }
};

}  // namespace effortless
