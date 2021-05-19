#include "timer.hpp"

#include <GLFW/glfw3.h>
#include <chrono>

namespace osrp {

Timer::Timer(std::function<double()> rawTimer)
    : startTime(rawTimer()), speed(1.0) {}

double Timer::GetRawTime() const { return rawTimer(); }
double Timer::GetTime() const { return (rawTimer() - startTime) / speed; }

double Timer::GetSpeed() const { return speed; }
void Timer::SetSpeed(double speed) { this->speed = speed; }

GlfwTimer::GlfwTimer(): Timer([](){ return glfwGetTime(); }) {}
HighResTimer::HighResTimer(): Timer([]() { return std::chrono::high_resolution_clock::now(); })

}  // namespace osrp

