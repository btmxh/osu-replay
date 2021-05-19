#include <iostream>
#include <memory>

#include "beatmap.hpp"
#include "glctx.hpp"
#include "replay.hpp"
#include "timer.hpp"

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
  while (!ctx->ShouldClose()) {
    ctx->BeginFrame();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ctx->EndFrame();
  }
}

