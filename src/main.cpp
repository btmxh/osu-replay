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
  GLuint ffs = 131185;
  glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &ffs, false);

  osrp::Texture texture;
  osrp::CreateTexture(texture, "res/magma/bg.jpg");

  osrp::Texture cursor, cursorTrail;
  osrp::CreateTexture(cursor, "res/skin/cursor.png");
  osrp::CreateTexture(cursorTrail, "res/skin/cursortrail.png");
  if (GLAD_GL_ARB_bindless_texture) {
    texture.MakeResident();
    cursor.MakeResident();
    cursorTrail.MakeResident();
  }

  auto frameBefore = replay.replayData.begin();
  constexpr size_t TRAIL_FRAMES = 4;

  std::unique_ptr<osrp::UIRenderer> renderer = osrp::CreateUIRenderer(*ctx);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  timer->SetSpeed(1.5);

  while (!ctx->ShouldClose()) {
    ctx->BeginFrame();

    glClear(GL_COLOR_BUFFER_BIT);

    renderer->BeginFrame();

    auto time = timer->GetTime();

    while (frameBefore < replay.replayData.end() - 1) {
      if (time * 1000.0 >= (frameBefore + 1)->time) {
        ++frameBefore;
      } else {
        break;
      }
    }
    if (frameBefore != replay.replayData.end() &&
        time * 1000.0 >= frameBefore->time) {
      size_t itr = 0;
      const glm::vec2 off{30.0f, 30.0f};
      for (auto it = frameBefore; it-- > replay.replayData.begin();) {
        renderer->Quad(it->pos - off, it->pos + off, cursorTrail);
        if (++itr >= TRAIL_FRAMES) {
          break;
        }
      }

      auto pos = frameBefore->pos;
      if (frameBefore < replay.replayData.end() - 1) {
        auto frameAfter = frameBefore + 1;
        assert(frameAfter->time >= time * 1000.0);
        float lerpFactor = (float)(time * 1000.0f - frameBefore->time) /
                           (frameAfter->time - frameBefore->time);
        std::cout << lerpFactor << std::endl;
        pos =
            frameBefore->pos * (1 - lerpFactor) + frameAfter->pos * lerpFactor;
      }

      renderer->Quad(pos - off, pos + off, cursor);
    }

    renderer->EndFrame();

    ctx->EndFrame();
  }
}

