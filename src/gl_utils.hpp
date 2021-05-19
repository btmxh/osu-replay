#pragma once

#include <glad/gl.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "io.hpp"
#include "result.hpp"
#include "stb_image.h"

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

struct Texture {
  static GLuint create() {
    GLuint t;
    glGenTextures(1, &t);
    return t;
  }

  static void destroy(GLuint t) { glDeleteTextures(1, &t); }
};

struct ShaderProgram {
  static GLuint create() { return glCreateProgram(); }

  static void destroy(GLuint program) { glDeleteProgram(program); }
};
}  // namespace raii_gl

template <typename GLType, typename RAII>
class GLHandle {
 public:
  template <typename... Args>
  explicit GLHandle(Args&&... args) {
    handle = RAII::create(std::forward<Args>(args)...);
  }

  explicit GLHandle(GLType handle) : handle(handle) {}

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
class Texture : public GLHandle<GLuint, raii_gl::Texture> {
 public:
  Texture() : GLHandle() {}

  GLuint64 GetBindlessHandle() const { return bindlessHandle; }

  void MakeResident() {
    glMakeTextureHandleResidentARB(bindlessHandle =
                                       glGetTextureHandleARB(Get()));
  }

 private:
  GLuint64 bindlessHandle;
};
using ShaderProgram = GLHandle<GLuint, raii_gl::ShaderProgram>;

enum class ShaderErrorCode { SHADER = 0, PROGRAM = 1 };

struct ShaderErrorCategory : public std::error_category {
  const char* name() const noexcept override { return "shader_error_category"; }

  std::string message(int type) const noexcept override {
    switch (type) {
      case static_cast<int>(ShaderErrorCode::SHADER):
        return "shader_error";
      case static_cast<int>(ShaderErrorCode::PROGRAM):
        return "program_error";
      default:
        return "unknown_error";
    }
  }
};

const ShaderErrorCategory seCategory;

inline std::error_code MakeErrorCode(ShaderErrorCode err) {
  return std::error_code(static_cast<int>(err), seCategory);
}

template <typename StringType>
inline Result<GLuint> CreateShaderProgram(
    const std::map<GLenum, std::vector<StringType>>& sources) {
  GLuint program = glCreateProgram();
  GLint i;
  std::vector<GLuint> shaders;
  shaders.reserve(sources.size());
  for (const auto& [type, source] : sources) {
    auto shader = glCreateShader(type);
    std::vector<const char*> str;
    std::vector<GLint> sizes;
    str.reserve(source.size());
    sizes.reserve(source.size());
    for (const auto& sourceStr : source) {
      str.push_back(sourceStr.data());
      sizes.push_back(sourceStr.size());
    }
    glShaderSource(shader, source.size(), str.data(), sizes.data());
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &i);
    if (!i) {
      char error[256];
      glGetShaderInfoLog(shader, sizeof(error), nullptr, error);
      std::cerr << "Shader compilation failed. Error: " << error << std::endl;
      return Result<GLuint>(MakeErrorCode(ShaderErrorCode::SHADER));
    }

    glAttachShader(program, shader);
    shaders.push_back(shader);
  }

  glLinkProgram(program);
  glValidateProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &i);
  if (!i) {
    char error[256];
    glGetProgramInfoLog(program, sizeof(error), nullptr, error);
    std::cerr << "Program linking failed. Error: " << error << std::endl;
    return Result<GLuint>(MakeErrorCode(ShaderErrorCode::PROGRAM));
  }

  for (auto shader : shaders) {
    glDetachShader(program, shader);
    glDeleteShader(shader);
  }

  return Result<GLuint>(program);
}

struct STBIErrorCategory : public std::error_category {
  const char* name() const noexcept override { return "stbi_error"; }
  std::string message(int i) const noexcept override { return name(); }
};

extern const STBIErrorCategory stbieCategory;

inline bool CreateTexture(Texture& texture, const fs::path& path) {
  auto pathstr = path.string();
  int w, h, c;
  auto pixels = stbi_load(pathstr.c_str(), &w, &h, &c, 0);
  if (!pixels) {
    std::cerr << "failed to load image at path" << pathstr
              << ". error: " << stbi_failure_reason() << std::endl;
    return false;
  }
  glBindTexture(GL_TEXTURE_2D, texture);
  auto type = [&]() -> GLenum {
    switch (c) {
      case 1:
        return GL_RED;
      case 2:
        return GL_RG;
      case 3:
        return GL_RGB;
      default:
        return GL_RGBA;
    };
  }();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, type, GL_UNSIGNED_BYTE,
               pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  stbi_image_free(pixels);
  return true;
}

}  // namespace osrp

