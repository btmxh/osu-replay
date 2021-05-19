#include "ui_renderer.hpp"

#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "io.hpp"
#include "strings.hpp"

namespace osrp {

namespace uir {
enum class GLExtSupport { NO_BINDLESS_TEXTURE, NV_GPU_SHADER5 };

struct Vertex {
  glm::vec2 pos;
  glm::vec2 tcoords;
};

template <GLExtSupport support>
struct Quad {
  float opacity;
  uint64_t texture;
};

template <>
struct Quad<GLExtSupport::NO_BINDLESS_TEXTURE> {
  float opacity;
};

constexpr GLsizei MAX_QUADS = 256;

template <GLExtSupport support>
constexpr GLsizei MAX_VERTICES = MAX_QUADS * 6;

template <GLExtSupport support>
struct VBO {
  std::array<Vertex, MAX_VERTICES<support>> vertices;
};

template <GLExtSupport support>
struct UBO {
  glm::mat4 ortho;
  std::array<Quad<support>, MAX_QUADS> quads;
};

struct DrawArraysIndirectCommand {
  GLuint count;
  GLuint instanceCount;
  GLuint first;
  GLuint baseInstance;
};

template <typename T, GLenum BindSlot>
class TypedBuffer : public Buffer {
 public:
  explicit TypedBuffer(const T& initialValue, GLenum usage = GL_STREAM_DRAW)
      : Buffer() {
    glBindBuffer(BindSlot, Get());
    glBufferData(BindSlot, sizeof(initialValue),
                 reinterpret_cast<const void*>(&initialValue), usage);
  }

  explicit TypedBuffer(GLenum usage = GL_STREAM_DRAW) : Buffer() {
    glBindBuffer(BindSlot, Get());
    glBufferData(BindSlot, sizeof(T), nullptr, usage);
  }

  void Bind() { glBindBuffer(BindSlot, Get()); }

  T& Map() {
    Bind();
    void* buf = glMapBuffer(BindSlot, GL_READ_WRITE);
    return *reinterpret_cast<T*>(buf);
  }

  T& MapWrite() {
    Bind();
    void* buf = glMapBuffer(BindSlot, GL_WRITE_ONLY);
    return *reinterpret_cast<T*>(buf);
  }

  const T& MapRead() {
    Bind();
    const void* buf = glMapBuffer(BindSlot, GL_READ_ONLY);
    return *reinterpret_cast<const T*>(buf);
  }

  void Unmap() {
    Bind();
    glUnmapBuffer(BindSlot);
  }
};

/* some comments:
 *
 * Depending on the support of OpenGL on client machine, we will use different
 * rendering techniques. If ARB_bindless_textures is supported, we can draw
 * everything in one call, but we still need another helper extension.
 *
 * The best one is NV_gpu_shader5, but it's NVIDIA specific.
 */
template <GLExtSupport support>
class UIRendererImpl : public UIRenderer {
 public:
  explicit UIRendererImpl(GLContext& gl)
      : UIRenderer(gl), program(CreateProgram()) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<const void*>(offsetof(Vertex, pos)));
    glVertexAttribPointer(
        1, 2, GL_FLOAT, false, sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, tcoords)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
  }

  void Flush() {
    vbo.Unmap();
    ubo.Unmap();
    glUseProgram(program);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    if constexpr (support == GLExtSupport::NO_BINDLESS_TEXTURE) {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, currentTexture);
      currentTexture = 0;
    }
    glDrawArrays(GL_TRIANGLES, 0, quads * 6);
  }

  void Rebind() {
    vboData = &vbo.MapWrite();
    uboData = &ubo.MapWrite();
    vboPtr = 0;
    uboPtr = 0;
    quads = 0;
  }

  void FlushAndRebind() {
    Flush();
    Rebind();
  }

  void BeginFrame() override {
    Rebind();
    auto [w, h] = gl.GetFramebufferSize();
    uboData->ortho =
        glm::ortho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f);
  }
  void EndFrame() override { Flush(); }
  void Quad(glm::vec2 v0, glm::vec2 v1, Texture& tex, float opacity = 1.0f,
            glm::vec2 t0 = glm::vec2(0.0f, 0.0f),
            glm::vec2 t1 = glm::vec2(1.0f, 1.0f)) override {
    if constexpr (support == GLExtSupport::NO_BINDLESS_TEXTURE) {
      if (tex.Get() != currentTexture && currentTexture != 0) {
        FlushAndRebind();
      }
      currentTexture = tex.Get();
    }
    vboData->vertices[vboPtr++] = {{v0.x, v0.y}, {t0.s, t0.t}};
    vboData->vertices[vboPtr++] = {{v0.x, v1.y}, {t0.s, t1.t}};
    vboData->vertices[vboPtr++] = {{v1.x, v1.y}, {t1.s, t1.t}};
    vboData->vertices[vboPtr++] = {{v1.x, v1.y}, {t1.s, t1.t}};
    vboData->vertices[vboPtr++] = {{v1.x, v0.y}, {t1.s, t0.t}};
    vboData->vertices[vboPtr++] = {{v0.x, v0.y}, {t0.s, t0.t}};
    uboData->quads[uboPtr].opacity = opacity;
    if constexpr (support == GLExtSupport::NV_GPU_SHADER5) {
      uboData->quads[uboPtr].texture = tex.GetBindlessHandle();
    }
    uboPtr++;
    quads++;
  }

 private:
  ShaderProgram program;
  VertexArray vao;
  TypedBuffer<VBO<support>, GL_ARRAY_BUFFER> vbo;
  TypedBuffer<UBO<support>, GL_UNIFORM_BUFFER> ubo;

  VBO<support>* vboData;
  UBO<support>* uboData;
  size_t vboPtr = 0;
  size_t uboPtr = 0;

  // only matter when support == GLExtSupport::NO_BINDLESS_TEXTURE
  GLuint currentTexture = 0;

  GLsizei quads;

  static constexpr std::string_view SHADER_HEADER = R"(
#version 420 core
#define NO_BINDLESS_TEXTURE 0
#define NV_GPU_SHADER5 1
#define OSRP_COMPILE
#define MAX_QUADS 256
)";

  static GLuint CreateProgram() {
    auto loadShader = [&](const fs::path& shaderPath) {
      std::stringstream content;
      ReadLines(shaderPath, [&](const auto& line) {
        // since a lot of GLSL linter will throw an error if we don't put
        // #version directives in the first line, the glsl file will also
        // contain the #version directive
        if (!RemovePrefix(line, "#version").has_value())
          content << line << "\n";
      });
      return content.str();
    };

    auto vertexCode = loadShader("res/shaders/ui.vert.glsl");
    auto fragmentCode = loadShader("res/shaders/ui.frag.glsl");

    auto nonConstexprCode = std::string("#define GL_EXT_SUPPORT ") +
                            std::to_string(static_cast<int>(support)) + "\n";

    std::map<GLenum, std::vector<std::string_view>> sources = {
        {GL_VERTEX_SHADER, {SHADER_HEADER, nonConstexprCode, vertexCode}},
        {GL_FRAGMENT_SHADER, {SHADER_HEADER, nonConstexprCode, fragmentCode}}};

    auto result = CreateShaderProgram(sources);
    if (!result) {
      throw std::runtime_error("Unable to create shader");
    }

    return result.Value();
  }
};

}  // namespace uir

UIRenderer::UIRenderer(GLContext& gl) : gl(gl) {}

std::unique_ptr<UIRenderer> CreateUIRenderer(GLContext& gl) {
  if (GLAD_GL_ARB_bindless_texture && GLAD_GL_NV_gpu_shader5) {
    return std::make_unique<
        uir::UIRendererImpl<uir::GLExtSupport::NV_GPU_SHADER5>>(gl);
  }
  return std::make_unique<
      uir::UIRendererImpl<uir::GLExtSupport::NO_BINDLESS_TEXTURE>>(gl);
}

}  // namespace osrp

