# all .cpp files
set(
  sources
  main.cpp
  Renderer.cpp
  Materials.cpp
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${sources})
