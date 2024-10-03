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
// TODO:
// 1. Instruction Cycle that adds abstraction above executor, make executor feel confortable with multibyte instructions.
// 2. For that to happen, an "InstructionHandler" class needs to be implemented that will guide the processor on how to interpret an instruction.

class InstructionHandler{
private:
    unordered_map<uint8_t, std::pair<uint8_t, uint8_t>> instructionInfo;
public:
    InstructionHandler(){
        // there are a total of 246 usable opcodes in 8085A as per Intel(R) Manual for 80 Processor Family.
        // 202 out of those are actually 1 byte opcodes so it makes sense to assign all of them to 1 byte instruction initially
        // we will later make exceptions for 2 or 3 byte ones, that aint much anyway(44)
        // structure of unordered_map: <opcode, {byte_size, number_of_cycles}
        // the reason for including number of cycles is for clock accurate emulation which will be implemented later on.
        for (int i = 0; i < 256; i++) {
            instructionInfo[i] = {1, 4};
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
    }

};

class mem {
private:
    array<bitset<8>, 65536> memory;
    bitset<16> rom_end_address;
public:
    explicit mem(uint16_t rom_size = 16384) {
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
    bitset<8> W,Z;
public:
    mem memory;
    bitset<8> A,B,C,D,E,H,L,M;
    eightfive(){
        A = B = C = D = E = H = L = 0;
        programCounter = 0;
    }
    bitset<8> loadM(){
        bitset<16> address = (static_cast<unsigned long long>(H.to_ulong()) << 8) | L.to_ulong();
        M = memory.read(address);
        return M;
    }

    bitset<8> flags; // S(7) Z(6) x(5) AC(4) x(3) P(2) x(1) Cy(0)
    // PC and MS are implemented as 16 bit bitset which deviates from the accurate representation of how
    // things works in 8085 internally, they are implemented as a register pair and used thereof like that.
    // To ease of things a bit we are instead going for a direct 16 bit approach, it should not affect the
    // actual results from this emulated processor.
    stack<bitset<16>> mainstack;
    bitset<16> programCounter;

private:
    unordered_map<bitset<8>, function<void()>> optable = {
            // Handles all cases of MOV r,r;(49 cases)
            {0b01111111, [](){}},
            {0b01111000, [this](){ A = B; }},
            {0b01111001, [this](){ A = C; }},
            {0b01111010, [this](){ A = D; }},
            {0b01111011, [this](){ A = E; }},
            {0b01111100, [this](){ A = H; }},
            {0b01111101, [this](){ A = L; }},
            {0b01000111, [this](){ B = A; }},
            {0b01000000, [this](){}},
            {0b01000001, [this](){ B = C;}},
            {0b01000010, [this](){ B = D;}},
            {0b01000011, [this](){ B = E;}},
            {0b01000100, [this](){ B = H;}},
            {0b01000101, [this](){ B = L;}},
            {0b01001111, [this](){ C = A;}},
            {0b01001000, [this](){ C = B;}},
            {0b01001000, [this](){ C = B;}},
            {0b01001001, [this](){}},
            {0b01001010, [this](){ C = D;}},
            {0b01001011, [this](){ C = E;}},
            {0b01001100, [this](){ C = H;}},
            {0b01001101, [this](){ C = L;}},
            {0b01010111, [this](){ D = A;}},
            {0b01010000, [this](){ D = B;}},
            {0b01010001, [this](){ D = C;}},
            {0b01010010, [this](){}},
            {0b01010011, [this](){ D = E;}},
            {0b01010100, [this](){ D = H;}},
            {0b01010101, [this](){ D = L;}},
            {0b01011111, [this](){ E = A;}},
            {0b01011000, [this](){ E = B;}},
            {0b01011001, [this](){ E = C;}},
            {0b01011010, [this](){ E = D;}},
            {0b01011011, [this](){}},
            {0b01011100, [this](){ E = H;}},
            {0b01011101, [this](){ E = L;}},
            {0b01100111, [this](){ H = A;}},
            {0b01100000, [this](){ H = B;}},
            {0b01100001, [this](){ H = C;}},
            {0b01100010, [this](){ H = D;}},
            {0b01100011, [this](){ H = E;}},
            {0b01100100, [this](){}},
            {0b01100101, [this](){ H = L;}},
            {0b01101111, [this](){ L = A;}},
            {0b01101000, [this](){ L = B;}},
            {0b01101001, [this](){ L = C;}},
            {0b01101010, [this](){ L = D;}},
            {0b01101011, [this](){ L = E;}},
            {0b01101100, [this](){ L = H;}},
            {0b01101101, [this](){ }},

            // Handles cases of MOV r,M (7 cases)
            {0b01111110, [this](){A = loadM();}},
            {0b01000110, [this](){B = loadM();}},
            {0b01001110, [this](){C = loadM();}},
            {0b01010110, [this](){D = loadM();}},
            {0b01011110, [this](){E = loadM();}},
            {0b01100110, [this](){H = loadM();}},
            {0b01101110, [this](){L = loadM();}},

            // Handles cases of MOV M,r (7 cases)

            {0b01110111, [this](){M = A;}},
            {0b01110000, [this](){M = B;}},
            {0b01110001, [this](){M = C;}},
            {0b01110010, [this](){M = D;}},
            {0b01110011, [this](){M = E;}},
            {0b01110100, [this](){M = H;}},
            {0b01110101, [this](){M = L;}},

            // ADD r(7 cases);
            {0b10000111, [this](){ALU(A,0b00000000);}},
            {0b10000000, [this](){ALU(B,0b00000000);}},
            {0b10000001, [this](){ALU(C,0b00000000);}},
            {0b10000010, [this](){ALU(D,0b00000000);}},
            {0b10000011, [this](){ALU(E,0b00000000);}},
            {0b10000100, [this](){ALU(H,0b00000000);}},
            {0b10000101, [this](){ALU(L,0b00000000);}},
            {0b10000110, [this](){ALU(M,0b00000000);}},

            // ADC r(7 cases);
            {0b10001111, [this](){ALU(A,0b00000001);}},
            {0b10001000, [this](){ALU(B,0b00000001);}},
            {0b10001001, [this](){ALU(C,0b00000001);}},
            {0b10001010, [this](){ALU(D,0b00000001);}},
            {0b10001011, [this](){ALU(E,0b00000001);}},
            {0b10001100, [this](){ALU(H,0b00000001);}},
            {0b10001101, [this](){ALU(L,0b00000001);}},
            {0b10001110, [this](){ALU(M,0b00000001);}},

            // SUB r(7 cases);
            {0b10010111, [this](){ALU(A,0b00000011);}},
            {0b10010000, [this](){ALU(B,0b00000011);}},
            {0b10010001, [this](){ALU(C,0b00000011);}},
            {0b10010010, [this](){ALU(D,0b00000011);}},
            {0b10010011, [this](){ALU(E,0b00000011);}},
            {0b10010100, [this](){ALU(H,0b00000011);}},
            {0b10010101, [this](){ALU(L,0b00000011);}},
            {0b10010110, [this](){ALU(M,0b00000011);}},
    };
public:
    void parity(){
        bitset<8> store(0b00000000);
        for (int j=0; j<8; j++){
            store[0] = store[0]^A[j];
        }
        if (store == 0b00000001) flags[2] = false;
        else flags[2] = true;
    }
    bitset<16> incrementor_decrementor(bitset<16> input, bitset<8> controller){
        // use WZ Registers to break down 16 bits into 8 bits chunks, then increment lower byte, check carry and adjust.
        if (controller == 0b00000000){ // ADD 1
            W = (input.to_ullong() & 0xFF00) >> 8;
            Z = (input.to_ullong() & 0x00FF);
            // add 1 to Z, adjust carry in W, combine WZ to input and return;
            bitset<1> temp_carry = 0b0; // as implemented internally inside incrementor/decrementor, does not affect any flags.
            // deviates from hardware accuracy in favour of performance, we perform integer math and check if it exeeds 2^8, if yes then its a overflow and set carry to 1, return the value to Z anyway.
            uint8_t z_val = Z.to_ulong();
            z_val++;
            temp_carry = (z_val == 0) ? 1 : 0;
            Z = bitset<8>(z_val);

            if (temp_carry == 1) {// adds carry to 1 in case of lower byte overflow, applies same logic from above.
                uint8_t w_val = W.to_ulong();
                w_val++;
                W = bitset<8>(w_val);
            }
            return bitset<16>((W.to_ullong() << 8) | Z.to_ullong());
        }
        else if (controller == 0b00000011) {// SUB 1
            W = (input.to_ullong() & 0xFF00) >> 8;
            Z = (input.to_ullong() & 0x00FF);
            bitset<1> temp_carry = 0b0;
            uint8_t z_val = Z.to_ulong();
            z_val--;
            temp_carry = (z_val == 255) ? 1 : 0;
            Z = bitset<8>(z_val);
            if (temp_carry == 1) {// adds carry to 1 in case of lower byte overflow, applies same logic from above.
                uint8_t w_val = W.to_ulong();
                w_val--;
                W = bitset<8>(w_val);
            }
            return bitset<16>((W.to_ullong() << 8) | Z.to_ullong());
        }
        return 0b0;
    }
    void ALU(bitset<8> input, bitset<8> controller){// ALU ONLY TAKES VALUES AS INPUTS, ONLY VALUES.

        // plans to optimize ALU using bit-manipulation, deviation from original architecture but wont be an issue accuracy wise.
        if (controller == 0b00000000 || controller == 0b00000001){ //0 or 1
            bitset<1> carryin = (controller == 0b00000001) ? flags[0] : 0;
            for (int i = 0; i <= 7; i++) {
                if (A[i] == 0b0 && input[i] == 0b0 && (flags[0] == 0b0)) {
                    A[i] = 0b0;
                    if (carryin == 1) A[i] = 0b1;
                    flags[0] = 0b1;
                }
                else if (A[i] == 0b0 && input[i] == 0b0 && (flags[0] == 0b1)) {
                    A[i] = 0b1;
                    flags[0] = 0b0;
                }
                else if ((A[i] == 0b0 && input[i] == 0b1 || A[i] == 0b1 && input[i] == 0b0) && (flags[0] == 0b0)) {
                    A[i] = 0b1;
                    flags[0] = 0b0;
                }
                else if ((A[i] == 0b0 && input[i] == 0b1 || A[i] == 0b1 && input[i] == 0b0) && (flags[0] == 0b1)) {
                    A[i] = 0b0;
                    flags[0] = 0b1;
                    if (i == 3) flags[4] = 0b1; // Auxiliary Carry
                }
                else if (A[i] == 0b1 && input[i] == 0b1 && flags[0] == 0b0) {
                    A[i] = 0b0;
                    flags[0] = 0b1;
                    if (i == 3) flags[4] = 0b1; // Auxiliary Carry
                }
                else if (A[i] == 0b1 && input[i] == 0b1 && flags[0] == 0b1) {
                    A[i] = 0b1;
                    flags[0] = 0b1;
                    if (i == 3) flags[4] = 0b1;
                }
                if (i == 7 && A[7] == 0b1) flags[7] = 0b1;
                else if (i == 7 && A[7] == 0b0) flags[7] = 0b0;
            }
            // Check for zero flag
            if (A == 0b00000000) flags[6] = 0b1;
            else flags[6] = 0b0;
            // Update parity flag
            parity();
        }

        else if (controller == 0b00000011){//3 SUB/SUI
            // to substract stuff which is A = A - INPUT here, we take the 2s complement of INPUT and then perform ADD.
            input.flip();
            input = bitset<8>(input.to_ulong() + 1);
            ALU(input,0b00000000);
        }
    };

    void executor(bitset<8> opcode) {
        // implemented in compliance with the intel 8085A manual
        auto opcheck = optable.find(opcode);
        if (opcheck != optable.end()){
            opcheck->second();
        }
    }

    void InstructionCycle(){
        bitset<8> opcode = memory.read(programCounter);
        while(opcode != 0b01110110){// HLT
            //fetch/execute/decode



        }


    }

};
void printResult(eightfive &cpu) {
    cout << "Result " << cpu.A <<endl;
    for (int i = 7; i >= 0; i--) {
        cout << cpu.flags[i] << " ";
    }
    cout << endl;
    cout << "S(7) Z(6) x(5) AC(4) x(3) P(2) x(1) Cy(0)" << endl;
}



void resetCPU(eightfive cpu){
    cpu.flags = 0;
}


void mainloop(eightfive cpu){
    std::regex pattern("^START: (\\d{4})$");
    std::smatch matches;
    std::ifstream file("input.txt");
    std::string str;
    std::string mem_ad_loc;
    std::string mem_ad;
    while (std::getline(file, mem_ad_loc))
    {
        if (std::regex_match(mem_ad_loc, matches, pattern)) {
            mem_ad = matches[1].str();
            break;
        }
        else{
            return;
        }
    }
    std::getline(file, str);
    unsigned long start_address = std::stoul(mem_ad, nullptr, 16);
    while (std::getline(file, str)) {
        unsigned long data = std::stoul(str, nullptr, 16);
        cpu.memory.write(start_address, data);
    }
    cpu.programCounter = start_address;
    cpu.InstructionCycle(); // implement.
}
int main() {
    eightfive cpu;
    resetCPU(cpu);
    mainloop(cpu); // comment it out in case you want to tinker around stuff. InsCycle isnt implemented right now anyways.
    return 0;
}
