#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <utility>

namespace osrp {
class GLContext {
 public:
  GLContext() {}
  virtual ~GLContext() {}

  virtual std::pair<int, int> GetFramebufferSize() const = 0;
  virtual bool ShouldClose() const = 0;
  virtual void BeginFrame() const = 0;
  virtual void EndFrame() const = 0;
};

class GlfwWindowGLContext : public GLContext {
 public:
  GlfwWindowGLContext(int w, int h, const char* title);
  ~GlfwWindowGLContext();

  std::pair<int, int> GetFramebufferSize() const;
  bool ShouldClose() const;
  void BeginFrame() const;
  void EndFrame() const;
 private:
  GLFWwindow* window;
};
}  // namespace osrp

