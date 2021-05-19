#pragma once

#include <glad/gl.h>

#include <utility>

namespace osrp {
namespace raii_gl {
struct VertexArray {
  static GLuint create() {
    GLuint va;
    glGenVertexArrays(1, &va);
    return va;
  }

  static void destroy(GLuint va) { glDeleteVertexArrays(1, &va); }
};

struct Buffer {
  static GLuint create() {
    GLuint b;
    glGenBuffers(1, &b);
    return b;
  }

  static void destroy(GLuint b) { glDeleteBuffers(1, &b); }
};
}  // namespace raii_gl

template <typename GLType, typename RAII>
class GLHandle {
 public:
  template <typename... Args>
  explicit GLHandle(Args&&... args) {
    handle = RAII::create(std::forward<Args>(args)...);
  }

  ~GLHandle() { RAII::destroy(handle); }

  GLType Get() const { return handle; }
  operator GLType() const { return handle; }
  GLType* operator->() { return &handle; }
  const GLType* operator->() const { return handle; }

 private:
  GLType handle;
};

using VertexArray = GLHandle<GLuint, raii_gl::VertexArray>;
using Buffer = GLHandle<GLuint, raii_gl::Buffer>;

}  // namespace osrp

