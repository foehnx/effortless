#pragma once

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace effortless {

template<typename T = double> class Statistic {
 public:
  Statistic(const std::string &name = "Statistic",
            const int max_samples = std::numeric_limits<int>::max())
    : name_(name), max_samples_(max_samples) {}
  Statistic(const Statistic &rhs) = default;
  Statistic &operator=(const Statistic &rhs) {
    n_ = rhs.n_;
    mean_ = rhs.mean_;
    last_ = rhs.last_;
    S_ = rhs.S_;
    min_ = rhs.min_;
    max_ = rhs.max_;
    return *this;
  }

  T operator<<(const T in) {
    if (!std::isfinite(in)) return in;

    mean_ = n_ ? mean_ : in;
    last_ = in;
    const T mean_last = mean_;
    n_ += n_ < max_samples_ ? 1 : 0;
    mean_ += (in - mean_last) / (T)(n_);
    S_ += (in - mean_last) * (in - mean_);
    min_ = std::min(in, min_);
    max_ = std::max(in, max_);

    return mean_;
  }

  T add(const T in) { return operator<<(in); }

  T operator()() const { return mean_; }
  operator double() const { return (double)mean_; }
  operator float() const { return (float)mean_; }
  operator int() const { return n_; }

  int count() const { return n_; }
  T last() const { return last_; }
  T mean() const { return mean_; }
  T std() const { return n_ > 1 ? std::sqrt(S_ / (T)(n_ - 1)) : 0.0; }
  T min() const { return min_; }
  T max() const { return max_; }

  const std::string &name() const { return name_; }

  void reset() {
    n_ = 0;
    last_ = 0.0;
    mean_ = 0.0;
    S_ = 0.0;
    min_ = std::numeric_limits<T>::max();
    max_ = std::numeric_limits<T>::min();
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
  const int max_samples_;
  int n_{0};
  T mean_{0.0};
  T last_{0.0};
  T S_{0.0};
  T min_{std::numeric_limits<T>::max()};
  T max_{std::numeric_limits<T>::min()};
};

}  // namespace effortless
