#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <array>
#include <memory>
#include <mutex>
#include <vector>

#include "VM.hpp"

static constexpr std::array<uint8_t, 256> genBitsSet() {
    std::array<uint8_t, 256> vals{};
    for (int i = 0; i < 256; i++) {
        int count = 0;
        for (int j = 0; j < 8; j++) {
            if (i & (1 << j))
                count++;
        }
        vals[i] = count;
    }
    return vals;
}

static const constexpr std::array<uint8_t, 256> cpuBitsSet = genBitsSet();

struct VMState {
    union reg_pair {
	struct
	{
#ifdef WORDS_BIGENDIAN
	    uint8_t B3;
	    uint8_t B2;
	    uint8_t B1;
	    uint8_t B0;
#else
	    uint8_t B0;
	    uint8_t B1;
	    uint8_t B2;
	    uint8_t B3;
#endif
	} B;
	struct
	{
#ifdef WORDS_BIGENDIAN
	    uint16_t W1;
	    uint16_t W0;
#else
	    uint16_t W0;
	    uint16_t W1;
#endif
	} W;
#ifdef WORDS_BIGENDIAN
	volatile uint32_t I;
#else
	uint32_t I;
#endif
    };

    std::size_t speed;
    bool crashed = true;
    std::size_t ramSize;
    std::size_t extendedRamSize;
    std::size_t maxRAMAddr;
    uint8_t* RAM;
    VM* vm;

    reg_pair reg[45];
    bool N_FLAG = false;
    bool C_FLAG = false;
    bool Z_FLAG = false;
    bool V_FLAG = false;
    uint32_t armNextPC = 0;
    uint32_t cpuPrefetch[2];

    void prefetch() {
	cpuPrefetch[0] = exec16(armNextPC);
	cpuPrefetch[1] = exec16(armNextPC + 2);
    }

    void prefetchNext() {
	cpuPrefetch[1] = exec16(armNextPC+2);
    }

    void thumbUI(uint32_t opcode) {
	printf("UNKNOWN OP %x\n", opcode);
    }

// Common macros //////////////////////////////////////////////////////////

    static inline uint32_t NEG(const uint32_t i) {
	return i >> 31;
    }
    static inline uint32_t POS(const uint32_t i) {
	return (~i) >> 31;
    }


// C core

    static inline bool ADDCARRY(const uint32_t a, const uint32_t b, const uint32_t c) {
	return	(NEG(a) & NEG(b)) |
	    (NEG(a) & POS(c)) |
	    (NEG(b) & POS(c));
    }

    static inline bool ADDOVERFLOW(const uint32_t a, const uint32_t b, const uint32_t c) {
	return	(NEG(a) & NEG(b) & POS(c)) |
	    (POS(a) & POS(b) & NEG(c));
    }

    static inline bool SUBCARRY(const uint32_t a, const uint32_t b, const uint32_t c) {
	return	(NEG(a) & POS(b)) |
	    (NEG(a) & POS(c)) |
	    (POS(b) & POS(c));
    }

    static inline bool SUBOVERFLOW(const uint32_t a, const uint32_t b, const uint32_t c) {
	return	(NEG(a) & POS(b) & POS(c)) |
	    (POS(a) & NEG(b) & NEG(c));
    }

// 3-argument ADD/SUB /////////////////////////////////////////////////////

// ADD Rd, Rs, Rn
    template <int N>
    void thumb18(uint32_t opcode) {
	int dest = opcode & 0x07;
	int source = (opcode >> 3) & 0x07;
	uint32_t lhs = reg[source].I;
        uint32_t rhs = reg[N].I;
	uint32_t res = lhs + rhs;
	reg[dest].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = ADDCARRY(lhs, rhs, res);
	V_FLAG = ADDOVERFLOW(lhs, rhs, res);
    }

// SUB Rd, Rs, Rn
    template <int N>
    void thumb1A(uint32_t opcode) {
	int dest = opcode & 0x07;
	int source = (opcode >> 3) & 0x07;
	uint32_t lhs = reg[source].I;
	uint32_t rhs = reg[N].I;
	uint32_t res = lhs - rhs;
	reg[dest].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = SUBCARRY(lhs, rhs, res);
	V_FLAG = SUBOVERFLOW(lhs, rhs, res);
    }

// ADD Rd, Rs, #Offset3
    template <int N>
    void thumb1C(uint32_t opcode) {
	int dest = opcode & 0x07;
	int source = (opcode >> 3) & 0x07;
	uint32_t lhs = reg[source].I;
	uint32_t rhs = N;
	uint32_t res = lhs + rhs;
	reg[dest].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = ADDCARRY(lhs, rhs, res);
	V_FLAG = ADDOVERFLOW(lhs, rhs, res);
    }

// SUB Rd, Rs, #Offset3
    template <int N>
    void thumb1E(uint32_t opcode) {
	int dest = opcode & 0x07;
	int source = (opcode >> 3) & 0x07;
	uint32_t lhs = reg[source].I;
	uint32_t rhs = N;
	uint32_t res = lhs - rhs;
	reg[dest].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = SUBCARRY(lhs, rhs, res);
	V_FLAG = SUBOVERFLOW(lhs, rhs, res);
    }

// Shift instructions /////////////////////////////////////////////////////

// LSL Rd, Rm, #Imm 5
    template <int N>
    void thumb00(uint32_t opcode) {
        if constexpr (N != 0) {
            int dest = opcode & 0x07;
            int source = (opcode >> 3) & 0x07;
            uint32_t value;
            int shift = N;
            C_FLAG = (reg[source].I >> (32 - shift)) & 1 ? true : false;
            value = reg[source].I << shift;
            reg[dest].I = value;
            N_FLAG = (value & 0x80000000 ? true : false);
            Z_FLAG = (value ? false : true);
        } else {
            int dest = opcode & 0x07;
            int source = (opcode >> 3) & 0x07;
            uint32_t value = reg[(source)].I;
            reg[dest].I = value;
            N_FLAG = (value & 0x80000000 ? true : false);
            Z_FLAG = (value ? false : true);
        }
    }

// LSR Rd, Rm, #Imm 5
    template <int N>
    void thumb08(uint32_t opcode) {
        if constexpr (N == 0) {
            int dest = opcode & 0x07;
            int source = (opcode >> 3) & 0x07;
            uint32_t value = 0;
            C_FLAG = reg[(source)].I & 0x80000000 ? true : false;
            reg[dest].I = value;
            N_FLAG = (value & 0x80000000 ? true : false);
            Z_FLAG = (value ? false : true);
        } else {
            int dest = opcode & 0x07;
            int source = (opcode >> 3) & 0x07;
            uint32_t value;
            int shift = N;
            C_FLAG = (reg[source].I >> (shift - 1)) & 1 ? true : false;
            value = reg[(source)].I >> shift;
            reg[dest].I = value;
            N_FLAG = (value & 0x80000000 ? true : false);
            Z_FLAG = (value ? false : true);
        }
    }

// ASR Rd, Rm, #Imm 5
    template <int N>
    void thumb10(uint32_t opcode) {
        if constexpr (N == 0) {
            int dest = opcode & 0x07;
            int source = (opcode >> 3) & 0x07;
            uint32_t value;
            if (reg[(source)].I & 0x80000000)
            {
                value = 0xFFFFFFFF;
                C_FLAG = true;
            }
            else
            {
                value = 0;
                C_FLAG = false;
            }
            reg[dest].I = value;
            N_FLAG = (value & 0x80000000 ? true : false);
            Z_FLAG = (value ? false : true);
        } else {
            int dest = opcode & 0x07;
            int source = (opcode >> 3) & 0x07;
            uint32_t value;
            int shift = N;
            C_FLAG = ((int32_t)reg[source].I >> (int)(shift - 1)) & 1 ? true : false;
            value = (int32_t)reg[(source)].I >> (int)shift;
            reg[dest].I = value;
            N_FLAG = (value & 0x80000000 ? true : false);
            Z_FLAG = (value ? false : true);
        }
    }

// MOV/CMP/ADD/SUB immediate //////////////////////////////////////////////

// MOV RN, #Offset8
    template <int N>
    void thumb20(uint32_t opcode) {
	reg[N].I = opcode & 255;
	N_FLAG = false;
	Z_FLAG = (reg[N].I ? false : true);
    }

// CMP RN, #Offset8
    template <int N>
    void thumb28(uint32_t opcode) {
	uint32_t lhs = reg[(N)].I;
	uint32_t rhs = (opcode & 255);
	uint32_t res = lhs - rhs;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = SUBCARRY(lhs, rhs, res);
	V_FLAG = SUBOVERFLOW(lhs, rhs, res);
    }

// ADD RN,#Offset8
    template <int N>
    void thumb30(uint32_t opcode) {
	uint32_t lhs = reg[(N)].I;
	uint32_t rhs = (opcode & 255);
	uint32_t res = lhs + rhs;
	reg[N].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = ADDCARRY(lhs, rhs, res);
	V_FLAG = ADDOVERFLOW(lhs, rhs, res);
    }

// SUB RN,#Offset8
    template <int N>
    void thumb38(uint32_t opcode) {
	uint32_t lhs = reg[(N)].I;
	uint32_t rhs = (opcode & 255);
	uint32_t res = lhs - rhs;
	reg[N].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = SUBCARRY(lhs, rhs, res);
	V_FLAG = SUBOVERFLOW(lhs, rhs, res);
    }

// ALU operations /////////////////////////////////////////////////////////

    inline void CMP_RD_RS(int dest, uint32_t value) {
	uint32_t lhs = reg[(dest)].I;
	uint32_t rhs = value;
	uint32_t res = lhs - rhs;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = SUBCARRY(lhs, rhs, res);
	V_FLAG = SUBOVERFLOW(lhs, rhs, res);
    }

// AND Rd, Rs
    void thumb40_0(uint32_t opcode) {
	int dest = opcode & 7;
	reg[(dest)].I &= reg[((opcode >> 3)&7)].I;
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	Z_FLAG = reg[dest].I ? false : true;
    }

// EOR Rd, Rs
    void thumb40_1(uint32_t opcode) {
	int dest = opcode & 7;
	reg[dest].I ^= reg[((opcode >> 3)&7)].I;
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	Z_FLAG = reg[dest].I ? false : true;
    }

// LSL Rd, Rs
    void thumb40_2(uint32_t opcode) {
	int dest = opcode & 7;
	uint32_t value = reg[((opcode >> 3)&7)].B.B0;
	if (value)
	{
	    if (value == 32) {
		value = 0;
		C_FLAG = (reg[dest].I & 1 ? true : false);
	    }
	    else if (value < 32) {
		C_FLAG = (reg[dest].I >> (32 - value)) & 1 ? true : false;
		value = reg[dest].I << value;
	    }
	    else {
		value = 0;
		C_FLAG = false;
	    }
	    reg[dest].I = value;
	}
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	Z_FLAG = reg[(dest)].I ? false : true;
    }

// LSR Rd, Rs
    void thumb40_3(uint32_t opcode) {
	int dest = opcode & 7;
	uint32_t value = reg[((opcode >> 3)&7)].B.B0;
	if (value)
	{
	    if (value == 32) {
		value = 0;
		C_FLAG = (reg[dest].I & 0x80000000 ? true : false);
	    }
	    else if (value < 32) {
		C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false;
		value = reg[dest].I >> value;
	    }
	    else {
		value = 0;
		C_FLAG = false;
	    }
	    reg[dest].I = value;
	}
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	Z_FLAG = reg[(dest)].I ? false : true;
    }

// ASR Rd, Rs
    void thumb41_0(uint32_t opcode) {
	int dest = opcode & 7;
	uint32_t value = reg[((opcode >> 3)&7)].B.B0;
	if (value)
	{
	    if (value < 32) {
		C_FLAG = ((int32_t)reg[dest].I >> (int)(value - 1)) & 1 ? true : false;
		value = (int32_t)reg[dest].I >> (int)value;
		reg[dest].I = value;
	    }
	    else {
		if (reg[dest].I & 0x80000000)
		{
		    reg[dest].I = 0xFFFFFFFF;
		    C_FLAG = true;
		}
		else
		{
		    reg[dest].I = 0x00000000;
		    C_FLAG = false;
		}
	    }
	}
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
	Z_FLAG = reg[(dest)].I ? false : true;
    }

// ADC Rd, Rs
    void thumb41_1(uint32_t opcode) {
	int dest = opcode & 0x07;
	uint32_t value = reg[((opcode >> 3)&7)].I;
	uint32_t lhs = reg[(dest)].I;
	uint32_t rhs = value;
	uint32_t res = lhs + rhs + (uint32_t)C_FLAG;
	reg[dest].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = ADDCARRY(lhs, rhs, res);
	V_FLAG = ADDOVERFLOW(lhs, rhs, res);
    }

// SBC Rd, Rs
    void thumb41_2(uint32_t opcode) {
	int dest = opcode & 0x07;
	uint32_t value = reg[((opcode >> 3)&7)].I;
	uint32_t lhs = reg[(dest)].I;
	uint32_t rhs = value;
	uint32_t res = lhs - rhs - !((uint32_t)C_FLAG);
	reg[dest].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = SUBCARRY(lhs, rhs, res);
	V_FLAG = SUBOVERFLOW(lhs, rhs, res);
    }

// ROR Rd, Rs
    void thumb41_3(uint32_t opcode) {
	int dest = opcode & 7;
	uint32_t value = reg[((opcode >> 3)&7)].B.B0;

	if (value)
	{
	    value = value & 0x1f;
	    if (value == 0) {
		C_FLAG = (reg[dest].I & 0x80000000 ? true : false);
	    }
	    else {
		C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false;
		value = ((reg[dest].I << (32 - value)) |
			 (reg[dest].I >> value));
		reg[dest].I = value;
	    }
	}
	N_FLAG = reg[(dest)].I & 0x80000000 ? true : false;
	Z_FLAG = reg[dest].I ? false : true;
    }

// TST Rd, Rs
    void thumb42_0(uint32_t opcode) {
	uint32_t value = reg[(opcode & 7)].I & reg[((opcode >> 3) & 7)].I;
	N_FLAG = value & 0x80000000 ? true : false;
	Z_FLAG = value ? false : true;
    }

// NEG Rd, Rs
    void thumb42_1(uint32_t opcode) {
	int dest = opcode & 7;
	int source = (opcode >> 3) & 7;
	uint32_t lhs = reg[(source)].I;
	uint32_t rhs = 0;
	uint32_t res = rhs - lhs;
	reg[dest].I = res;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = SUBCARRY(rhs, lhs, res);
	V_FLAG = SUBOVERFLOW(rhs, lhs, res);
    }

// CMP Rd, Rs
    void thumb42_2(uint32_t opcode) {
	int dest = opcode & 7;
	uint32_t value = reg[((opcode >> 3)&7)].I;
        (dest);
	CMP_RD_RS(dest, value);
    }

// CMN Rd, Rs
    void thumb42_3(uint32_t opcode) {
	int dest = opcode & 7;
	uint32_t value = reg[((opcode >> 3)&7)].I;
	uint32_t lhs = reg[(dest)].I;
	uint32_t rhs = value;
	uint32_t res = lhs + rhs;
	Z_FLAG = (res == 0) ? true : false;
	N_FLAG = NEG(res) ? true : false;
	C_FLAG = ADDCARRY(lhs, rhs, res);
	V_FLAG = ADDOVERFLOW(lhs, rhs, res);
    }

// ORR Rd, Rs
    void thumb43_0(uint32_t opcode) {
	int dest = opcode & 7;
	reg[(dest)].I |= reg[((opcode >> 3) & 7)].I;
	Z_FLAG = reg[dest].I ? false : true;
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
    }

// MUL Rd, Rs
    void thumb43_1(uint32_t opcode) {
	int dest = opcode & 7;
	uint32_t rm = reg[(dest)].I;
	reg[(dest)].I = reg[(opcode >> 3) & 7].I * rm;
	if (((int32_t)rm) < 0)
	    rm = ~rm;
	Z_FLAG = reg[dest].I ? false : true;
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
    }

// BIC Rd, Rs
    void thumb43_2(uint32_t opcode) {
	int dest = opcode & 7;
	reg[(dest)].I &= ~reg[((opcode >> 3) & 7)].I;
	Z_FLAG = reg[dest].I ? false : true;
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
    }

// MVN Rd, Rs
    void thumb43_3(uint32_t opcode) {
	int dest = opcode & 7;
	reg[dest].I = ~reg[((opcode >> 3) & 7)].I;
	Z_FLAG = reg[dest].I ? false : true;
	N_FLAG = reg[dest].I & 0x80000000 ? true : false;
    }

// High-register instructions and BX //////////////////////////////////////

// ADD Rd, Rs
    void thumb44_0(uint32_t opcode) {
	reg[(opcode&7)].I += reg[(((opcode>>3)&7))].I;
    }

// ADD Rd, Hs
    void thumb44_1(uint32_t opcode) {
        reg[(opcode&7)].I += reg[(((opcode>>3)&7)+8)].I;
    }

// ADD Hd, Rs
    void thumb44_2(uint32_t opcode) {
	reg[((opcode&7)+8)].I += reg[((opcode>>3)&7)].I;
	if ((opcode&7) == 7)
	{
	    reg[15].I &= 0xFFFFFFFE;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// ADD Hd, Hs
    void thumb44_3(uint32_t opcode) {
	reg[((opcode&7)+8)].I += reg[(((opcode>>3)&7)+8)].I;
	if ((opcode&7) == 7)
	{
	    reg[15].I &= 0xFFFFFFFE;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// CMP Rd, Hs
    void thumb45_1(uint32_t opcode) {
	int dest = opcode & 7;
	uint32_t value = reg[(((opcode>>3)&7)+8)].I;
	CMP_RD_RS(dest, value);
    }

// CMP Hd, Rs
    void thumb45_2(uint32_t opcode) {
	int dest = (opcode & 7) + 8;
	uint32_t value = reg[((opcode>>3)&7)].I;
	CMP_RD_RS(dest, value);
    }

// CMP Hd, Hs
    void thumb45_3(uint32_t opcode) {
	int dest = (opcode & 7) + 8;
	uint32_t value = reg[(((opcode>>3)&7)+8)].I;
	CMP_RD_RS(dest, value);
    }

// MOV Rd, Rs
    void thumb46_0(uint32_t opcode) {
	reg[opcode&7].I = reg[(((opcode>>3)&7))].I;
    }

// MOV Rd, Hs
    void thumb46_1(uint32_t opcode) {
	reg[opcode&7].I = reg[(((opcode>>3)&7)+8)].I;
    }

// MOV Hd, Rs
    void thumb46_2(uint32_t opcode) {
	reg[(opcode&7)+8].I = reg[((opcode>>3)&7)].I;
	if ((opcode&7) == 7)
	{
	    reg[15].I &= 0xFFFFFFFE;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// MOV Hd, Hs
    void thumb46_3(uint32_t opcode) {
	reg[(opcode&7)+8].I = reg[(((opcode>>3)&7)+8)].I;
	if ((opcode&7) == 7)
	{
	    reg[15].I &= 0xFFFFFFFE;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BX Rs
    void thumb47(uint32_t opcode) {
	int base = (opcode >> 3) & 15;
	reg[15].I = reg[(base)].I;
        reg[15].I &= 0xFFFFFFFE;
        armNextPC = reg[15].I;
        reg[15].I += 2;
        prefetch();
    }

// BLX Rs
    void thumb47b(uint32_t opcode) {
	int base = (opcode >> 3) & 15;

	if (reg[base].I >= ramSize) {
            auto call = (reg[base].I - ramSize) >> 2;
            // printf("API call: %d\n", int(call));
            if (call < vm->apiIndex.size()) {
                VM::Args args{this, vm};
                vm->apiIndex[call](args);
                switch (args.result.index()) {
                case 0:
                    break;
                case 1:
                    reg[0].I = std::get<int32_t>(args.result);
                    break;
                case 2:
                    reg[0].I = std::get<uint32_t>(args.result);
                    break;
                case 3:
                    reg[0].I = *reinterpret_cast<uint32_t*>(&std::get<float>(args.result));
                    break;
                case 4:
                    reg[0].I = std::get<bool>(args.result);
                    break;
                case 5:
                    reg[0].I = vm->toGuest(std::get<std::string>(args.result));
                    break;
                }
            }
	    return;
	}

	reg[14].I = reg[15].I-2;
	reg[15].I = reg[base].I & 0xFFFFFFFE;
	armNextPC = reg[15].I;
	reg[15].I += 2;
	prefetch();
    }

// Load/store instructions ////////////////////////////////////////////////

// LDR R0~R7,[PC, #Imm]
    void thumb48(uint32_t opcode) {
	uint8_t regist = (opcode >> 8) & 7;
	uint32_t address = (reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);
	reg[regist].I = read32(address);
    }

// STR Rd, [Rs, Rn]
    void thumb50(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + reg[((opcode>>6)&7)].I;
	write32(address, reg[opcode & 7].I);
    }

// STRH Rd, [Rs, Rn]
    void thumb52(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + reg[((opcode>>6)&7)].I;
	write16(address, reg[opcode&7].W.W0);
    }

// STRB Rd, [Rs, Rn]
    void thumb54(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + reg[((opcode >>6)&7)].I;
	write8(address, reg[opcode & 7].B.B0);
    }

// LDSB Rd, [Rs, Rn]
    void thumb56(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + reg[((opcode>>6)&7)].I;
	reg[opcode&7].I = (int8_t)read8(address);
    }

// LDR Rd, [Rs, Rn]
    void thumb58(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + reg[((opcode>>6)&7)].I;
	reg[opcode&7].I = read32(address);
    }

// LDRH Rd, [Rs, Rn]
    void thumb5A(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + reg[((opcode>>6)&7)].I;
	reg[opcode&7].I = read16(address);
    }

// LDRB Rd, [Rs, Rn]
    void thumb5C(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + reg[((opcode>>6)&7)].I;
	reg[opcode&7].I = read8(address);
    }

// LDSH Rd, [Rs, Rn]
    void thumb5E(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + reg[((opcode>>6)&7)].I;
	reg[opcode&7].I = (int16_t)read16s(address);
    }

// STR Rd, [Rs, #Imm]
    void thumb60(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + (((opcode>>6)&31)<<2);
	write32(address, reg[opcode&7].I);
    }

// LDR Rd, [Rs, #Imm]
    void thumb68(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + (((opcode>>6)&31)<<2);
	reg[opcode&7].I = read32(address);
    }

// STRB Rd, [Rs, #Imm]
    void thumb70(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + (((opcode>>6)&31));
	write8(address, reg[opcode&7].B.B0);
    }

// LDRB Rd, [Rs, #Imm]
    void thumb78(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + (((opcode>>6)&31));
	reg[opcode&7].I = read8(address);
    }

// STRH Rd, [Rs, #Imm]
    void thumb80(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + (((opcode>>6)&31)<<1);
	write16(address, reg[opcode&7].W.W0);
    }

// LDRH Rd, [Rs, #Imm]
    void thumb88(uint32_t opcode) {
	uint32_t address = reg[((opcode>>3)&7)].I + (((opcode>>6)&31)<<1);
	reg[opcode&7].I = read16(address);
    }

// STR R0~R7, [SP, #Imm]
    void thumb90(uint32_t opcode) {
	uint8_t regist = (opcode >> 8) & 7;
	uint32_t address = reg[13].I + ((opcode&255)<<2);
	write32(address, reg[regist].I);
    }

// LDR R0~R7, [SP, #Imm]
    void thumb98(uint32_t opcode) {
	uint8_t regist = (opcode >> 8) & 7;
	uint32_t address = reg[13].I + ((opcode&255)<<2);
	reg[regist].I = read32(address);
    }

// PC/stack-related ///////////////////////////////////////////////////////

// ADD R0~R7, PC, Imm
    void thumbA0(uint32_t opcode) {
	uint8_t regist = (opcode >> 8) & 7;
	reg[regist].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
    }

// ADD R0~R7, SP, Imm
    void thumbA8(uint32_t opcode) {
	uint8_t regist = (opcode >> 8) & 7;
	reg[regist].I = reg[13].I + ((opcode&255)<<2);
    }

// ADD SP, Imm
    void thumbB0(uint32_t opcode) {
	int offset = (opcode & 127) << 2;
	if (opcode & 0x80)
	    offset = -offset;
	reg[13].I += offset;
    }

// SXTH
    void thumbB2a(uint32_t opcode) {
	uint32_t src = reg[((opcode >> 3) & 7)].I;
	int dest = opcode & 7;
	int mask = 0xFFFF;
	int sign = (src & 0x8000) ? ~mask : 0;
	reg[dest].I = (mask & src) | sign;
    }

// SXTB
    void thumbB2b(uint32_t opcode) {
	uint32_t src = reg[((opcode >> 3) & 7)].I;
	int dest = opcode & 7;
	int mask = 0xFF;
	int sign = (src & 0x80) ? ~mask : 0;
	reg[dest].I = (mask & src) | sign;
    }

// UXTH R0~7
    void thumbB2c(uint32_t opcode) {
	uint32_t src = reg[((opcode >> 3) & 7)].I;
	int dest = opcode & 7;
	int mask = 0xFFFF;
	reg[dest].I = (mask & src);
    }

// UXTB
    void thumbB2d(uint32_t opcode) {
	uint32_t src = reg[((opcode >> 3) & 7)].I;
	int dest = opcode & 7;
	int mask = 0xFF;
	reg[dest].I = (mask & src);
    }

// Push and pop ///////////////////////////////////////////////////////////

     inline void PUSH_REG(uint32_t opcode, int &count, uint32_t &address, int val, int r) {
	if (opcode & val) {
	    write32(address, reg[(r)].I);
	    ++count;
	    address += 4;
	}
    }

     inline void POP_REG(uint32_t opcode, int &count, uint32_t &address, int val, int r) {
	if (opcode & val) {
	    reg[r].I = read32(address);
	    ++count;
	    address += 4;
	}
    }

// PUSH {Rlist}
    void thumbB4(uint32_t opcode) {
	int count = 0;
	uint32_t temp = reg[13].I - 4 * cpuBitsSet[opcode & 0xff];
	uint32_t address = temp & 0xFFFFFFFC;
	PUSH_REG(opcode, count, address, 1, 0);
	PUSH_REG(opcode, count, address, 2, 1);
	PUSH_REG(opcode, count, address, 4, 2);
	PUSH_REG(opcode, count, address, 8, 3);
	PUSH_REG(opcode, count, address, 16, 4);
	PUSH_REG(opcode, count, address, 32, 5);
	PUSH_REG(opcode, count, address, 64, 6);
	PUSH_REG(opcode, count, address, 128, 7);
	reg[13].I = temp;
    }

// PUSH {Rlist, LR}
    void thumbB5(uint32_t opcode) {
	int count = 0;
	uint32_t temp = reg[13].I - 4 - 4 * cpuBitsSet[opcode & 0xff];
	uint32_t address = temp & 0xFFFFFFFC;
	PUSH_REG(opcode, count, address, 1, 0);
	PUSH_REG(opcode, count, address, 2, 1);
	PUSH_REG(opcode, count, address, 4, 2);
	PUSH_REG(opcode, count, address, 8, 3);
	PUSH_REG(opcode, count, address, 16, 4);
	PUSH_REG(opcode, count, address, 32, 5);
	PUSH_REG(opcode, count, address, 64, 6);
	PUSH_REG(opcode, count, address, 128, 7);
	PUSH_REG(opcode, count, address, 256, 14);
	reg[13].I = temp;
    }

// CPSID / CPSIE
    void thumbB6(uint32_t opcode){
	// armIrqEnable = 1 ^ ((opcode>>4)&1);
    }

// REV rx
    void thumbBA(uint32_t opcode) {
	uint32_t src = reg[((opcode >> 3) & 7)].I;
	int dest = opcode & 7;
	reg[dest].I = (src<<24)
	    | (src>>24)
	    | ((src&0xFF00)<<8)
	    | ((src&0xFF0000)>>8);
    }

// REV16
    void thumbBAb(uint32_t opcode) {
	uint32_t src = reg[((opcode >> 3) & 7)].I;
	int dest = opcode & 7;
	reg[dest].I = ((src&0xFF00FF00)>>8)
	    | ((src&0x00FF00FF)<<8);
    }


// REVSH
    void thumbBAc(uint32_t opcode) {
	uint32_t src = reg[((opcode >> 3) & 7)].I;
	int dest = opcode & 7;
	uint32_t sign = src&0x8000 ? 0xFFFFFF : 0;
	reg[dest].I = ((src&0xFF00)>>8) | sign;
    }

// POP {Rlist}
    void thumbBC(uint32_t opcode) {
	int count = 0;
	uint32_t address = reg[13].I & 0xFFFFFFFC;
	uint32_t temp = reg[13].I + 4*cpuBitsSet[opcode & 0xFF];
	POP_REG(opcode, count, address, 1, 0);
	POP_REG(opcode, count, address, 2, 1);
	POP_REG(opcode, count, address, 4, 2);
	POP_REG(opcode, count, address, 8, 3);
	POP_REG(opcode, count, address, 16, 4);
	POP_REG(opcode, count, address, 32, 5);
	POP_REG(opcode, count, address, 64, 6);
	POP_REG(opcode, count, address, 128, 7);
	reg[13].I = temp;
    }

// POP {Rlist, PC}
    void thumbBD(uint32_t opcode) {
	int count = 0;
	uint32_t address = reg[13].I & 0xFFFFFFFC;
	uint32_t temp = reg[13].I + 4 + 4*cpuBitsSet[opcode & 0xFF];
	POP_REG(opcode, count, address, 1, 0);
	POP_REG(opcode, count, address, 2, 1);
	POP_REG(opcode, count, address, 4, 2);
	POP_REG(opcode, count, address, 8, 3);
	POP_REG(opcode, count, address, 16, 4);
	POP_REG(opcode, count, address, 32, 5);
	POP_REG(opcode, count, address, 64, 6);
	POP_REG(opcode, count, address, 128, 7);
	reg[15].I = (read32(address) & 0xFFFFFFFE);
	++count;
	armNextPC = reg[15].I;
	reg[15].I += 2;
	reg[13].I = temp;
	prefetch();
    }

// BKPT
    void thumbBE( uint32_t opcode ){
        static bool wasHit = false;
        if (!wasHit) {
            wasHit = true;
            printf("Breakpoint hit. Halting.\n");
            armNextPC -= 2;
            reg[15].I -= 2;
        }
    }

    // NOP
    void thumbBF( uint32_t opcode ){
    }

// Load/store multiple ////////////////////////////////////////////////////

    inline void THUMB_STM_REG(uint32_t opcode, int &count, uint32_t &address, uint32_t temp, int val, int r, int b) {
	if (opcode & val) {
	    write32(address, reg[r].I);
	    reg[b].I = temp;
	    ++count;
	    address += 4;
	}
    }

    inline void THUMB_LDM_REG(uint32_t opcode, int &count, uint32_t &address, int val, int r) {
	if (opcode & (val)) {
	    reg[r].I = read32(address);
	    ++count;
	    address += 4;
	}
    }

// STM R0~7!, {Rlist}
    void thumbC0(uint32_t opcode) {
	uint8_t regist = (opcode >> 8) & 7;
	uint32_t address = reg[regist].I & 0xFFFFFFFC;
	uint32_t temp = reg[regist].I + 4*cpuBitsSet[opcode & 0xff];
	int count = 0;
	// store
	THUMB_STM_REG(opcode, count, address, temp, 1, 0, regist);
	THUMB_STM_REG(opcode, count, address, temp, 2, 1, regist);
	THUMB_STM_REG(opcode, count, address, temp, 4, 2, regist);
	THUMB_STM_REG(opcode, count, address, temp, 8, 3, regist);
	THUMB_STM_REG(opcode, count, address, temp, 16, 4, regist);
	THUMB_STM_REG(opcode, count, address, temp, 32, 5, regist);
	THUMB_STM_REG(opcode, count, address, temp, 64, 6, regist);
	THUMB_STM_REG(opcode, count, address, temp, 128, 7, regist);
    }

// LDM R0~R7!, {Rlist}
    void thumbC8(uint32_t opcode) {
	uint8_t regist = (opcode >> 8) & 7;
	uint32_t address = reg[regist].I & 0xFFFFFFFC;
	int count = 0;
	THUMB_LDM_REG(opcode, count, address, 1, 0);
	THUMB_LDM_REG(opcode, count, address, 2, 1);
	THUMB_LDM_REG(opcode, count, address, 4, 2);
	THUMB_LDM_REG(opcode, count, address, 8, 3);
	THUMB_LDM_REG(opcode, count, address, 16, 4);
	THUMB_LDM_REG(opcode, count, address, 32, 5);
	THUMB_LDM_REG(opcode, count, address, 64, 6);
	THUMB_LDM_REG(opcode, count, address, 128, 7);
	if (!(opcode & (1<<regist)))
	    reg[regist].I = address;
    }

// Conditional branches ///////////////////////////////////////////////////

// BEQ offset
    void thumbD0(uint32_t opcode) {
	if (Z_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BNE offset
    void thumbD1(uint32_t opcode) {
	if (!Z_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BCS offset
    void thumbD2(uint32_t opcode) {
	if (C_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BCC offset
    void thumbD3(uint32_t opcode) {
	if (!C_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BMI offset
    void thumbD4(uint32_t opcode) {
	if (N_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BPL offset
    void thumbD5(uint32_t opcode) {
	if (!N_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BVS offset
    void thumbD6(uint32_t opcode) {
	if (V_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BVC offset
    void thumbD7(uint32_t opcode) {
	if (!V_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BHI offset
    void thumbD8(uint32_t opcode) {
	if (C_FLAG && !Z_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BLS offset
    void thumbD9(uint32_t opcode) {
	if (!C_FLAG || Z_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BGE offset
    void thumbDA(uint32_t opcode) {
	if (N_FLAG == V_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BLT offset
    void thumbDB(uint32_t opcode) {
	if (N_FLAG != V_FLAG) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BGT offset
    void thumbDC(uint32_t opcode) {
	if (!Z_FLAG && (N_FLAG == V_FLAG)) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// BLE offset
    void thumbDD(uint32_t opcode) {
	if (Z_FLAG || (N_FLAG != V_FLAG)) {
	    reg[15].I += ((int8_t)(opcode & 0xFF)) << 1;
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetch();
	}
    }

// SWI, B, BL /////////////////////////////////////////////////////////////

// SWI #comment
    void thumbDF(uint32_t opcode) {
	CPUSoftwareInterrupt(opcode & 0xFF);
    }

// B offset
    void thumbE0(uint32_t opcode) {
	int offset = (opcode & 0x3FF) << 1;
	if (opcode & 0x0400)
	    offset |= 0xFFFFF800;
	reg[15].I += offset;
	armNextPC = reg[15].I;
	reg[15].I += 2;
	prefetch();
    }

// BLL #offset (forward)
    void thumbF0(uint32_t opcode) {
        if(opcode == 0xF3BF){
            // DMB = 11110011101111111000111101010000
            armNextPC = reg[15].I;
            prefetch();
            reg[15].I += 2;
            return;
        }

	// int offset = (opcode & 0x7FF);
	// reg[14].I = reg[15].I + (offset << 12);
	opcode = (opcode << 16) | cpuPrefetch[0];
	uint32_t s = (opcode>>26)&1;
	uint32_t a = (opcode>>13)&1;
	uint32_t c = (opcode>>11)&1;
	uint32_t i = ((opcode&0x3FF0000)>>5) | (opcode&0x7FF);
	uint32_t I1 = 1 ^ a ^ s;
	uint32_t I2 = 1 ^ c ^ s;
	uint32_t imm = (-s<<23) | (I1<<22) | (I2<<21) | i;
	imm <<= 1;
	reg[14].I = reg[15].I;
	reg[15].I += imm;
	armNextPC = reg[15].I;
	prefetch();
	reg[15].I += 2;
    }

// MRS
    void thumbF3(uint32_t opcode) {
	reg[(cpuPrefetch[0]>>8)&0xF].I = reg[13].I;
	armNextPC = reg[15].I;
	prefetch();
	reg[15].I += 2;
    }

// BLL #offset (backward)
    void thumbF4(uint32_t opcode) {
	opcode = (opcode << 16) | cpuPrefetch[0];
	uint32_t s = (opcode>>26)&1;
	uint32_t a = (opcode>>13)&1;
	uint32_t c = (opcode>>11)&1;
	uint32_t i = ((opcode&0x3FF0000)>>5) | (opcode&0x7FF);
	uint32_t I1 = 1 ^ a ^ s;
	uint32_t I2 = 1 ^ c ^ s;
	uint32_t imm = (-s<<23) | (I1<<22) | (I2<<21) | i;
	imm <<= 1;
	reg[14].I = reg[15].I;
	reg[15].I += imm;
	armNextPC = reg[15].I;
	prefetch();
	reg[15].I += 2;
    }

// BLH #offset
    void thumbF8(uint32_t opcode) {
	int offset = (opcode & 0x7FF);
	uint32_t temp = reg[15].I-2;
	reg[15].I = (reg[14].I + (offset<<1))&0xFFFFFFFE;
	armNextPC = reg[15].I;
	reg[15].I += 2;
	reg[14].I = temp|1;
	prefetch();
    }

// Instruction table //////////////////////////////////////////////////////

    using insnfunc_t = void (VMState::*)(uint32_t opcode);

#define R &VMState::
    static inline const insnfunc_t thumbInsnTable[1024] = {
	R thumb00<0>, R thumb00<1>, R thumb00<2>, R thumb00<3>, R thumb00<4>, R thumb00<5>, R thumb00<6>, R thumb00<7>,  // 00
	R thumb00<8>, R thumb00<9>, R thumb00<10>,R thumb00<11>,R thumb00<12>,R thumb00<13>,R thumb00<14>,R thumb00<15>,
	R thumb00<16>,R thumb00<17>,R thumb00<18>,R thumb00<19>,R thumb00<20>,R thumb00<21>,R thumb00<22>,R thumb00<23>,
	R thumb00<24>,R thumb00<25>,R thumb00<26>,R thumb00<27>,R thumb00<28>,R thumb00<29>,R thumb00<30>,R thumb00<31>,
	R thumb08<0>, R thumb08<1>, R thumb08<2>, R thumb08<3>, R thumb08<4>, R thumb08<5>, R thumb08<6>, R thumb08<7>,  // 08
	R thumb08<8>, R thumb08<9>, R thumb08<10>,R thumb08<11>,R thumb08<12>,R thumb08<13>,R thumb08<14>,R thumb08<15>,
	R thumb08<16>,R thumb08<17>,R thumb08<18>,R thumb08<19>,R thumb08<20>,R thumb08<21>,R thumb08<22>,R thumb08<23>,
	R thumb08<24>,R thumb08<25>,R thumb08<26>,R thumb08<27>,R thumb08<28>,R thumb08<29>,R thumb08<30>,R thumb08<31>,
	R thumb10<0>, R thumb10<1>, R thumb10<2>, R thumb10<3>, R thumb10<4>, R thumb10<5>, R thumb10<6>, R thumb10<7>,  // 10
	R thumb10<8>, R thumb10<9>, R thumb10<10>,R thumb10<11>,R thumb10<12>,R thumb10<13>,R thumb10<14>,R thumb10<15>,
	R thumb10<16>,R thumb10<17>,R thumb10<18>,R thumb10<19>,R thumb10<20>,R thumb10<21>,R thumb10<22>,R thumb10<23>,
	R thumb10<24>,R thumb10<25>,R thumb10<26>,R thumb10<27>,R thumb10<28>,R thumb10<29>,R thumb10<30>,R thumb10<31>,
	R thumb18<0>,R thumb18<1>,R thumb18<2>,R thumb18<3>,R thumb18<4>,R thumb18<5>,R thumb18<6>,R thumb18<7>,  // 18
	R thumb1A<0>,R thumb1A<1>,R thumb1A<2>,R thumb1A<3>,R thumb1A<4>,R thumb1A<5>,R thumb1A<6>,R thumb1A<7>,
	R thumb1C<0>,R thumb1C<1>,R thumb1C<2>,R thumb1C<3>,R thumb1C<4>,R thumb1C<5>,R thumb1C<6>,R thumb1C<7>,
	R thumb1E<0>,R thumb1E<1>,R thumb1E<2>,R thumb1E<3>,R thumb1E<4>,R thumb1E<5>,R thumb1E<6>,R thumb1E<7>,
	R thumb20<0>,R thumb20<0>,R thumb20<0>,R thumb20<0>,R thumb20<1>,R thumb20<1>,R thumb20<1>,R thumb20<1>,  // 20
	R thumb20<2>,R thumb20<2>,R thumb20<2>,R thumb20<2>,R thumb20<3>,R thumb20<3>,R thumb20<3>,R thumb20<3>,
	R thumb20<4>,R thumb20<4>,R thumb20<4>,R thumb20<4>,R thumb20<5>,R thumb20<5>,R thumb20<5>,R thumb20<5>,
	R thumb20<6>,R thumb20<6>,R thumb20<6>,R thumb20<6>,R thumb20<7>,R thumb20<7>,R thumb20<7>,R thumb20<7>,
	R thumb28<0>,R thumb28<0>,R thumb28<0>,R thumb28<0>,R thumb28<1>,R thumb28<1>,R thumb28<1>,R thumb28<1>,  // 28
	R thumb28<2>,R thumb28<2>,R thumb28<2>,R thumb28<2>,R thumb28<3>,R thumb28<3>,R thumb28<3>,R thumb28<3>,
	R thumb28<4>,R thumb28<4>,R thumb28<4>,R thumb28<4>,R thumb28<5>,R thumb28<5>,R thumb28<5>,R thumb28<5>,
	R thumb28<6>,R thumb28<6>,R thumb28<6>,R thumb28<6>,R thumb28<7>,R thumb28<7>,R thumb28<7>,R thumb28<7>,
	R thumb30<0>,R thumb30<0>,R thumb30<0>,R thumb30<0>,R thumb30<1>,R thumb30<1>,R thumb30<1>,R thumb30<1>,  // 30
	R thumb30<2>,R thumb30<2>,R thumb30<2>,R thumb30<2>,R thumb30<3>,R thumb30<3>,R thumb30<3>,R thumb30<3>,
	R thumb30<4>,R thumb30<4>,R thumb30<4>,R thumb30<4>,R thumb30<5>,R thumb30<5>,R thumb30<5>,R thumb30<5>,
	R thumb30<6>,R thumb30<6>,R thumb30<6>,R thumb30<6>,R thumb30<7>,R thumb30<7>,R thumb30<7>,R thumb30<7>,
	R thumb38<0>,R thumb38<0>,R thumb38<0>,R thumb38<0>,R thumb38<1>,R thumb38<1>,R thumb38<1>,R thumb38<1>,  // 38
	R thumb38<2>,R thumb38<2>,R thumb38<2>,R thumb38<2>,R thumb38<3>,R thumb38<3>,R thumb38<3>,R thumb38<3>,
	R thumb38<4>,R thumb38<4>,R thumb38<4>,R thumb38<4>,R thumb38<5>,R thumb38<5>,R thumb38<5>,R thumb38<5>,
	R thumb38<6>,R thumb38<6>,R thumb38<6>,R thumb38<6>,R thumb38<7>,R thumb38<7>,R thumb38<7>,R thumb38<7>,
	R thumb40_0,R thumb40_1,R thumb40_2,R thumb40_3,R thumb41_0,R thumb41_1,R thumb41_2,R thumb41_3,  // 40
	R thumb42_0,R thumb42_1,R thumb42_2,R thumb42_3,R thumb43_0,R thumb43_1,R thumb43_2,R thumb43_3,
	R thumb44_0,R thumb44_1,R thumb44_2,R thumb44_3,R thumbUI,R thumb45_1,R thumb45_2,R thumb45_3,
	R thumb46_0,R thumb46_1,R thumb46_2,R thumb46_3,R thumb47,R thumb47,R thumb47b,R thumb47b,
	R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,  // 48
	R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,
	R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,
	R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,R thumb48,
	R thumb50,R thumb50,R thumb50,R thumb50,R thumb50,R thumb50,R thumb50,R thumb50,  // 50
	R thumb52,R thumb52,R thumb52,R thumb52,R thumb52,R thumb52,R thumb52,R thumb52,
	R thumb54,R thumb54,R thumb54,R thumb54,R thumb54,R thumb54,R thumb54,R thumb54,
	R thumb56,R thumb56,R thumb56,R thumb56,R thumb56,R thumb56,R thumb56,R thumb56,
	R thumb58,R thumb58,R thumb58,R thumb58,R thumb58,R thumb58,R thumb58,R thumb58,  // 58
	R thumb5A,R thumb5A,R thumb5A,R thumb5A,R thumb5A,R thumb5A,R thumb5A,R thumb5A,
	R thumb5C,R thumb5C,R thumb5C,R thumb5C,R thumb5C,R thumb5C,R thumb5C,R thumb5C,
	R thumb5E,R thumb5E,R thumb5E,R thumb5E,R thumb5E,R thumb5E,R thumb5E,R thumb5E,
	R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,  // 60
	R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,
	R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,
	R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,R thumb60,
	R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,  // 68
	R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,
	R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,
	R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,R thumb68,
	R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,  // 70
	R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,
	R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,
	R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,R thumb70,
	R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,  // 78
	R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,
	R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,
	R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,R thumb78,
	R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,  // 80
	R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,
	R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,
	R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,R thumb80,
	R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,  // 88
	R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,
	R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,
	R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,R thumb88,
	R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,  // 90
	R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,
	R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,
	R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,R thumb90,
	R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,  // 98
	R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,
	R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,
	R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,R thumb98,
	R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,  // A0
	R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,
	R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,
	R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,R thumbA0,
	R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,  // A8
	R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,
	R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,
	R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,R thumbA8,
	R thumbB0,R thumbB0,R thumbB0,R thumbB0,R thumbUI,R thumbUI,R thumbUI,R thumbUI,  // B0
	R thumbB2a,R thumbB2b,R thumbB2c,R thumbB2d,R thumbUI,R thumbUI,R thumbUI,R thumbUI,
	R thumbB4,R thumbB4,R thumbB4,R thumbB4,R thumbB5,R thumbB5,R thumbB5,R thumbB5,
	R thumbUI,R thumbB6,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,
	R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,  // B8
	R thumbBA,R thumbBAb,R thumbUI,R thumbBAc,R thumbUI,R thumbUI,R thumbUI,R thumbUI,
	R thumbBC,R thumbBC,R thumbBC,R thumbBC,R thumbBD,R thumbBD,R thumbBD,R thumbBD,
	R thumbBE,R thumbBE,R thumbBE,R thumbBE,R thumbBF,R thumbUI,R thumbUI,R thumbUI,
	R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,  // C0
	R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,
	R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,
	R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,R thumbC0,
	R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,  // C8
	R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,
	R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,
	R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,R thumbC8,
	R thumbD0,R thumbD0,R thumbD0,R thumbD0,R thumbD1,R thumbD1,R thumbD1,R thumbD1,  // D0
	R thumbD2,R thumbD2,R thumbD2,R thumbD2,R thumbD3,R thumbD3,R thumbD3,R thumbD3,
	R thumbD4,R thumbD4,R thumbD4,R thumbD4,R thumbD5,R thumbD5,R thumbD5,R thumbD5,
	R thumbD6,R thumbD6,R thumbD6,R thumbD6,R thumbD7,R thumbD7,R thumbD7,R thumbD7,
	R thumbD8,R thumbD8,R thumbD8,R thumbD8,R thumbD9,R thumbD9,R thumbD9,R thumbD9,  // D8
	R thumbDA,R thumbDA,R thumbDA,R thumbDA,R thumbDB,R thumbDB,R thumbDB,R thumbDB,
	R thumbDC,R thumbDC,R thumbDC,R thumbDC,R thumbDD,R thumbDD,R thumbDD,R thumbDD,
	R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbDF,R thumbDF,R thumbDF,R thumbDF,
	R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,  // E0
	R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,
	R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,
	R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,R thumbE0,
	R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,  // E8
	R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,
	R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,
	R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,R thumbUI,
	R thumbF0,R thumbF0,R thumbF0,R thumbF0,R thumbF0,R thumbF0,R thumbF0,R thumbF0,  // F0
	R thumbF0,R thumbF0,R thumbF0,R thumbF0,R thumbF0,R thumbF0,R thumbF0,R thumbF3,
	R thumbF4,R thumbF4,R thumbF4,R thumbF4,R thumbF4,R thumbF4,R thumbF4,R thumbF4,
	R thumbF4,R thumbF4,R thumbF4,R thumbF4,R thumbF4,R thumbF4,R thumbF4,R thumbF4,
	R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,  // F8
	R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,
	R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,
	R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,R thumbF8,
    };

    #undef R

    void CPUSoftwareInterrupt(int comment){
    }

    void CPUUpdateCPSR()
    {
	uint32_t CPSR = reg[16].I & 0x40;
	if (N_FLAG)
	    CPSR |= 0x80000000;
	if (Z_FLAG)
	    CPSR |= 0x40000000;
	if (C_FLAG)
	    CPSR |= 0x20000000;
	if (V_FLAG)
	    CPSR |= 0x10000000;
	// if (!armIrqEnable) CPSR |= 0x80;
	CPSR |= 1<<24; // T-bit
	reg[16].I = CPSR;
    }

    void CPUUpdateFlags()
    {
	uint32_t CPSR = reg[16].I;
	N_FLAG = (CPSR & 0x80000000) ? true : false;
	Z_FLAG = (CPSR & 0x40000000) ? true : false;
	C_FLAG = (CPSR & 0x20000000) ? true : false;
	V_FLAG = (CPSR & 0x10000000) ? true : false;
    }

    void reset() {
        crashed = false;

	// clean registers
	for (int i = 0; i < 45; i++)
	    reg[i].I = 0;

        reg[0].I = ramSize;
	reg[15].I = read32(8) & ~1;
	reg[13].I = ramSize - 4;

	C_FLAG = false;
	V_FLAG = false;
	N_FLAG = false;
	Z_FLAG = false;

	CPUUpdateCPSR();

	armNextPC = reg[15].I;
	reg[15].I += 2;

	prefetch();
    }

    void exec(std::size_t s) {
        if (crashed)
            return;
        this->speed = s;
        for (std::size_t i = 0; i < this->speed; ++i) {
	    auto OPCODE = cpuPrefetch[0];
	    cpuPrefetch[0] = cpuPrefetch[1];
	    armNextPC = reg[15].I;
	    reg[15].I += 2;
	    prefetchNext();
	    (this->*thumbInsnTable[OPCODE>>6])(OPCODE);
	}
    }

    uint32_t read32(uint32_t address) {
        if (address + 3 > maxRAMAddr) {
            if (!crashed)
                printf("read32 invalid address %p\n", (void*)(intptr_t)address);
            speed = 0;
            crashed = true;
            return 0xCCCCCCCC;
        }
        return *reinterpret_cast<uint32_t*>(&RAM[address]);
    }

    uint32_t read16(uint32_t address){
        if (address + 1 > maxRAMAddr) {
            if (!crashed)
                printf("read16 invalid address %p (> %p)\n", (void*)(intptr_t)address, (void*)(intptr_t)maxRAMAddr);
            crashed = true;
            return 0;
        }
        return *reinterpret_cast<uint16_t*>(&RAM[address]);
    }

    int16_t read16s(uint32_t address){
        if (address + 1 > maxRAMAddr) {
            if (!crashed)
                printf("read16s invalid address %p\n", (void*)(intptr_t)address);
            speed = 0;
            crashed = true;
            return 0;
        }
        return *reinterpret_cast<int16_t*>(&RAM[address]);
    }

    uint8_t read8(uint32_t address) {
        if (address > maxRAMAddr) {
            if (!crashed)
                printf("read8 invalid address %p\n", (void*)(intptr_t)address);
            speed = 0;
            crashed = true;
            return 0;
        }
        return RAM[address];
    }


    uint32_t exec16(uint32_t address){
        if (address + 1 > maxRAMAddr) {
            if (!crashed)
                printf("exec16 invalid address %p (> %p)\n", (void*)(intptr_t)address, (void*)(intptr_t)maxRAMAddr);
            crashed = true;
            return 0;
        }
        return *reinterpret_cast<uint16_t*>(&RAM[address]);
	// return read16(address);
    }

    void write32(uint32_t address, uint32_t value) {
        if (address >= ramSize - 3) {
            if (!crashed)
                printf("write32 invalid address %p\n", (void*)(intptr_t)address);
            crashed = true;
            speed = 0;
        } else {
            *reinterpret_cast<uint32_t*>(&RAM[address]) = value;
        }
    }

    void write16(uint32_t address, uint16_t value) {
        if (address >= ramSize - 1) {
            if (!crashed)
                printf("write16 invalid address %p\n", (void*)(intptr_t)address);
            crashed = true;
            speed = 0;
        } else {
            *reinterpret_cast<uint16_t*>(&RAM[address]) = value;
        }
    }

    void write8(uint32_t address, uint8_t b) {
        if (address >= ramSize) {
            if (!crashed)
                printf("write8 invalid address %p\n", (void*)(intptr_t)address);
            crashed = true;
            speed = 0;
        } else {
            RAM[address] = b;
        }
    }
};

uint32_t VM::Args::get(uint32_t i) const {
    if (i < 4)
        return state->reg[i].I;
    return state->read32(state->reg[13].I + (i - 4) * 4);
}

void* VM::Args::getPtr(uint32_t i) const {
    auto local = get(i);
    if (!local || local >= state->maxRAMAddr)
        return nullptr;
    return state->RAM + local;
}

uint32_t countImports(const std::byte* RAM, std::uint32_t ramSize) {
    uint32_t importCount = 0;
    for (uint32_t importPtr = 12; importPtr + 4 < ramSize; importPtr += 4) {
        uint32_t keyPtr = static_cast<uint32_t>(RAM[importPtr + 3]) << 24;
        keyPtr |= static_cast<uint32_t>(RAM[importPtr + 2]) << 16;
        keyPtr |= static_cast<uint32_t>(RAM[importPtr + 1]) << 8;
        keyPtr |= static_cast<uint32_t>(RAM[importPtr]);
        if (keyPtr == 0)
            break;
        importCount++;
    }
    return importCount;
}

void VM::boot(const std::vector<std::byte>& image, std::size_t ramSize) {
    if (image.size() <= 8) {
        printf("Invalid image size\n");
        return;
    }

    if (char(image[0]) != 'D' ||
        char(image[1]) != 'I' ||
        char(image[2]) != 'R' ||
        char(image[3]) != 'T') {
        printf("Invalid image magic\n");
    }

    std::size_t headerRamSize = 0;
    headerRamSize = static_cast<uint32_t>(image[7]) << 24;
    headerRamSize |= static_cast<uint32_t>(image[6]) << 16;
    headerRamSize |= static_cast<uint32_t>(image[5]) << 8;
    headerRamSize |= static_cast<uint32_t>(image[4]);
    ramSize = std::max(std::max(ramSize, headerRamSize), image.size() + 1024*1024);

    ramSize += 3;
    ramSize &=~ 3;

    uint32_t importCount = countImports(image.data(), ramSize);
    auto extendedRamSize = ramSize + importCount * 4;
    printf("Booting VM with size=%.2fKB, api size=%d, and image size=%.2fKB\n", ramSize / 1024.0f, (int)api.size(), image.size() / 1024.0f);

    {
        std::lock_guard guard{ramSizeMutex};
        RAM.resize(extendedRamSize);
    }

    auto size = std::min(ramSize, image.size());
    for (auto i = 0; i < size; ++i)
        RAM[i] = static_cast<uint8_t>(image[i]);

    link(ramSize);

    state = std::make_shared<VMState>();
    state->vm = this;
    state->RAM = RAM.data();
    state->ramSize = ramSize;
    state->extendedRamSize = extendedRamSize;
    state->maxRAMAddr = extendedRamSize - 1;
    state->reset();
}

void VM::link(uint32_t ramSize) {
    apiIndex.clear();
    auto& globalAPI = globalMap();
    for (uint32_t importPtr = 12; importPtr + 4 < ramSize; importPtr += 4) {
        uint32_t keyPtr = static_cast<uint32_t>(RAM[importPtr + 3]) << 24;
        keyPtr |= static_cast<uint32_t>(RAM[importPtr + 2]) << 16;
        keyPtr |= static_cast<uint32_t>(RAM[importPtr + 1]) << 8;
        keyPtr |= static_cast<uint32_t>(RAM[importPtr]);
        if (keyPtr == 0)
            break;
        std::string key;
        for (char c; keyPtr < RAM.size() && (c = static_cast<char>(RAM[keyPtr])) != 0; ++keyPtr)
            key.push_back(c);
        auto it = globalAPI.find(key);
        if (it == globalAPI.end()) {
            it = api.find(key);
            if (it == api.end()) {
                printf("WARNING: Invalid import #%d: [%s]\n", importPtr>>2, key.c_str());
                continue;
            }
        }
        std::uint32_t apiPtr = ramSize + (apiIndex.size() << 2);
        auto keyBackupPtr = ramSize + (apiIndex.size() << 2);
        apiIndex.push_back(it->second);
        RAM[importPtr + 3] = apiPtr >> 24;
        RAM[importPtr + 2] = apiPtr >> 16;
        RAM[importPtr + 1] = apiPtr >> 8;
        RAM[importPtr    ] = apiPtr;
        RAM[keyBackupPtr + 3] = keyPtr >> 24;
        RAM[keyBackupPtr + 2] = keyPtr >> 16;
        RAM[keyBackupPtr + 1] = keyPtr >> 8;
        RAM[keyBackupPtr + 0] = keyPtr;
        // printf("linked [%s @ %x] = %x\n", key.c_str(), importPtr, apiPtr);
    }
}


void VM::run() {
    state->exec(speed);
}

std::vector<std::byte> VM::suspend() {
    std::vector<std::byte> data;
    data.resize(sizeof(VMState) + RAM.size());
    std::memcpy(data.data(), state.get(), sizeof(VMState));
    std::memcpy(data.data() + sizeof(VMState), RAM.data(), RAM.size());
    return data;
}

void VM::thaw(const std::vector<std::byte>& data) {
    state = std::make_shared<VMState>();
    state->vm = this;
    std::memcpy(state.get(), data.data(), sizeof(VMState));
    {
        std::lock_guard guard{ramSizeMutex};
        RAM.resize(state->maxRAMAddr + 1);
    }
    std::memcpy(RAM.data(), data.data() + sizeof(VMState), RAM.size());
    state->RAM = RAM.data();

    uint32_t ramSize = state->ramSize;
    auto importCount = countImports(reinterpret_cast<std::byte*>(RAM.data()), ramSize);
    memcpy(RAM.data() + 12, RAM.data() + ramSize, importCount << 2);
    link(ramSize);
}

void VM::yield() {
    state->speed = 0;
}

void* VM::toHost(uint32_t ptr, std::size_t size) const {
    if (!ptr)
        return nullptr;
    uint32_t max = RAM.size();
    if (ptr >= max || ptr + size > max) {
        printf("toHost error: Invalid ptr %p\n", (void*)(uintptr_t)ptr);
        return nullptr;
    }
    return state->RAM + ptr;
}

uint32_t VM::toGuest(const void* data, std::size_t size) {
    {
        std::lock_guard guard{ramSizeMutex};
        RAM.resize(state->extendedRamSize + size);
    }
    state->RAM = RAM.data();
    state->maxRAMAddr = RAM.size() - 1; // state->extendedRamSize + size - 1;
    memcpy(state->RAM + state->extendedRamSize, data, size);
    return state->extendedRamSize;
}

bool VM::crashed() const {
    return state->crashed;
}
