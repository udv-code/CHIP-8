#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
// Copyright (c) 2020 udv. All rights reserved.

#include <cstdio>
#include <cerrno>
#include <cstring>

#include <random>

#include "chip8.hpp"

namespace chip8 {
	chip8::chip8() = default;
	chip8::~chip8() = default;

	void chip8::cycle() {
		opcode = memory[pc] << 8 | memory[pc + 1];

		switch (opcode & 0xF000) {
			case 0x000: {
				switch (opcode & 0x000F) {
					// 00E0: clear the screen
					case 0x000: {
						for (unsigned char &i : gfx) {
							i = 0x0;
						}
						draw = true;
						next_instruction();
						break;
					}

						// 00EE: returns from subroutine
					case 0x000E: {
						--sp;
						pc = stack[sp];
						next_instruction();
						break;
					}

					default: {
						unknown_opcode_error();
					}
				}
				break;
			}

				// 1NNN: goto NNN
			case 0x1000: {
				pc = opcode & 0x0FFF;
				break;
			}

				// 2NNN: call subroutine at NNN
			case 0x2000: {
				stack[sp] = pc;
				++sp;
				pc = opcode & 0x0FFF;
				break;
			}

				// 3XNN: skips the next instruction if VX == NN
			case 0x3000: {
				if (V[((opcode & 0x0F00) >> 8)] == (opcode & 0x00FF)) {
					next_instruction();
				}
				next_instruction();

				break;
			}

				// 4XNN: Skips the next instruction if VX doesn't equal NN.
			case 0x4000: {
				if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
					next_instruction();
				}
				next_instruction();

				break;
			}

				// 5XY0: Skips the next instruction if VX equals VY
			case 0x500: {
				if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x0F0) >> 4]) {
					next_instruction();
				}
				next_instruction();
			}

				// 6XNN: Sets VX to NN
			case 0x6000: {
				V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
				next_instruction();

				break;
			}

				// 7XNN: sets VX to NN
			case 0x7000: {
				V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
				next_instruction();

				break;
			}

			case 0x8000: {
				switch (opcode & 0x00F) {
					// 8XY0: sets VX to the value of VY
					case 0x0000: {
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
						next_instruction();

						break;
					}

						// 8XY1: sets VX to "VX OR VY"
					case 0x0001: {
						V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];

						next_instruction();
						break;
					}

						// 8XY2: sets VX to "VX AND VY"
					case 0x0002: {
						V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];

						next_instruction();
						break;
					}

						// 8XY3: sets VX to "VX XOR VY"
					case 0x0003: {
						V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];

						next_instruction();
						break;
					}

						// 8XY4: adds VY to VX. VF setis to 1 if carry.
					case 0x0004: {
						V[0xF] = 0;
						if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
							V[0xF] = 1;
						}
						V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];

						next_instruction();
						break;
					}

						// 8XY5: VY is substacted from VX. VF set to 1 if carry.
					case 0x0005: {
						V[0xF] = 1;
						if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
							V[0xF] = 0;
						}
						V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];

						next_instruction();
						break;
					}

						// 8XY6: VX >> 1. VF is set to the value of the least significant bit of VX before the shift
					case 0x0006: {
						V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
						V[(opcode & 0x0F00) >> 8] >>= 1;

						next_instruction();
						break;
					}

						// 8XY7: VX is set to (VY - VX). VF set to 0 if borrow, otherwise, to 1.
					case 0x0007: {
						V[0xF] = 1;
						if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
							V[0xF] = 0;
						}
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];

						next_instruction();
						break;
					}

						// 8XYE: VX << 1. VF is set to the value of the most significant bit of VX before the shift
					case 0x000E: {
						V[0xF] = V[(opcode & 0x0F00) >> 8] << 7;
						V[(opcode & 0x0F00) >> 8] <<= 1;

						next_instruction();
						break;
					}

					default:
						unknown_opcode_error();
				}

				break;
			}

				// 9XY0: Skips the next instruction if VX doesn't equal VY
			case 0x9000: {
				if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
					next_instruction();
				}

				next_instruction();
				break;
			}

				// ANNN: sets I to the address NNN
			case 0xA000: {
				I = opcode & 0x0FFF;
				next_instruction();
				break;
			}

				// BNNN: jumt to NNN + V0
			case 0xB000: {
				pc = (opcode * 0x0FFF) + V[0x0];
				break;
			}

				// CXNN: sets VX to a (random number & NN)
			case 0xC000: {
				V[(opcode & 0x0F00) >> 8] = (random_number() % 0xFF) & (opcode & 0x00FF);
				next_instruction();
				break;
			}

				// DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
				// Each row of 8 pixels is read as bit-coded starting from memory location I;
				// I value doesn't change after the execution of this instruction.
				// VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
				// and to 0 if that doesn't happen
			case 0xD000: {
				uint16_t x = V[(opcode & 0x0F00) >> 8];
				uint16_t y = V[(opcode & 0x00F0) >> 4];
				uint16_t height = opcode & 0x000F;
				uint16_t pixel;

				V[0xF] = 0;
				for (int yline = 0; yline < height; ++yline) {
					pixel = memory[I + yline];
					for (int xline = 0; xline < 8; ++xline) {
						if ((pixel & (0x80 >> xline)) != 0) {
							if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
								V[0xF] = 1;
							}
							gfx[(x + xline + ((y + yline) * 64))] ^= 1;
						}
					}
				}

				draw = true;
				next_instruction();
				break;
			}

			case 0xE000: {
				switch (opcode & 0x00FF) {
					// EX9E: Skips the next instruction if the key stored in VX is pressed
					case 0x009E: {
						next_instruction();
						if (key[V[(opcode & 0x0F00) >> 8]] != 0) {
							next_instruction();
						}
						break;
					}

						// EXA1: Skips the next instruction if the key stored in VX isn't pressed
					case 0x00A1: {
						next_instruction();
						if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
							next_instruction();
						}
						break;
					}

					default:
						unknown_opcode_error();
				}
				break;
			}

			case 0xF000: {
				switch (opcode & 0x00FF) {
					// FX07: Sets VX to the value of the delay timer
					case 0x0007: {
						V[(opcode & 0x0F00) >> 8] = delay_timer;
						next_instruction();
						break;
					}

						// FX0A: A key press is awaited, and then stored in VX
					case 0x000A: {
						bool keyPress = false;

						for (int i = 0; i < 16; ++i) {
							if (key[i] != 0) {
								V[(opcode & 0x0F00) >> 8] = i;
								keyPress = true;
							}
						}

						// If we didn't received a keypress, skip this cycle and try again.
						if (!keyPress) {
							return;
						}

						next_instruction();
						break;
					}

						// FX15: Sets the delay timer to VX
					case 0x0015: {
						delay_timer = V[(opcode & 0x0F00) >> 8];

						next_instruction();
						break;
					}

						// FX18: Sets the sound timer to VX
					case 0x0018: {
						sound_timer = V[(opcode & 0x0F00) >> 8];

						next_instruction();
						break;
					}

						// FX1E: Adds VX to I
					case 0x001E: {
						V[0xF] = 0;
						if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF) {
							V[0xF] = 1;  // VF is set to 1 when range overflow (I + VX > 0xFFF), and 0 when there isn't.
						}
						I += V[(opcode & 0x0F00) >> 8];

						next_instruction();
						break;
					}

						// FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
					case 0x0029: {
						I = V[(opcode & 0x0F00) >> 8] * 0x5;

						next_instruction();
						break;
					}

						// FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
					case 0x0033: {
						memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
						memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
						memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;

						next_instruction();
						break;
					}

						// FX55: Stores V0 to VX in memory starting at address I
					case 0x0055: {
						for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
							memory[I + i] = V[i];
						}

						// On the original interpreter, when the operation is done, I = I + X + 1.
						I += ((opcode & 0x0F00) >> 8) + 1;

						next_instruction();
						break;
					}

						// FX65: Fills V0 to VX with values from memory starting at address I
					case 0x0065: {
						for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
							V[i] = memory[I + i];
						}

						// On the original interpreter, when the operation is done, I = I + X + 1.
						I += ((opcode & 0x0F00) >> 8) + 1;

						next_instruction();
						break;
					}

					default:
						unknown_opcode_error();
				}
				break;
			}

			default: {
				unknown_opcode_error();
			}
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

	int chip8::random_number() noexcept {
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution dist(0, 255);
		return dist(rd);
	}
}
#pragma clang diagnostic pop