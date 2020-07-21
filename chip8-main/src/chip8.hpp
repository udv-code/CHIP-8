// Copyright (c) 2020 udv. All rights reserved.

#ifndef CHIP8
#define CHIP8

#include <cstdint>

#define CHIP8_DISPLAY_WIDTH_DEFAULT 64
#define CHIP8_DISPLAY_HEIGHT_DEFAULT 32
#define CHIP8_DISPLAY_SIZE_DEFAULT CHIP8_DISPLAY_WIDTH_DEFAULT * CHIP8_DISPLAY_HEIGHT_DEFAULT

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
	public:
		chip8();
		~chip8();

		bool draw;

		void cycle();
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
		void unknown_opcode_error() const noexcept;
		void next_instruction() noexcept;

		static int random_number() noexcept;
	};
}

#endif //CHIP_8_CHIP8
