cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

### Global configs
set(CMAKE_DEBUG_POSTFIX "-debug")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

### Modules
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(printhelp)

### Project config
set(CHIP8_PROJECT_NAME "chip8")
set(CHIP8_VERSION_MAJOR 1)
set(CHIP8_VERSION_MINOR 0)
set(CHIP8_VERSION_HOTFIX 0)
set(CHIP8_VERSION "${CHIP8_VERSION_MAJOR}.${CHIP8_VERSION_MINOR}.${CHIP8_VERSION_HOTFIX}")
set(CHIP8_NAME_VERSION "${CHIP8_PROJECT_NAME} v${CHIP8_VERSION}")
### Target Config
set(CHIP8_TARGET_NAME "${CHIP8_PROJECT_NAME}")

if (WIN32)
	set(CMAKE_USE_RELATIVE_PATHS ON)
endif ()

### Modules
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/cmake")

### Project setup
project(
		${CHIP8_PROJECT_NAME}
		VERSION ${CHIP8_VERSION}
		LANGUAGES CXX
)

### Vendor
find_package(glfw3 3.3 REQUIRED)
find_package(glad REQUIRED)
find_package(OpenGL REQUIRED)

### Target
add_subdirectory(chip8-main)

target_link_libraries(
		${CHIP8_TARGET_NAME}
		PRIVATE
		glfw
		glad::glad
		${OPENGL_LIBRARIES}
)

### Assets
configure_file(assets/pong.rom ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pong.rom COPYONLY)