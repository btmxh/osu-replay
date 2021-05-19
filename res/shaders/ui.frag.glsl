#version 420 core

#ifndef OSRP_COMPILE
// this block will be active if it's compiled by a GLSL linter/parser, but not
// when it's compiled by osu-replay
#define NO_BINDLESS_TEXTURE 0
#define NV_GPU_SHADER5 1
#define MAX_QUADS 256

#define GL_EXT_SUPPORT NO_BINDLESS_TEXTURE

#endif
#line 12
#if GL_EXT_SUPPORT == NV_GPU_SHADER5
#extension GL_NV_gpu_shader5 : enable
#endif

#define tc vf_tcoords
layout(location = 0) in vec2 tc;
#if GL_EXT_SUPPORT == NV_GPU_SHADER5
layout(location = 1) flat in int quadIndex;
#endif

layout(location = 0) out vec4 color;

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

#if GL_EXT_SUPPORT == NO_BINDLESS_TEXTURE
layout(binding = 1) uniform sampler2D tex;
#endif

void main() {
#if GL_EXT_SUPPORT == NO_BINDLESS_TEXTURE
  color = texture(tex, tc);
#elif GL_EXT_SUPPORT == NV_GPU_SHADER5
  color = texture(sampler2D(quad[quadIndex].tex), tc);
#endif
}

