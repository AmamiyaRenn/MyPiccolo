set(ENGINE_ROOT_DIR "${PICCOLO_ROOT_DIR}/engine")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")

add_subdirectory(3rdparty)

add_subdirectory(source/runtime)
add_subdirectory(source/editor)