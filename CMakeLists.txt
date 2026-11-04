cmake_minimum_required(VERSION 3.21)
project(pke_ecs VERSION 1.0.0 LANGUAGES CXX)

option(PKE_ECS_BUILD_TESTS "Build tests" OFF)
option(PKE_ECS_INSTALL "Install targets" ON)

add_library(pke_ecs
  src/EntityManager.cpp
  src/SystemManager.cpp
)

target_include_directories(pke_ecs
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(pke_ecs PUBLIC cxx_std_17)

if(PKE_ECS_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
  enable_testing()
  add_subdirectory(tests)
endif()

if(PKE_ECS_INSTALL)
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
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pke_ecsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pke_ecsConfig.cmake
    INSTALL_DESTINATION lib/cmake/pke_ecs
  )
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/pke_ecsConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pke_ecsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/pke_ecsConfigVersion.cmake
    DESTINATION lib/cmake/pke_ecs
  )
endif()
