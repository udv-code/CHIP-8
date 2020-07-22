// Copyright (c) 2020 udv. All rights reserved.

#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "chip8.hpp"
#include "shader.hpp"

//region Emulator
chip8::chip8 *emulator;
//endregion
//region Display dimensions and data
constexpr int display_size_modifier = 10;
constexpr int display_width = DISPLAY_WIDTH * display_size_modifier;
constexpr int display_height = DISPLAY_HEIGHT * display_size_modifier;

unsigned char screen_data[DISPLAY_WIDTH][DISPLAY_HEIGHT][3] = {0};
//endregion
//region GLFW Callbacks
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
int key_state(GLFWwindow *window, int key);
void process_input(GLFWwindow *window);
void update_display_texture(const chip8::chip8 &c8);
//endregion
//region Texture
GLuint display_texture;
GLuint vbo, vao, ebo;

constexpr float display_vertices[] = {
		// Positions            // Texture coordinates
		/*LB*/-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		/*RB*/ 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		/*RT*/ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		/*LT*/-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};

constexpr GLuint indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
};
//endregion

int main(int argc, char **argv) {
	//region Setup
	emulator = new chip8::chip8{};

	if (argc < 2) {
		printf("Usage: chip8.exe <game_filename>\n\n");
		return 65;
	}

	if (!emulator->load_game(argv[1])) {
		return 1;
	}
	//endregion
	//region GLFW Context
	if (glfwInit() != GLFW_TRUE) {
		printf("Failed to initialize glfw!");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow *window = glfwCreateWindow(display_width, display_height, "CHIP8", nullptr, nullptr);
	if (window == nullptr) {
		printf("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//endregion
	//region GLad
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		printf("Failed to initialize GLAD");
		return -1;
	}
	//endregion
	//region Display setup
	glViewport(0, 0, display_width, display_height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//endregion
	//region Texture
	shader shader("texture.vs.glsl", "texture.fs.glsl");

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(display_vertices), display_vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) nullptr);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenTextures(1, &display_texture);
	glBindTexture(GL_TEXTURE_2D, display_texture);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Clear screen data
	for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
		for (int x = 0; x < DISPLAY_WIDTH; ++x) {
			screen_data[x][y][0] = screen_data[x][y][1] = screen_data[x][y][2] = sin(glfwGetTime()) * 255;
		}
	}

	// Create a texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE,
	             (GLvoid *) screen_data);
	glGenerateMipmap(GL_TEXTURE_2D);

	shader.use();
	glUniform1i(glGetUniformLocation(shader.ID, "display_texture"), 0);
	//endregion

	//region Main loop
	while (!glfwWindowShouldClose(window)) {
		process_input(window);
		glfwSwapInterval(0);

		//region Emulator cycle
		emulator->cycle();
		if (emulator->draw) {
			glClear(GL_COLOR_BUFFER_BIT);

			update_display_texture(*emulator);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, display_texture);

			shader.use();

			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

			emulator->draw = false;

#ifdef DEBUG_TEXTURE
			auto* pixels = new GLubyte[262144];
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			FILE* file = fopen("texture.tga", "w");
			fprintf(file, "%s", pixels);
			fclose(file);
#endif
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//endregion

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	glfwTerminate();
	return 0;
}

void update_display_texture(const chip8::chip8 &c8) {
	glBindTexture(GL_TEXTURE_2D, display_texture);
	// Update pixels
	for (int x = 0; x < DISPLAY_WIDTH; ++x) {
		for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
			// if (c8.gfx[(y * DISPLAY_WIDTH) + x] == 0) {
			// 	screen_data[x][y][0] = screen_data[x][y][1] = screen_data[x][y][2] = 0;    // Disabled
			// } else {
			// 	screen_data[x][y][0] = screen_data[x][y][1] = screen_data[x][y][2] = 255;  // Enabled
			// }
			screen_data[x][y][0] = screen_data[x][y][1] = screen_data[x][y][2] = x + y;
		}
	}

	// Update Texture
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
	                (GLvoid *) screen_data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void process_input(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	emulator->key[0x1] = key_state(window, GLFW_KEY_1);
	emulator->key[0x2] = key_state(window, GLFW_KEY_2);
	emulator->key[0x3] = key_state(window, GLFW_KEY_3);
	emulator->key[0xC] = key_state(window, GLFW_KEY_4);

	emulator->key[0x4] = key_state(window, GLFW_KEY_Q);
	emulator->key[0x5] = key_state(window, GLFW_KEY_W);
	emulator->key[0x6] = key_state(window, GLFW_KEY_E);
	emulator->key[0xD] = key_state(window, GLFW_KEY_R);

	emulator->key[0x7] = key_state(window, GLFW_KEY_A);
	emulator->key[0x8] = key_state(window, GLFW_KEY_S);
	emulator->key[0x9] = key_state(window, GLFW_KEY_D);
	emulator->key[0xE] = key_state(window, GLFW_KEY_F);

	emulator->key[0xA] = key_state(window, GLFW_KEY_Z);
	emulator->key[0x0] = key_state(window, GLFW_KEY_X);
	emulator->key[0xB] = key_state(window, GLFW_KEY_C);
	emulator->key[0xF] = key_state(window, GLFW_KEY_V);
}

int key_state(GLFWwindow *window, int key) {
	if (glfwGetKey(window, key) == GLFW_PRESS) {
		return 1;
	} else if (glfwGetKey(window, key) == GLFW_RELEASE) {
		return 0;
	}
	return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}