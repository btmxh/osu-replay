#version 420 core

#ifndef OSRP_COMPILE
// this block will be active if it's compiled by a GLSL linter/parser, but not when it's compiled by osu-replay
#define NO_BINDLESS_TEXTURE 0
#define NV_GPU_SHADER5 1
#define MAX_QUADS 256

#define GL_EXT_SUPPORT NO_BINDLESS_TEXTURE

#endif
#line 12
#if GL_EXT_SUPPORT == NV_GPU_SHADER5
#extension GL_NV_gpu_shader5 : enable
#endif

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tcoords;

layout(location = 0) out vec2 vf_tcoords;
#if GL_EXT_SUPPORT == NV_GPU_SHADER5
layout(location = 1) flat out int quadIndex;
#endif

#if GL_EXT_SUPPORT == NV_GPU_SHADER5
struct Quad {
  float opacity;
  uint64_t tex;
};
#endif

layout(std140, binding = 0) uniform UIUniform {
  mat4 ortho;
#if GL_EXT_SUPPORT == NV_GPU_SHADER5
  Quad quad[MAX_QUADS];
#endif
};

void main() {
  gl_Position = ortho * vec4(pos, 0.0, 1.0);
  vf_tcoords = tcoords;

#if GL_EXT_SUPPORT == NV_GPU_SHADER5
  quadIndex = gl_VertexID / 6;
#endif
}

