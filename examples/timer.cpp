#include "effortless/timer.hpp"

#include <unistd.h>

#include <catch2/catch.hpp>

#include "effortless/logger.hpp"


using namespace effortless;
using Scalar = double;

static constexpr Scalar tol = 1e-2;
static constexpr Scalar margin = 5e-4;

/// Example of Timer as unit test
TEST_CASE("Timer: Simple Timing", "[timer]") {
  static constexpr int N = 100;
  static constexpr Scalar dt = 0.01;  // us
  static constexpr int dt_us = (int)(1e6 * dt);

  Timer timer("Example");

  for (int i = 0; i < N; ++i) {
    timer.tic();
    usleep(dt_us);
    timer.toc();
  }

  Logger("").debug() << timer;

  CHECK(timer.count() == N);
  CHECK(timer.mean() == Approx(dt).margin(margin));
}

TEST_CASE("Timer: Advanced Timing", "[timer]") {
  static constexpr Scalar dt_min = 1e-3;
  static constexpr Scalar dt_max = 5e-3;
  static constexpr uint dt_min_us = (int)(1e6 * dt_min);
  static constexpr uint dt_max_us = (int)(1e6 * dt_max);
  static constexpr uint n = 500;
  static constexpr uint dt_step = (dt_max_us - dt_min_us) / n;

  Timer timer;

  for (uint dt = dt_min_us; dt <= dt_max_us; dt += dt_step) {
    timer.tic();
    usleep(dt);
    timer.toc();
  }

  CHECK(timer.min() == Approx(dt_min).margin(margin));
  CHECK(timer.mean() == Approx((dt_min + dt_max) / 2).margin(margin));
  CHECK(timer.max() == Approx(dt_max).margin(dt_max));
  CHECK(timer.std() == Approx((dt_max - dt_min) / sqrt(12.0)).margin(margin));
  CHECK(timer.count() == n + 1);
}

/// Example of Scopedtimer as unit test.
TEST_CASE("Timer: ScopedTimer", "[timer]") {
  static constexpr int dt = 1e5;

  Timer timer;
  {                    // As an example, a scope to put our timer in...
    ScopedTimer timy;  // ...and the timer it self. This is all you need to do.

    timer = timy;  // This thing you dont need. It's only for unit testing.

    // This would be the code to time.
    usleep(dt);
  }  // Here the scope ends, the ScopedTimer gets destroyed and prints

  // Everything below is just unit testing and not part of the example.
  timer.toc();
  CHECK(timer.mean() == Approx(1e-6 * dt).epsilon(tol));
}

TEST_CASE("Timer: Nested Timing", "[timer]") {
  static constexpr int N = 100;
  static constexpr Scalar dt = 0.001;  // us
  static constexpr int dt_us = (int)(1e6 * dt);

  Timer timer_parent{"Parent"};
  NestedTimer timer_child = timer_parent.nest("Child");

  for (int i = 0; i < N; ++i) {
    timer_parent.tic();
    usleep(dt_us);
    timer_child->tic();
    usleep(dt_us);
    timer_child->toc();
    timer_parent.toc();
  }

  Logger("").debug() << timer_parent;

  CHECK(timer_child->count() == N);
  CHECK(timer_parent.count() == N);
  CHECK(timer_child->mean() == Approx(dt).margin(margin));
  CHECK(timer_parent.mean() == Approx(2.0 * dt).margin(margin));
}
