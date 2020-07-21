// Copyright (c) 2020 udv. All rights reserved.

#ifndef CHIP8
#define CHIP8

#include <cstdint>
#include <cstdlib>

#define CHIP8_DISPLAY_WIDTH_DEFAULT 64
#define CHIP8_DISPLAY_HEIGHT_DEFAULT 32
#define CHIP8_DISPLAY_SIZE_DEFAULT CHIP8_DISPLAY_WIDTH_DEFAULT * CHIP8_DISPLAY_HEIGHT_DEFAULT

#define DISPLAY_WIDTH CHIP8_DISPLAY_WIDTH_DEFAULT
#define DISPLAY_HEIGHT CHIP8_DISPLAY_HEIGHT_DEFAULT
#define DISPLAY_SIZE CHIP8_DISPLAY_SIZE_DEFAULT

#define INSTRUCTION_NAME(x) i##x
#define INSTRUCTION(x) static void INSTRUCTION_NAME(x) (chip8 &c) noexcept

namespace chip8 {
	constexpr unsigned char fontset[80] = {
			0xF0, 0x90, 0x90, 0x90, 0xF0, //0
			0x20, 0x60, 0x20, 0x20, 0x70, //1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
			0x90, 0x90, 0xF0, 0x10, 0x10, //4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
			0xF0, 0x10, 0x20, 0x40, 0x40, //7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
			0xF0, 0x90, 0xF0, 0x90, 0x90, //A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
			0xF0, 0x80, 0x80, 0x80, 0xF0, //C
			0xE0, 0x90, 0x90, 0x90, 0xE0, //D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
			0xF0, 0x80, 0xF0, 0x80, 0x80  //F
	};

	class chip8 {
	private:
		struct instructions {
			// 00E0: clear the screen
			INSTRUCTION(00E0) {
				for (unsigned char &i : c.gfx) {
					i = 0x0;
				}
				c.draw = true;
				c.next_instruction();
			};

			// 00EE: returns from subroutine
			INSTRUCTION(00EE) {
				--c.sp;
				c.pc = c.stack[c.sp];
				c.next_instruction();
			}

			// Jumps to address NNN.
			INSTRUCTION(1NNN) {
				c.pc = c.opcode & 0x0FFFu;
			}

			// Calls subroutine at NNN
			INSTRUCTION(2NNN) {
				c.stack[c.sp] = c.pc;
				++c.sp;
				c.pc = c.opcode & 0x0FFFu;
			}

			// Skips the next instruction if VX equals NN.
			// (Usually the next instruction is a jump to skip a code block)
			INSTRUCTION(3XNN) {
				if (c.V[((c.opcode & 0x0F00u) >> 8u)] == (c.opcode & 0x00FFu)) {
					c.next_instruction();
				}
				c.next_instruction();
			}

			// Skips the next instruction if VX doesn't equal NN.
			// (Usually the next instruction is a jump to skip a code block)
			INSTRUCTION(4XNN) {
				if (c.V[((c.opcode & 0x0F00u) >> 8u)] != (c.opcode & 0x00FFu)) {
					c.next_instruction();
				}
				c.next_instruction();
			}

			// Skips the next instruction if VX equals VY.
			// (Usually the next instruction is a jump to skip a code block)
			INSTRUCTION(5XY0) {
				if (c.V[(c.opcode & 0x0F00u) >> 8u] == c.V[(c.opcode & 0x00F0u) >> 4u]) {
					c.next_instruction();
				}
				c.next_instruction();
			}

			// Sets VX to NN.
			INSTRUCTION(6XNN) {
				c.V[(c.opcode & 0x0F00u) >> 8u] = c.opcode & 0x00FFu;
				c.next_instruction();
			}

			// Adds NN to VX. (Carry flag is not changed)
			INSTRUCTION(7XNN) {
				c.V[(c.opcode & 0x0F00u) >> 8u] += c.opcode & 0x00FFu;
				c.next_instruction();
			}

			// Sets VX to the value of VY.
			INSTRUCTION(8XY0) {
				c.V[(c.opcode & 0x0F00u) >> 8u] = c.V[(c.opcode & 0x00F0u) >> 4u];
				c.next_instruction();
			}

			// Sets VX to VX or VY. (Bitwise OR operation)
			INSTRUCTION(8XY1) {
				c.V[(c.opcode & 0x0F00u) >> 8u] |= c.V[(c.opcode & 0x00F0u) >> 4u];
				c.next_instruction();
			}

			// Sets VX to VX and VY. (Bitwise AND operation)
			INSTRUCTION(8XY2) {
				c.V[(c.opcode & 0x0F00u) >> 8u] &= c.V[(c.opcode & 0x00F0u) >> 4u];
				c.next_instruction();
			}

			// Sets VX to VX xor VY.
			INSTRUCTION(8XY3) {
				c.V[(c.opcode & 0x0F00u) >> 8u] ^= c.V[(c.opcode & 0x00F0u) >> 4u];
				c.next_instruction();
			}

			// Adds VY to VX.
			// VF is set to 1 when there's a carry, and to 0 when there isn't.
			INSTRUCTION(8XY4) {
				if (c.V[(c.opcode & 0x00F0u) >> 4u] > (0xFFu - c.V[(c.opcode & 0x0F00u) >> 8u])) {
					c.V[0xF] = 1;
				} else {
					c.V[0xF] = 0;
				}
				c.V[(c.opcode & 0x0F00u) >> 8u] += c.V[(c.opcode & 0x00F0u) >> 4u];
				c.next_instruction();
			}

			// VY is subtracted from VX.
			// VF is set to 0 when there's a borrow, and 1 when there isn't.
			INSTRUCTION(8XY5) {
				if (c.V[(c.opcode & 0x00F0u) >> 4u] > c.V[(c.opcode & 0x0F00u) >> 8u]) {
					c.V[0xF] = 0;
				} else {
					c.V[0xF] = 1;
				}
				c.V[(c.opcode & 0x0F00u) >> 8u] -= c.V[(c.opcode & 0x00F0u) >> 4u];
				c.next_instruction();
			}

			// Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
			INSTRUCTION(8XY6) {
				c.V[0xF] = c.V[(c.opcode & 0x0F00u) >> 8u] & 0x1u;
				c.V[(c.opcode & 0x0F00u) >> 8u] >>= 1u;
				c.next_instruction();
			}

			// Sets VX to VY minus VX.
			// VF is set to 0 when there's a borrow, and 1 when there isn't.
			INSTRUCTION(8XY7) {
				if (c.V[(c.opcode & 0x0F00u) >> 8u] > c.V[(c.opcode & 0x00F0u) >> 4u]) {
					c.V[0xF] = 0; // there is a borrow
				} else {
					c.V[0xF] = 1;
				}

				c.V[(c.opcode & 0x0F00u) >> 8u] = c.V[(c.opcode & 0x00F0u) >> 4u] - c.V[(c.opcode & 0x0F00u) >> 8u];
				c.next_instruction();
			}

			// Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
			INSTRUCTION(8XYE) {
				c.V[0xF] = c.V[(c.opcode & 0x0F00u) >> 8u] >> 7u;
				c.V[(c.opcode & 0x0F00u) >> 8u] <<= 1u;
				c.next_instruction();
			}

			// Skips the next instruction if VX doesn't equal VY.
			// (Usually the next instruction is a jump to skip a code block)
			INSTRUCTION(9XY0) {
				if (c.V[(c.opcode & 0x0F00u) >> 8u] != c.V[(c.opcode & 0x00F0u) >> 4u]) {
					c.next_instruction();
				}
				c.next_instruction();
			}

			// Sets I to the address NNN.
			INSTRUCTION(ANNN) {
				c.I = c.opcode & 0x0FFFu;
				c.next_instruction();
			}

			// Jumps to the address NNN plus V0.
			INSTRUCTION(BNNN) {
				c.pc = (c.opcode & 0x0FFFu) + c.V[0];
			}

			// Sets VX to the result of a bitwise and operation on a random number
			// (Typically: 0 to 255) and NN.
			INSTRUCTION(CXNN) {
				c.V[(c.opcode & 0x0F00u) >> 8u] = (rand() % 0xFFu) & (c.opcode & 0x00FFu);
				c.next_instruction();
			}

			// Draws a sprite at coordinate (VX, VY)
			// that has a width of 8 pixels and a height of N pixels.
			// Each row of 8 pixels is read as bit-coded starting from memory location I;
			// I value doesn’t change after the execution of this instruction.
			// As described above VF is set
			// to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
			// and to 0 if that doesn’t happen
			INSTRUCTION(DXYN) {
				unsigned short x = c.V[(c.opcode & 0x0F00u) >> 8u];
				unsigned short y = c.V[(c.opcode & 0x00F0u) >> 4u];
				unsigned short height = c.opcode & 0x000Fu;
				unsigned short pixel;

				c.V[0xF] = 0;
				for (unsigned int yline = 0; yline < height; yline++) {
					pixel = c.memory[c.I + yline];
					for (unsigned int xline = 0; xline < 8; xline++) {
						if ((pixel & (0x80u >> xline)) != 0) {
							if (c.gfx[(x + xline + ((y + yline) * DISPLAY_WIDTH))] == 1) {
								c.V[0xF] = 1;
							}
							c.gfx[x + xline + ((y + yline) * DISPLAY_WIDTH)] ^= 1u;
						}
					}
				}

				c.draw = true;
				c.next_instruction();
			}

			// Skips the next instruction if the key stored in VX is pressed.
			// (Usually the next instruction is a jump to skip a code block)
			INSTRUCTION(EX9E) {
				c.next_instruction();
				if (c.key[c.V[(c.opcode & 0x0F00u) >> 8u]] != 0) {
					c.next_instruction();
				}
			}

			// Skips the next instruction if the key stored in VX isn't pressed.
			// (Usually the next instruction is a jump to skip a code block)
			INSTRUCTION(EXA1) {
				c.next_instruction();
				if (c.key[c.V[(c.opcode & 0x0F00u) >> 8u]] == 0) {
					c.next_instruction();
				}
			}

			// Sets VX to the value of the delay timer.
			INSTRUCTION(FX07) {
				c.V[(c.opcode & 0x0F00u) >> 8u] = c.delay_timer;
				c.next_instruction();
			}

			// A key press is awaited, and then stored in VX.
			// (Blocking Operation. All instruction halted until next key event)
			INSTRUCTION(FX0A) {
				bool keyPress = false;

				for (int i = 0; i < 16; ++i) {
					if (c.key[i] != 0) {
						c.V[(c.opcode & 0x0F00u) >> 8u] = i;
						keyPress = true;
					}
				}

				// If we didn't received a keypress, skip this cycle and try again.
				if (!keyPress) {
					return;
				}

				c.next_instruction();
			}

			// Sets the delay timer to VX.
			INSTRUCTION(FX15) {
				c.delay_timer = c.V[(c.opcode & 0x0F00u) >> 8u];
				c.next_instruction();
			}

			// Sets the sound timer to VX.
			INSTRUCTION(FX18) {
				c.sound_timer = c.V[(c.opcode & 0x0F00u) >> 8u];
				c.next_instruction();
			}

			// Adds VX to I. VF is not affected.
			INSTRUCTION(FX1E) {
				if (c.I + c.V[(c.opcode & 0x0F00u) >> 8u] > 0xFFFu) {
					c.V[0xF] = 1;
				} else {
					c.V[0xF] = 0;
				}
				c.I += c.V[(c.opcode & 0x0F00u) >> 8u];
				c.next_instruction();
			}

			// Sets I to the location of the sprite for the character in VX.
			// Characters 0-F (in hexadecimal) are represented by a 4x5 font.
			INSTRUCTION(FX29) {
				c.I = c.V[(c.opcode & 0x0F00u) >> 8u] * 0x5u;
				c.next_instruction();
			}

			// Stores the binary-coded decimal representation of VX,
			// with the most significant of three digits at the address in I,
			// the middle digit at I plus 1,
			// and the least significant digit at I plus 2.
			// (In other words, take the decimal representation of VX,
			// place the hundreds digit in memory at location in I,
			// the tens digit at location I + 1, and the ones digit at location I + 2.)
			INSTRUCTION(FX33) {
				c.memory[c.I] = c.V[(c.opcode & 0x0F00u) >> 8u] / 100;
				c.memory[c.I + 1] = (c.V[(c.opcode & 0x0F00u) >> 8u] / 10) % 10;
				c.memory[c.I + 2] = (c.V[(c.opcode & 0x0F00u) >> 8u] % 100) % 10;
				c.next_instruction();
			}

			// Stores V0 to VX (including VX) in memory starting at address I.
			// The offset from I is increased by 1 for each value written,
			// but I itself is left unmodified.
			INSTRUCTION(FX55) {
				for (int i = 0; i <= ((c.opcode & 0x0F00u) >> 8u); ++i) {
					c.memory[c.I + i] = c.V[i];
				}

				// On the original interpreter, when the operation is done, I = I + X + 1.
				c.I += ((c.opcode & 0x0F00u) >> 8u) + 1;
				c.next_instruction();
			}

			// Fills V0 to VX (including VX) with values from memory starting at address I.
			// The offset from I is increased by 1 for each value written,
			// but I itself is left unmodified.
			INSTRUCTION(FX65) {
				for (int i = 0; i <= ((c.opcode & 0x0F00u) >> 8u); ++i) {
					c.V[i] = c.memory[c.I + i];
				}

				// On the original interpreter, when the operation is done, I = I + X + 1.
				c.I += ((c.opcode & 0x0F00u) >> 8u) + 1;
				c.next_instruction();
			}
		};

	public:
		chip8();
		~chip8();

		bool draw;

		void cycle() noexcept;
		bool load_game(const char *filename);

		unsigned char gfx[CHIP8_DISPLAY_SIZE_DEFAULT];  // 64x32 display
		unsigned char key[16];       // HEX-based keypad


	private:
		void init() noexcept;

		uint16_t opcode;             // 35 opcodes
		unsigned char memory[4096];  // 4K memory
		unsigned char V[16];         // 15 8-bit registers (V0 - VE)

		uint16_t I;                  // Index register
		uint16_t pc;                 // Program Counter


		// Interrupts and hardware registers
		unsigned char delay_timer;
		unsigned char sound_timer;

		uint16_t stack[16];          // 16 levels of stack
		uint16_t sp;                 // Stack pointer

		void next_instruction() noexcept;
		void unknown_opcode_error() const noexcept;
	};
}

#endif //CHIP_8_CHIP8
