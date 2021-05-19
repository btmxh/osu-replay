#include "glctx.hpp"

osrp::GlfwWindowGLContext::GlfwWindowGLContext(int w, int h, const char* title)
    : window([&]() {
        glfwInit();
        return glfwCreateWindow(w, h, title, NULL, NULL);
        }()) {
      glfwMakeContextCurrent(window);
      gladLoadGL(glfwGetProcAddress);
    }

osrp::GlfwWindowGLContext::~GlfwWindowGLContext() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

std::pair<int, int> osrp::GlfwWindowGLContext::GetFramebufferSize() const {
  std::pair<int, int> ret;
  glfwGetFramebufferSize(window, &ret.first, &ret.second);
  return ret;
}

bool osrp::GlfwWindowGLContext::ShouldClose() const {
  return glfwWindowShouldClose(window);
}

void osrp::GlfwWindowGLContext::BeginFrame() const { glfwPollEvents(); }

void osrp::GlfwWindowGLContext::EndFrame() const { glfwSwapBuffers(window); }

