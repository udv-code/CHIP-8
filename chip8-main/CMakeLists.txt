configure_step("Target")

# Helper vars
get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR} NAME)
set(CHIP8_BINARY_DIR ${${CHIP8_TARGET_NAME}_BINARY_DIR}/${CURRENT_DIR})
set(CHIP8_SOURCE_DIR ${${CHIP8_TARGET_NAME}_SOURCE_DIR}/${CURRENT_DIR})

# Target
add_executable(
		${CHIP8_TARGET_NAME}

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

end_configure_step("Target")