cmake_minimum_required(VERSION 3.10)

project(osu-replay C CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/glfw)
add_subdirectory(external/stb)
add_subdirectory(external/openal-soft)
add_subdirectory(external/glad-46-core-allexts)

add_executable(osu_replay src/main.cpp)
target_link_libraries(osu_replay PUBLIC glfw OpenAL stb glad)

