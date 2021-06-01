#pragma once

#include <chrono>
#include <functional>

namespace effortless {

template<typename T> class Throttler {
 public:
  Throttler(T& obj, const double period_seconds)
    : Throttler(obj, std::chrono::microseconds((int)(1e6 * period_seconds))) {}
  Throttler(T& obj, const std::chrono::microseconds period)
    : obj(obj), period(period) {}

  template<class F, class... Args> void operator()(F&& f, const Args&... args) {
    const std::chrono::steady_clock::time_point t_now =
      std::chrono::steady_clock::now();
    if ((t_now - t_last) > period) {
      std::invoke(f, obj, args...);
      t_last = t_now;
    }
  }

 private:
  T& obj;
  const std::chrono::microseconds period;
  std::chrono::steady_clock::time_point t_last;
};

}  // namespace effortless
