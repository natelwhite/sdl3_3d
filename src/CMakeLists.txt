# all .cpp files
set(
  sources
  main.cpp
  Renderer.cpp
  Materials.cpp
  Math.cpp
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${sources})
