#include <iostream>
#include <memory>

#include "beatmap.hpp"
#include "glctx.hpp"
#include "replay.hpp"
#include "timer.hpp"
#include "ui_renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
  osrp::Beatmap map("res/magma/magma_top_diff.osu");
  std::cout << map.GetProperty(osrp::KeyValueSection::METADATA, "Title").Value()
            << std::endl;

  osrp::Replay replay("res/magma/wc_replay.osr");
  std::cout << replay.playerName << std::endl;

  std::unique_ptr<osrp::AbstractTimer> timer =
      std::make_unique<osrp::HighResTimer>();

  std::unique_ptr<osrp::GLContext> ctx =
      std::make_unique<osrp::GlfwWindowGLContext>(1280, 720, "osu!");

  glDebugMessageCallback(
      [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
         GLchar const* message, void const* user_param) {
        auto const src_str = [source]() {
          switch (source) {
            case GL_DEBUG_SOURCE_API:
              return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
              return "WINDOW SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
              return "SHADER COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY:
              return "THIRD PARTY";
            case GL_DEBUG_SOURCE_APPLICATION:
              return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER:
              return "OTHER";
          }
        }();

        auto const type_str = [type]() {
          switch (type) {
            case GL_DEBUG_TYPE_ERROR:
              return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
              return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
              return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY:
              return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE:
              return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER:
              return "MARKER";
            case GL_DEBUG_TYPE_OTHER:
              return "OTHER";
          }
        }();

        auto const severity_str = [severity]() {
          switch (severity) {
            case GL_DEBUG_SEVERITY_NOTIFICATION:
              return "NOTIFICATION";
            case GL_DEBUG_SEVERITY_LOW:
              return "LOW";
            case GL_DEBUG_SEVERITY_MEDIUM:
              return "MEDIUM";
            case GL_DEBUG_SEVERITY_HIGH:
              return "HIGH";
          }
        }();

        std::cout << src_str << ", " << type_str << ", " << severity_str << ", "
                  << id << ": " << message << '\n';
      },
      nullptr);
  glEnable(GL_DEBUG_OUTPUT);

  osrp::Texture texture;
  osrp::CreateTexture(texture, "res/magma/bg.jpg");
  if (GLAD_GL_ARB_bindless_texture) {
    texture.MakeResident();
  }

  std::unique_ptr<osrp::UIRenderer> renderer = osrp::CreateUIRenderer(*ctx);

  while (!ctx->ShouldClose()) {
    ctx->BeginFrame();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderer->BeginFrame();

    renderer->Quad({100.0f, 100.0f}, {300.0f, 300.0f}, texture);
    renderer->Quad({200.0f, 100.0f}, {300.0f, 300.0f}, texture);
    renderer->Quad({100.0f, 300.0f}, {300.0f, 500.0f}, texture);

    renderer->EndFrame();

    ctx->EndFrame();
  }
}

