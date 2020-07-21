// Copyright (c) 2020 udv. All rights reserved.

#include <cstdio>
#include <cstring>
#include <ctime>

#include "chip8.hpp"

namespace chip8 {
	chip8::chip8() = default;
	chip8::~chip8() = default;

	void chip8::cycle() noexcept {
		opcode = memory[pc] << 8 | memory[pc + 1];

		switch (opcode & 0xF000) {
			case 0x0000: {
				switch (opcode & 0x000F) {
					case 0x0000: instructions::i00E0(*this); break;
					case 0x000E: instructions::i00EE(*this); break;
					default: unknown_opcode_error(); break;
				}
				break;
			}

			case 0x1000: instructions::i1NNN(*this); break;
			case 0x2000: instructions::i2NNN(*this); break;
			case 0x3000: instructions::i3XNN(*this); break;
			case 0x4000: instructions::i4XNN(*this); break;
			case 0x5000: instructions::i5XY0(*this); break;
			case 0x6000: instructions::i6XNN(*this); break;
			case 0x7000: instructions::i7XNN(*this); break;

			case 0x8000: {
				switch (opcode & 0x000F) {
					case 0x0000: instructions::i8XY0(*this); break;
					case 0x0001: instructions::i8XY1(*this); break;
					case 0x0002: instructions::i8XY2(*this); break;
					case 0x0003: instructions::i8XY3(*this); break;
					case 0x0004: instructions::i8XY4(*this); break;
					case 0x0005: instructions::i8XY5(*this); break;
					case 0x0006: instructions::i8XY6(*this); break;
					case 0x0007: instructions::i8XY7(*this); break;
					case 0x000E: instructions::i8XYE(*this); break;
					default: unknown_opcode_error(); break;
				}
				break;
			}

			case 0x9000: instructions::i9XY0(*this); break;
			case 0xA000: instructions::iANNN(*this); break;
			case 0xB000: instructions::iBNNN(*this); break;
			case 0xC000: instructions::iCXNN(*this); break;
			case 0xD000: instructions::iDXYN(*this); break;

			case 0xE000: {
				switch (opcode & 0x00FF) {
					case 0x009E: instructions::iEX9E(*this); break;
					case 0x00A1: instructions::iEXA1(*this); break;
					default: unknown_opcode_error(); break;
				}
				break;
			}

			case 0xF000: {
				switch (opcode & 0x00FF) {
					case 0x0007: instructions::iFX07(*this); break;
					case 0x000A: instructions::iFX0A(*this); break;
					case 0x0015: instructions::iFX15(*this); break;
					case 0x0018: instructions::iFX18(*this); break;
					case 0x001E: instructions::iFX1E(*this); break;
					case 0x0029: instructions::iFX29(*this); break;
					case 0x0033: instructions::iFX33(*this); break;
					case 0x0055: instructions::iFX55(*this); break;
					case 0x0065: instructions::iFX65(*this); break;
					default: unknown_opcode_error(); break;
				}
				break;
			}

			default:
				unknown_opcode_error();
				break;
		}

		if (delay_timer > 0) {
			--delay_timer;
		}
		if (sound_timer > 0) {
			if (sound_timer == 1) {
				printf("BEEP!\n");
			}
			--sound_timer;
		}
	}

	void chip8::next_instruction() noexcept { pc += 2; }

	void chip8::unknown_opcode_error() const noexcept { printf("Unknown opcode: 0x%X\n", opcode); }

	bool chip8::load_game(const char *filename) {
		init();
		printf("Loading: %s\n", filename);

		// Open file
		FILE *file;
		errno_t error;
		if ((error = fopen_s(&file, filename, "rb")) != 0) {
			fprintf(stderr, "cannot open file '%s': %s\n",
			        filename, strerror(error));
			return false;
		}

		// Check file size
		fseek(file, 0, SEEK_END);
		long lSize = ftell(file);
		rewind(file);
		printf("Filesize: %d\n", (int) lSize);

		// Allocate memory to contain the whole file
		char *buffer = (char *) malloc(sizeof(char) * lSize);
		if (buffer == nullptr) {
			fputs("Memory error", stderr);
			return false;
		}

		// Copy the file into the buffer
		size_t result = fread(buffer, 1, lSize, file);
		if (result != lSize) {
			fputs("Reading error", stderr);
			return false;
		}

		// Copy buffer to Chip8 memory
		if ((4096 - 512) > lSize) {
			for (int i = 0; i < lSize; ++i) {
				memory[i + 512] = buffer[i];
			}
		} else {
			printf("Error: ROM too big for memory");
		}
		free(buffer);

		// Close file, free buffer
		fclose(file);
		return true;
	}

	void chip8::init() noexcept {
		srand(time(nullptr));

		pc = 0x200;
		opcode = 0;
		I = 0;
		sp = 0;

		// Clear display
		for (unsigned char &i : gfx) {
			i = 0;
		}

		// Clear stack
		for (unsigned short &i : stack) {
			i = 0;
		}

		for (int i = 0; i < 16; ++i) {
			key[i] = V[i] = 0;
		}

		// Clear memory
		for (unsigned char &i : memory) {
			i = 0;
		}

		// Reset timers
		delay_timer = 0;
		sound_timer = 0;

		for (int i = 0; i < 80; ++i) {
			memory[i] = fontset[i];
		}
	}
}