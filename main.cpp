/*
 * iPort80 - Intel 8085 Microprocessor Emulator
 * Copyright (C) 2024 devporter007
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <bits/stdc++.h>
#include <fstream>
#include <string>
#include <regex>
using namespace std;
// aims to replicate arch design of 8085 exactly.
// memory specifications: 64k RAM+ROM(8bit);
// 8 bit registers.
// This design will follow the modular approach where each component is complete in itself.
// 8 bit opcode as standard in 8085
// There are some places where i have intentionally decided to deviate from 8085 internal implementations in favour of major performance benefits.

class InstructionHandler{
private:
    unordered_map<uint8_t, vector<uint8_t>> instructionInfo;
public:
    InstructionHandler(){
        // there are a total of 246 usable opcodes in 8085A as per Intel(R) Manual for 80 Processor Family.
        // 202 out of those are actually 1 byte opcodes so it makes sense to assign all of them to 1 byte instruction initially
        // we will later make exceptions for 2 or 3 byte ones, that aint much anyway(44)

        // structure of unordered_map: <opcode, {byte_size, number_of_cycles, additional_cycles, add_cycle_required}

        // the reason for including number of cycles is for clock accurate emulation which will be implemented later on.
        // cycles will be very helpful in cases like delays etc.
        for (int i = 0; i < 256; i++) {
            instructionInfo[i] = {1, 4, 0, 0};
        }

        // list of all 2 byte instructions
        // MVI R,data(8 opcodes)
        // ADI data (1 opcode)
        // ACI data (1 opcode)
        // SUI data (1 opcode)
        // SBI data (1 opcode)
        // ANI data (1 opcode)
        // XRI data (1 opcode)
        // ORI data (1 opcode)
        // CPI data (1 opcode)
        // IN d8 (1 opcode)
        // OUT d8 (1 opcode)

        // list of all 3 byte instructions
        // LXI r, Data (4 opcodes)
        // LDA data (1 opcode)
        // STA data (1 opcode)
        // LHLD data (1 opcode)
        // SHLD data (1 opcode)
        // JMP memory_address (1 opcode)
        // JC memory_address (1 opcode)
        // JNC memory_address (1 opcode)
        // JZ memory_address (1 opcode)
        // JNZ memory_address (1 opcode)
        // JP memory_address (1 opcode)
        // JM memory_address (1 opcode)
        // JPE memory_address (1 opcode)
        // JPO memory_address (1 opcode)
        // CALL memory_address (1 opcode)
        // CC memory_address (1 opcode)
        // CM memory_address (1 opcode)
        // CNC memory_address (1 opcode)
        // CNZ memory_address (1 opcode)
        // CP memory_address (1 opcode)
        // CPE memory_address (1 opcode)
        // CPO memory_address (1 opcode)
        // CZ memory_address (1 opcode)

        //format: <opcode, {byte_size, number_of_cycles, additional_cycles, add_cycle_required}
        pair<uint8_t, vector<uint8_t>> exceptions[] = {
                {0x36,{2,3,0,0}}, {0x3E,{2,2,0,0}}, {0x06,{2,2,0,0}},{0x0E,{2,2,0,0}},{0x16,{2,2,0,0}},{0x1E,{2,2,0,0}},{0x26,{2,2,0,0}},{0x2E,{2,2,0,0}},{0xC6,{2,2,0,0}},{0xCE,{2,2,0,0}},
                {0xD6,{2,2,0,0}},{0xDE,{2,2,0,0}},{0xE6,{2,2,0,0}},{0xEE,{2,2,0,0}},{0xF6,{2,2,0,0}},{0xFE,{2,2,0,0}},{0xDB,{2,3,0,0}},{0xD3,{2,3,0,0}},{0x01,{3,3,0,0}},{0x11,{3,3,0,0}},
                {0x21,{3,3,0,0}},{0x31,{3,3,0,0}},{0x3A,{3,4,0,0}},{0x32,{3,4,0,0}},{0x2A,{3,5,0,0}},{0x22,{3,5,0,0}},{0xC3,{2,3,0,0}},{0xDA,{3,2,1,1}},{0xD2,{3,2,1,1}},{0xCA,{3,2,1,1}}
                ,{0xC2,{3,2,1,1}},{0xF2,{3,2,1,1}},{0xFA,{3,2,1,1}},{0xEA,{3,2,1,1}},{0xE2,{3,2,1,1}},{0xCD,{3,6,0,0}},{0xDC,{3,6,3,1}},{0xFC,{3,6,3,1}},{0xD4,{3,6,3,1}},{0xC4,{3,6,3,1}},
                {0xF4,{3,6,3,1}},{0xEC,{3,6,3,1}},{0xE4,{3,6,3,1}},{0xCC,{3,6,3,1}}};

        for(auto items : exceptions){
            instructionInfo[items.first] = items.second;
        }
    }

    vector<uint8_t> retrieve_instruction(uint8_t opcode){
        auto it = instructionInfo.find(opcode);
        if (it != instructionInfo.end()) {
            return it->second;
        }
        else{
            return {0,0,0,0};
        }
    }

};


class mem {
private:
    array<bitset<8>, 65536> memory;
    bitset<16> rom_end_address;
public:
    explicit mem(uint16_t rom_size = 16383) {
        rom_end_address = bitset<16>(rom_size - 1);
    }
    void write(bitset<16> address, bitset<8> value) {
        if (address.to_ulong() > rom_end_address.to_ulong()) {
            memory[address.to_ulong()] = value;
        }
    }
    bitset<8> read(bitset<16> address) {
        return memory[address.to_ulong()];
    }
};
class eightfive{
private:
    InstructionHandler instructionHandler;
public:
    mem memory;
    bitset<8> A,B,C,D,E,H,L,M,W,Z;
    bitset<8> flags; // S(7) Z(6) x(5) AC(4) x(3) P(2) x(1) Cy(0)
    // PC and MS are implemented as 16 bit bitset which deviates from the accurate representation of how
    // things works in 8085 internally, they are implemented as a register pair and used thereof like that.
    // To ease of things a bit we are instead going for a direct 16 bit approach, it should not affect the
    // actual results from this emulated processor.
    stack<bitset<16>> mainstack;
    bitset<16> stackPointer;
    bitset<16> programCounter;
    eightfive() {
        // Initialize all registers to 0
        A = B = C = D = E = H = L = M = W = Z = bitset<8>(0);
        flags = bitset<8>(0);
        programCounter = bitset<16>(0);
    }

    bitset<8> loadM(){
        bitset<16> address = (static_cast<unsigned long long>(H.to_ulong()) << 8) | L.to_ulong();
        M = memory.read(address);
        return M;
    }

    void writeM(bitset<8> data){
        bitset<16> address = (static_cast<unsigned long long>(H.to_ulong()) << 8) | L.to_ulong();
        memory.write(address, data);
        return;
    }

private:
private:
    unordered_map<bitset<8>, function<void(eightfive&)>> optable = {
            // Handles all cases of MOV r,r;(49 cases)
            {0b01111111, [](eightfive& cpu){}},
            {0b01111000, [](eightfive& cpu){ cpu.A = cpu.B; }},
            {0b01111001, [](eightfive& cpu){ cpu.A = cpu.C; }},
            {0b01111010, [](eightfive& cpu){ cpu.A = cpu.D; }},
            {0b01111011, [](eightfive& cpu){ cpu.A = cpu.E; }},
            {0b01111100, [](eightfive& cpu){ cpu.A = cpu.H; }},
            {0b01111101, [](eightfive& cpu){ cpu.A = cpu.L; }},
            {0b01000111, [](eightfive& cpu){ cpu.B = cpu.A; }},
            {0b01000000, [](eightfive& cpu){}},
            {0b01000001, [](eightfive& cpu){ cpu.B = cpu.C;}},
            {0b01000010, [](eightfive& cpu){ cpu.B = cpu.D;}},
            {0b01000011, [](eightfive& cpu){ cpu.B = cpu.E;}},
            {0b01000100, [](eightfive& cpu){ cpu.B = cpu.H;}},
            {0b01000101, [](eightfive& cpu){ cpu.B = cpu.L;}},
            {0b01001111, [](eightfive& cpu){ cpu.C = cpu.A;}},
            {0b01001000, [](eightfive& cpu){ cpu.C = cpu.B;}},
            {0b01001000, [](eightfive& cpu){ cpu.C = cpu.B;}},
            {0b01001001, [](eightfive& cpu){}},
            {0b01001010, [](eightfive& cpu){ cpu.C = cpu.D;}},
            {0b01001011, [](eightfive& cpu){ cpu.C = cpu.E;}},
            {0b01001100, [](eightfive& cpu){ cpu.C = cpu.H;}},
            {0b01001101, [](eightfive& cpu){ cpu.C = cpu.L;}},
            {0b01010111, [](eightfive& cpu){ cpu.D = cpu.A;}},
            {0b01010000, [](eightfive& cpu){ cpu.D = cpu.B;}},
            {0b01010001, [](eightfive& cpu){ cpu.D = cpu.C;}},
            {0b01010010, [](eightfive& cpu){}},
            {0b01010011, [](eightfive& cpu){ cpu.D = cpu.E;}},
            {0b01010100, [](eightfive& cpu){ cpu.D = cpu.H;}},
            {0b01010101, [](eightfive& cpu){ cpu.D = cpu.L;}},
            {0b01011111, [](eightfive& cpu){ cpu.E = cpu.A;}},
            {0b01011000, [](eightfive& cpu){ cpu.E = cpu.B;}},
            {0b01011001, [](eightfive& cpu){ cpu.E = cpu.C;}},
            {0b01011010, [](eightfive& cpu){ cpu.E = cpu.D;}},
            {0b01011011, [](eightfive& cpu){}},
            {0b01011100, [](eightfive& cpu){ cpu.E = cpu.H;}},
            {0b01011101, [](eightfive& cpu){ cpu.E = cpu.L;}},
            {0b01100111, [](eightfive& cpu){ cpu.H = cpu.A;}},
            {0b01100000, [](eightfive& cpu){ cpu.H = cpu.B;}},
            {0b01100001, [](eightfive& cpu){ cpu.H = cpu.C;}},
            {0b01100010, [](eightfive& cpu){ cpu.H = cpu.D;}},
            {0b01100011, [](eightfive& cpu){ cpu.H = cpu.E;}},
            {0b01100100, [](eightfive& cpu){}},
            {0b01100101, [](eightfive& cpu){ cpu.H = cpu.L;}},
            {0b01101111, [](eightfive& cpu){ cpu.L = cpu.A;}},
            {0b01101000, [](eightfive& cpu){ cpu.L = cpu.B;}},
            {0b01101001, [](eightfive& cpu){ cpu.L = cpu.C;}},
            {0b01101010, [](eightfive& cpu){ cpu.L = cpu.D;}},
            {0b01101011, [](eightfive& cpu){ cpu.L = cpu.E;}},
            {0b01101100, [](eightfive& cpu){ cpu.L = cpu.H;}},
            {0b01101101, [](eightfive& cpu){ }},

            // Handles cases of MOV r,M (7 cases)
            {0b01111110, [](eightfive& cpu){ cpu.A = cpu.loadM();}},
            {0b01000110, [](eightfive& cpu){ cpu.B = cpu.loadM();}},
            {0b01001110, [](eightfive& cpu){ cpu.C = cpu.loadM();}},
            {0b01010110, [](eightfive& cpu){ cpu.D = cpu.loadM();}},
            {0b01011110, [](eightfive& cpu){ cpu.E = cpu.loadM();}},
            {0b01100110, [](eightfive& cpu){ cpu.H = cpu.loadM();}},
            {0b01101110, [](eightfive& cpu){ cpu.L = cpu.loadM();}},

            // Handles cases of MOV M,r (7 cases)
            {0b01110111, [](eightfive& cpu){ cpu.writeM(cpu.A);}},
            {0b01110000, [](eightfive& cpu){ cpu.writeM(cpu.B);}},
            {0b01110001, [](eightfive& cpu){ cpu.writeM(cpu.C);}},
            {0b01110010, [](eightfive& cpu){ cpu.writeM(cpu.D);}},
            {0b01110011, [](eightfive& cpu){ cpu.writeM(cpu.E);}},
            {0b01110100, [](eightfive& cpu){ cpu.writeM(cpu.H);}},
            {0b01110101, [](eightfive& cpu){ cpu.writeM(cpu.L);}},

            // ADD r/M(8 cases);
            {0b10000111, [](eightfive& cpu){ cpu.ALU(cpu.A,0b00000000);}},
            {0b10000000, [](eightfive& cpu){ cpu.ALU(cpu.B,0b00000000);}},
            {0b10000001, [](eightfive& cpu){ cpu.ALU(cpu.C,0b00000000);}},
            {0b10000010, [](eightfive& cpu){ cpu.ALU(cpu.D,0b00000000);}},
            {0b10000011, [](eightfive& cpu){ cpu.ALU(cpu.E,0b00000000);}},
            {0b10000100, [](eightfive& cpu){ cpu.ALU(cpu.H,0b00000000);}},
            {0b10000101, [](eightfive& cpu){ cpu.ALU(cpu.L,0b00000000);}},
            {0b10000110, [](eightfive& cpu){ cpu.ALU(cpu.loadM(),0b00000000);}},

            // ADC r/M(8 cases);
            {0b10001111, [](eightfive& cpu){ cpu.ALU(cpu.A,0b00000001);}},
            {0b10001000, [](eightfive& cpu){ cpu.ALU(cpu.B,0b00000001);}},
            {0b10001001, [](eightfive& cpu){ cpu.ALU(cpu.C,0b00000001);}},
            {0b10001010, [](eightfive& cpu){ cpu.ALU(cpu.D,0b00000001);}},
            {0b10001011, [](eightfive& cpu){ cpu.ALU(cpu.E,0b00000001);}},
            {0b10001100, [](eightfive& cpu){ cpu.ALU(cpu.H,0b00000001);}},
            {0b10001101, [](eightfive& cpu){ cpu.ALU(cpu.L,0b00000001);}},
            {0b10001110, [](eightfive& cpu){ cpu.ALU(cpu.loadM(),0b00000001);}},

            // SUB r/M(7 cases);
            {0b10010111, [](eightfive& cpu){ cpu.ALU(cpu.A,0b00000011);}},
            {0b10010000, [](eightfive& cpu){ cpu.ALU(cpu.B,0b00000011);}},
            {0b10010001, [](eightfive& cpu){ cpu.ALU(cpu.C,0b00000011);}},
            {0b10010010, [](eightfive& cpu){ cpu.ALU(cpu.D,0b00000011);}},
            {0b10010011, [](eightfive& cpu){ cpu.ALU(cpu.E,0b00000011);}},
            {0b10010100, [](eightfive& cpu){ cpu.ALU(cpu.H,0b00000011);}},
            {0b10010101, [](eightfive& cpu){ cpu.ALU(cpu.L,0b00000011);}},
            {0b10010110, [](eightfive& cpu){ cpu.ALU(cpu.loadM(),0b00000011);}},

            // MVI r/M (8 cases)
            {0x3E, [](eightfive& cpu){ cpu.A = cpu.Z;}},
            {0x06, [](eightfive& cpu){ cpu.B = cpu.Z;}},
            {0x0E, [](eightfive& cpu){ cpu.C = cpu.Z;}},
            {0x16, [](eightfive& cpu){ cpu.D = cpu.Z;}},
            {0x1E, [](eightfive& cpu){ cpu.E = cpu.Z;}},
            {0x26, [](eightfive& cpu){ cpu.H = cpu.Z;}},
            {0x2E, [](eightfive& cpu){ cpu.L = cpu.Z;}},
            {0x36, [](eightfive& cpu){ cpu.writeM(cpu.Z);}},

            // LXI rp, data16
            {0x01, [](eightfive& cpu){ cpu.B = cpu.W; cpu.C = cpu.Z;}},
            {0x11, [](eightfive& cpu){ cpu.D = cpu.W; cpu.E = cpu.Z;}},
            {0x21, [](eightfive& cpu){ cpu.H = cpu.W; cpu.L = cpu.Z;}},
            {0x31, [](eightfive& cpu){ cpu.stackPointer = ((cpu.W.to_ullong() << 8) | cpu.Z.to_ullong());}}, // LXI SP data16

            // LDA
            {0x3A, [](eightfive& cpu){ bitset<16> memad = ((cpu.W.to_ullong() << 8) | cpu.Z.to_ullong()); cpu.A = cpu.memory.read(memad);}},

            // STA
            {0x32, [](eightfive& cpu){ bitset<16> memad = ((cpu.W.to_ullong() << 8) | cpu.Z.to_ullong()); cpu.memory.write(memad, cpu.A);}},

            // LHLD
            {0x2A, [](eightfive& cpu){ bitset<16> memad = ((cpu.W.to_ullong() << 8) | cpu.Z.to_ullong()); bitset<8> lo = cpu.memory.read(memad); cpu.L = lo; memad = cpu.incrementor_decrementor(memad,0x0);bitset<8> ho = cpu.memory.read(memad); cpu.H = ho;}},

            //SHLD
            {0x22, [](eightfive& cpu){ bitset<16> memad = ((cpu.W.to_ullong() << 8) | cpu.Z.to_ullong()); cpu.memory.write(memad,cpu.L); memad = cpu.incrementor_decrementor(memad,0x0);cpu.memory.write(memad,cpu.H);}},

            //LDAX(2 cases)
            {0x0A, [](eightfive& cpu){ bitset<16> memad = ((cpu.B.to_ullong() << 8) | cpu.C.to_ullong()); cpu.A = cpu.memory.read(memad);}},
            {0x1A, [](eightfive& cpu){ bitset<16> memad = ((cpu.D.to_ullong() << 8) | cpu.E.to_ullong()); cpu.A = cpu.memory.read(memad);}},

            //STAX(2 cases)
            {0x02, [](eightfive& cpu){ bitset<16> memad = ((cpu.B.to_ullong() << 8) | cpu.C.to_ullong()); cpu.memory.write(memad,cpu.A);}},
            {0x12, [](eightfive& cpu){ bitset<16> memad = ((cpu.D.to_ullong() << 8) | cpu.E.to_ullong()); cpu.memory.write(memad,cpu.A);}},

            //XCHG , safely uses W-Z register pair to facilitate exchange as it is a single byte instruction, there is no need to worry about overwriting.
            {0xEB, [](eightfive& cpu){
                cpu.W = cpu.H;
                cpu.Z = cpu.L;
                cpu.H = cpu.D;
                cpu.L = cpu.E;
                cpu.D = cpu.W;
                cpu.E = cpu.Z;}},

            // ADI d8
            {0xC6, [](eightfive& cpu){ cpu.ALU(cpu.Z,0x0);}},

            // ACI d8
            {0xCE, [](eightfive& cpu){ cpu.ALU(cpu.Z,0x1);}},

            // SUI d8 D6
            {0xD6, [](eightfive& cpu){ cpu.ALU(cpu.Z,0x3);}},

            // SBB r(8 cases)
            {0x9F, [](eightfive& cpu){ cpu.ALU(cpu.A,0x4);}},
            {0x98, [](eightfive& cpu){ cpu.ALU(cpu.B,0x4);}},
            {0x99, [](eightfive& cpu){ cpu.ALU(cpu.C,0x4);}},
            {0x9A, [](eightfive& cpu){ cpu.ALU(cpu.D,0x4);}},
            {0x9B, [](eightfive& cpu){ cpu.ALU(cpu.E,0x4);}},
            {0x9C, [](eightfive& cpu){ cpu.ALU(cpu.H,0x4);}},
            {0x9D, [](eightfive& cpu){ cpu.ALU(cpu.L,0x4);}},
            {0x9E, [](eightfive& cpu){ cpu.ALU(cpu.loadM(),0x4);}},

            // SBI d8
            {0xDE, [](eightfive& cpu){ cpu.ALU(cpu.Z,0x4);}},

           /* // INR r //
            {0x04, [](eightfive& cpu){
                uint8_t vin = cpu.B.to_ulong();
                uint8_t result = vin + 1; // properly handles edge case of 255+1;
                cpu.B = result;



            }},*/
    };
public:
    void parity() {
        bool parity = 0;
        for (int j = 0; j < 8; j++) {
            parity ^= A[j];
        }
        flags[2] = (parity == 0);
    }

    bitset<16> incrementor_decrementor(bitset<16> input, bitset<8> controller){
        // Do not use WZ registers here, incrementor and decrementor circuit has its own dedicated registers as per the architecture to avoid overwrites.
        if (controller == 0b00000000){ // ADD 1
            uint16_t bin = input.to_ulong();
            bitset<8> temp_w = (bin & 0xFF00) >> 8;
            bitset<8> temp_z = (bin & 0x00FF);
            // add 1 to temp_z, adjust carry in temp_w, combine temp_w+temp_z to input and return;
            bitset<1> temp_carry = 0b0; // as implemented internally inside incrementor/decrementor, does not affect any flags.
            // deviates from hardware accuracy in favour of performance, we perform integer math and check if it exeeds 2^8, if yes then its a overflow and set carry to 1, return the value to temp_z anyway.
            uint8_t z_val = temp_z.to_ulong();
            z_val++;
            temp_carry = (z_val == 0) ? 1 : 0;
            temp_z = bitset<8>(z_val);

            if (temp_carry == 1) {// adds carry to 1 in case of lower byte overflow, applies same logic from above.
                uint8_t w_val = temp_w.to_ulong();
                w_val++;
                temp_w = bitset<8>(w_val);
            }
            return bitset<16>((temp_w.to_ullong() << 8) | temp_z.to_ullong());
        }
        else if (controller == 0b00000011) {// SUB 1
            uint16_t bin = input.to_ulong();
            bitset<8> temp_w = (bin & 0xFF00) >> 8;
            bitset<8> temp_z = (bin & 0x00FF);
            bitset<1> temp_carry = 0b0;
            uint8_t z_val = temp_z.to_ulong();
            z_val--;
            temp_carry = (z_val == 255) ? 1 : 0;
            temp_z = bitset<8>(z_val);
            if (temp_carry == 1) {// adds carry to 1 in case of lower byte overflow, applies same logic from above.
                uint8_t w_val = temp_w.to_ulong();
                w_val--;
                temp_w = bitset<8>(w_val);
            }
            return bitset<16>((temp_w.to_ullong() << 8) | temp_z.to_ullong());
        }
        return 0b0;
    }
    void ALU(bitset<8> input, bitset<8> controller){// ALU ONLY TAKES VALUES AS INPUTS, ONLY VALUES.
        if (controller == 0b00000000 || controller == 0b00000001){ // ADD or ADC
            bitset<1> carryin = (controller == 0b00000001) ? flags[0] : 0;
            bool carry_out = false;
            for (int i = 0; i <= 7; i++) {
                bool sum = A[i] ^ input[i] ^ carryin[0];
                carry_out = (A[i] & input[i]) | (A[i] & carryin[0]) | (input[i] & carryin[0]);
                A[i] = sum;
                if (i == 7) flags[0] = carry_out;
                if (i == 3 && carry_out) flags[4] = 0b1;
                carryin[0] = carry_out;
            }
            flags[7] = A[7];
            if (A == 0b00000000) flags[6] = 0b1;
            else flags[6] = 0b0;
            parity();
        }

        else if (controller == 0b00000011){// 3 : SUB/SUI
            // to substract stuff which is A = A - INPUT here, we take the 2s complement of INPUT and then perform ADD.
            input.flip();
            input = bitset<8>(input.to_ulong() + 1);
            ALU(input,0b00000000);
        }
        else if (controller == 0b00000100) { // 4 : SBB
            bool cain = flags[0];
            ALU(input, 0b00000011);
            if (cain) {
                bitset<8> one(1);
                ALU(one, 0b00000011);
            }
        }

    };

    void decode(bitset<8> opcode){
        vector<uint8_t> opcontrol = instructionHandler.retrieve_instruction(opcode.to_ulong());
        uint8_t opsize = opcontrol[0];
        auto opcheck = optable.find(opcode);
        if (opcheck != optable.end()){
            if (opsize == 1){
                programCounter = incrementor_decrementor(programCounter,0x0);
            }
            else if (opsize == 2){
                programCounter = incrementor_decrementor(programCounter,0x0);
                Z = memory.read(programCounter); // fetched second byte and stored in Z;
                programCounter = incrementor_decrementor(programCounter,0x0);
            }
            else if (opsize == 3){
                programCounter = incrementor_decrementor(programCounter,0x0);
                Z = memory.read(programCounter); // lower byte instruction fetched first to comply with 8085 arch.
                programCounter = incrementor_decrementor(programCounter,0x0);
                W = memory.read(programCounter); // higher byte instruction fetch second to comply with 8085 arch.
                programCounter = incrementor_decrementor(programCounter,0x0);
            }
        }
    }
    void executor(bitset<8> opcode) {
        auto opcheck = optable.find(opcode);
        if (opcheck != optable.end()) {
            opcheck->second(*this);
        }
    }

    void InstructionCycle(){
        while(true){// HLT
            bitset<8> opcode = memory.read(programCounter);
            if(opcode == 0b01110110) break;
            decode(opcode);
            executor(opcode);
        }
    }
};

void resetCPU(eightfive &cpu){
    cpu.flags = 0;
}

void mainloop(eightfive &cpu) {
    regex start_pattern("^START: (\\d{4})$");
    regex data_pattern("^DATA: (\\d{4})$");
    smatch matches;
    string workDir;
    cout << "Enter input.txt location: ";
    cin >> workDir;
    ifstream file(workDir);
    string line;
    uint16_t start_address = 0;
    uint16_t current_address = 0;
    while (getline(file, line)) {
        if (regex_match(line, matches, data_pattern)) {
            current_address = stoul(matches[1].str(), nullptr, 16);
            while (getline(file, line) && !line.empty() && line.find("DATA:") == string::npos && line.find("START:") == string::npos) {
                try {
                    unsigned long data = stoul(line, nullptr, 16);
                    cpu.memory.write(current_address, data);
                    current_address = cpu.incrementor_decrementor(current_address, 0x0).to_ulong();
                } catch (const std::exception& e) {
                    cerr << "Error processing data line: " << line << endl;
                    return;
                }
            }
            if (!line.empty()) {
                file.seekg(-line.length() - 1, ios_base::cur);
            }
        }
        if (regex_match(line, matches, start_pattern)) {
            break;
        }
    }
    file.clear();
    file.seekg(0, ios::beg);
    while (getline(file, line)) {
        if (regex_match(line, matches, start_pattern)) {
            start_address = stoul(matches[1].str(), nullptr, 16);
            current_address = start_address;
            while (getline(file, line)) {
                if (line.empty() || line.find_first_not_of(' ') == string::npos) {
                    continue;
                }
                try {
                    unsigned long data = stoul(line, nullptr, 16);
                    cpu.memory.write(current_address, data);
                    current_address = cpu.incrementor_decrementor(current_address, 0x0).to_ulong();
                } catch (const std::exception& e) {
                    cerr << "Error processing instruction line: " << line << endl;
                    return;
                }
            }
            break;
        }
    }
    if (start_address == 0) {
        cerr << "No START section found in the input file." << endl;
        return;
    }
    cpu.programCounter = start_address;
    cpu.InstructionCycle();
}
int main() {
    eightfive cpu;
    resetCPU(cpu);
    mainloop(cpu);
    cout << cpu.H << cpu.L;
    return 0;
}
