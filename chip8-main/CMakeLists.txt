configure_step("Target")

# Helper vars
get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR} NAME)
set(CHIP8_BINARY_DIR ${${CHIP8_TARGET_NAME}_BINARY_DIR}/${CURRENT_DIR})
set(CHIP8_SOURCE_DIR ${${CHIP8_TARGET_NAME}_SOURCE_DIR}/${CURRENT_DIR})
set(CHIP8_LIB_NAME ${CHIP8_TARGET_NAME}lib)

# Target
add_library(
		${CHIP8_LIB_NAME}
		STATIC
		src/chip8.hpp
		src/shader.hpp
		src/chip8.cpp
)

add_executable(
		${CHIP8_TARGET_NAME}
		src/main.cpp
)

target_link_libraries(
		${CHIP8_TARGET_NAME}
		PRIVATE
		${CHIP8_LIB_NAME}
)

set_target_properties(
		${CHIP8_TARGET_NAME}
		PROPERTIES
		CXX_STANDARD 17

		CXX_CPPLINT ""
		CXX_INCLUDE_WHAT_YOU_USE ""
		CXX_CLANG_TIDY ""
		LINK_WHAT_YOU_USE ""
)

configure_file(src/texture.fs.glsl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/texture.fs.glsl COPYONLY)
configure_file(src/texture.vs.glsl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/texture.vs.glsl COPYONLY)

end_configure_step("Target")