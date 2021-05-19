#pragma once

#include <GLFW/glfw3.h>

#include <chrono>
#include <functional>

namespace osrp {

class AbstractTimer {
  virtual double GetTime() const = 0;
  virtual double GetSpeed() const = 0;
  virtual void SetSpeed(double speed) = 0;
};

template <typename TimePoint>
class Timer : public AbstractTimer {
 public:
  Timer(std::function<TimePoint()> rawTimer,
        std::function<double(TimePoint, TimePoint)> diff)
      : rawTimer(rawTimer), diff(diff), startTime(rawTimer()), speed(1.0) {}

  TimePoint GetRawTime() const { return rawTimer(); }

  double GetTime() const { return diff(startTime, rawTimer()); }

  double GetSpeed() const { return speed; }
  void SetSpeed(double speed) { this->speed = speed; }

 private:
  std::function<TimePoint()> rawTimer;
  std::function<double(TimePoint, TimePoint)> diff;
  TimePoint startTime;
  double speed;
};

class GlfwTimer : public Timer<double> {
 public:
  GlfwTimer()
      : Timer([]() { return glfwGetTime(); },
              [](double s, double e) { return e - s; }) {}
};

class HighResTimer
    : public Timer<decltype(std::chrono::high_resolution_clock::now())> {
 public:
  using TimePoint = decltype(std::chrono::high_resolution_clock::now());
  // clang-format off
  HighResTimer()
      : Timer(
            []() { return std::chrono::high_resolution_clock::now(); },
            [](auto s, auto e) {
              return std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                         .count() / 1e9;
            }) {}
  // clang-format on
};
}  // namespace osrp

