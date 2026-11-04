# pke-ecs

Lightweight Entity Component System extracted from Pikselite Engine.

## Usage

`cmake
include(FetchContent)
FetchContent_Declare(
  pke_ecs
  GIT_REPOSITORY https://github.com/Pikselite/Pikselite-ECS.git
  GIT_TAG main
)
FetchContent_MakeAvailable(pke_ecs)

target_link_libraries(your_target PRIVATE pke::pke_ecs)
`

## License
MIT
