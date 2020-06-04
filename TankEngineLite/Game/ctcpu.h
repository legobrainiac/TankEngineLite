#ifndef CTCPU_H
#define CTCPU_H

#include <array>

constexpr int8_t REG_A = 0x00;
constexpr int8_t REG_B = 0x01;
constexpr int8_t REG_C = 0x02;

struct op
{
	constexpr op(int8_t _op, int8_t _o1, int8_t _o2) : opCode(_op), o1(_o1), o2(_o2) {}
	constexpr op(int8_t _op, int8_t _o1) : opCode(_op), o1(_o1), o2(0x00) {}
	constexpr op(int8_t _op) : opCode(_op), o1(0x00), o2(0x00) {}

	int8_t opCode, o1, o2;
};

struct cpu
{
	struct memory
	{
		int8_t registers[3]{};
		int8_t ram[128]{};
		struct
		{
			int8_t carry : 1;
			int8_t hlt : 1;
		}flags;
	}memory;

	 uint8_t pc = 0U;
};

//////////////////////////////////////////////////////////////////////////
// Method:		step
// FullName:	cpu::step(const std::array<op, S>& ops, cpu c)
// Description: Runs the next operation in ops using PC
// Access:		public 
// Returns:		constexpr cpu
// Qualifier: 
// Parameter:	const std::array<op, S>&
// Parameter:	cpu c
template<long unsigned int S>
constexpr cpu step(const std::array<op, S>& ops, cpu c)
{
	constexpr std::array<void(*)(int8_t, int8_t, cpu*), 256> op_solvers
	{
		[](int8_t, int8_t, cpu* c) constexpr { c->memory.registers[REG_C] = c->memory.registers[REG_A] + c->memory.registers[REG_B]; }, // 0x00, ADD => REG_C = REG_A + REG_B
		[](int8_t, int8_t, cpu* c) constexpr { c->memory.registers[REG_C] = c->memory.registers[REG_A] + c->memory.registers[REG_B]; }, // 0x01, SUB => REG_C = REG_A - REG_B
		[](int8_t o1, int8_t, cpu* c) constexpr { c->pc = (uint8_t)o1; }, // 0x02, JMP OA => PC = OA
		[](int8_t o1, int8_t, cpu* c) constexpr { if (c->memory.registers[REG_C] != 0U) c->pc = (uint8_t)o1; }, // 0x03, JNZ OA => PC = OA if REG_C != 0
		[](int8_t o1, int8_t, cpu* c) constexpr { if (c->memory.registers[REG_C] == 0U) c->pc = (uint8_t)o1; }, // 0x04, JZ OA => PC = OA if REG_C == 0
		[](int8_t, int8_t, cpu* c) constexpr { c->memory.registers[REG_C] = (c->memory.registers[REG_A] == c->memory.registers[REG_B]) ? 0x00 : 0x01; }, // 0x05, CMP => REG_C = REG_A == REG_B = 0 : 1
		[](int8_t o1, int8_t o2, cpu* c) constexpr { c->memory.ram[o2] = c->memory.registers[o1]; }, // 0x06, STO OA OB => RAM[OB] = OA
		[](int8_t o1, int8_t o2, cpu* c) constexpr { c->memory.registers[o1] = o2; }, // 0x07, MOV OA OB => REG[OA] = OB
		[](int8_t o1, int8_t, cpu* c) constexpr { c->memory.registers[o1]++; }, // 0x08, INC OA OB => REG[OA]++
		[](int8_t o1, int8_t, cpu* c) constexpr { c->memory.registers[o1]--; }, // 0x09, DEC OA OB => REG[OA]--
		[](int8_t o1, int8_t o2, cpu* c) constexpr { c->memory.ram[c->memory.registers[o1]] = c->memory.registers[o2]; }, // 0x0A, STA OA OB => RAM[REG[OA]] = REG[OB]
	};

	if (c.pc < ops.size())
	{
		const auto o = ops[c.pc++];
		op_solvers[o.opCode](o.o1, o.o2, &c);
	}

	return c;
}

//////////////////////////////////////////////////////////////////////////
// Method:		run
// FullName:	cpu::run(const std::array<op, S>& ops, cpu c)
// Description: Runs all the operations in ops
// Access:		public 
// Returns:		constexpr cpu
// Qualifier: 
// Parameter:	const std::array<op, S>&
// Parameter:	cpu c
template<long unsigned int S>
constexpr cpu run(const std::array<op, S>& ops, cpu c)
{
	constexpr std::array<void(*)(int8_t, int8_t, cpu*), 256> op_solvers
	{
		[](int8_t, int8_t, cpu* c) constexpr { c->memory.registers[REG_C] = c->memory.registers[REG_A] + c->memory.registers[REG_B]; }, // 0x00, ADD => REG_C = REG_A + REG_B
		[](int8_t, int8_t, cpu* c) constexpr { c->memory.registers[REG_C] = c->memory.registers[REG_A] + c->memory.registers[REG_B]; }, // 0x01, SUB => REG_C = REG_A - REG_B
		[](int8_t o1, int8_t, cpu* c) constexpr { c->pc = (uint8_t)o1; }, // 0x02, JMP OA => PC = OA
		[](int8_t o1, int8_t, cpu* c) constexpr { if (c->memory.registers[REG_C] != 0U) c->pc = (uint8_t)o1; }, // 0x03, JNZ OA => PC = OA if REG_C != 0
		[](int8_t o1, int8_t, cpu* c) constexpr { if (c->memory.registers[REG_C] == 0U) c->pc = (uint8_t)o1; }, // 0x04, JZ OA => PC = OA if REG_C == 0
		[](int8_t, int8_t, cpu* c) constexpr { c->memory.registers[REG_C] = (c->memory.registers[REG_A] == c->memory.registers[REG_B]) ? 0x00 : 0x01; }, // 0x05, CMP => REG_C = REG_A == REG_B = 0 : 1
		[](int8_t o1, int8_t o2, cpu* c) constexpr { c->memory.ram[o2] = c->memory.registers[o1]; }, // 0x06, STO OA OB => RAM[OB] = OA
		[](int8_t o1, int8_t o2, cpu* c) constexpr { c->memory.registers[o1] = o2; }, // 0x07, MOV OA OB => REG[OA] = OB
		[](int8_t o1, int8_t, cpu* c) constexpr { c->memory.registers[o1]++; }, // 0x08, INC OA OB => REG[OA]++
		[](int8_t o1, int8_t, cpu* c) constexpr { c->memory.registers[o1]--; }, // 0x09, DEC OA OB => REG[OA]--
		[](int8_t o1, int8_t o2, cpu* c) constexpr { c->memory.ram[c->memory.registers[o1]] = c->memory.registers[o2]; }, // 0x0A, STA OA OB => RAM[REG[OA]] = REG[OB]
	};

	while (c.pc < ops.size())
	{
		const auto o = ops[c.pc++];
		op_solvers[o.opCode](o.o1, o.o2, &c);
	}

	return c;
}

#endif // !CTCPU_H
