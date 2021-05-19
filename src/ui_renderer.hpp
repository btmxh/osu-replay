#pragma once

#include <array>
#include <memory>

#include "gl_utils.hpp"
#include "glad/gl.h"
#include "glctx.hpp"
#include "glm/glm.hpp"

namespace osrp {

class UIRenderer {
 public:
  explicit UIRenderer(GLContext& context);

  virtual void BeginFrame() = 0;
  virtual void Quad(glm::vec2 v0, glm::vec2 v1, Texture& tex,
                    float opacity = 1.0f, glm::vec2 t0 = glm::vec2(0.0f, 0.0f),
                    glm::vec2 t1 = glm::vec2(1.0f, 1.0f)) = 0;
  virtual void EndFrame() = 0;

 protected:
  GLContext& gl;
};

std::unique_ptr<UIRenderer> CreateUIRenderer(GLContext& gl);
}  // namespace osrp

