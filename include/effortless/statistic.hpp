#pragma once

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace effortless {

using Scalar = double;

class Statistic {
 public:
  Statistic(const std::string &name = "Statistic") : name_(name) {}
  Statistic(const Statistic &rhs) = default;
  Statistic &operator=(const Statistic &rhs) {
    n_ = rhs.n_;
    last_ = rhs.last_;
    min_ = rhs.min_;
    max_ = rhs.max_;
    sum_ = rhs.sum_;
    ssum_ = rhs.ssum_;
    return *this;
  }

  Scalar operator<<(const Scalar in) {
    if (!std::isfinite(in)) return std::numeric_limits<Scalar>::quiet_NaN();

    ++n_;
    sum_ += in;
    ssum_ += in * in;
    last_ = in;
    min_ = std::min(in, min_);
    max_ = std::max(in, max_);

    return mean();
  }

  Scalar add(const Scalar in) { return operator<<(in); }

  [[nodiscard]] Scalar operator()() const { return mean(); }
  [[nodiscard]] operator double() const { return (double)mean(); }
  [[nodiscard]] operator float() const { return (float)mean(); }
  [[nodiscard]] operator int() const { return n_; }

  [[nodiscard]] int count() const { return n_; }
  [[nodiscard]] Scalar last() const { return last_; }
  [[nodiscard]] Scalar mean() const { return sum_ / ((Scalar)n_); }
  [[nodiscard]] Scalar std() const {
    if (!n_) return 0.0;
    const Scalar m = mean();
    return std::sqrt(ssum_ / n_ - m * m);
  }
  [[nodiscard]] Scalar min() const { return min_; }
  [[nodiscard]] Scalar max() const { return max_; }
  [[nodiscard]] Scalar sum() const { return sum_; }

  [[nodiscard]] const std::string &name() const { return name_; }

  void reset() {
    n_ = 0;
    sum_ = 0.0;
    ssum_ = 0.0;
    last_ = 0.0;
    min_ = std::numeric_limits<Scalar>::max();
    max_ = std::numeric_limits<Scalar>::min();
  }

  friend std::ostream &operator<<(std::ostream &os, const Statistic &s) {
    if (s.n_ < 1) os << s.name_ << "has no sample yet!" << std::endl;

    const std::streamsize prec = os.precision();
    os.precision(3);

    os << std::left << std::setw(16) << s.name_ << "mean|std  ";
    os << std::left << std::setw(5) << s.mean() << "|";
    os << std::left << std::setw(5) << s.std() << "  [min|max:  ";
    os << std::left << std::setw(5) << s.min() << "|";
    os << std::left << std::setw(5) << s.max() << "]" << std::endl;

    os.precision(prec);
    return os;
  }

 protected:
  const std::string name_;
  int n_{0};
  Scalar sum_{0.0};
  Scalar ssum_{0.0};
  Scalar last_{0.0};
  Scalar min_{std::numeric_limits<Scalar>::max()};
  Scalar max_{std::numeric_limits<Scalar>::min()};
};

}  // namespace effortless
