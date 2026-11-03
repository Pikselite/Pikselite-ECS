cmake_minimum_required(VERSION 3.21)
project(pke_ecs VERSION 1.0.0 LANGUAGES CXX)

add_library(pke_ecs src/EntityManager.cpp)

target_include_directories(pke_ecs PUBLIC
  $<BUILD_INTERFACE:/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(pke_ecs PUBLIC cxx_std_17)

include(CMakePackageConfigHelpers)
install(TARGETS pke_ecs EXPORT pke_ecsTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT pke_ecsTargets
  FILE pke_ecsTargets.cmake
  NAMESPACE pke::
  DESTINATION lib/cmake/pke_ecs
)
