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

// TODO:
// 1. Instruction Cycle that adds abstraction above executor, make executor feel confortable with multibyte instructions.


class mem {
public:
    array<bitset<8>, 65536> memory;
    bitset<16> rom_end_address;
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
        void ALU(bitset<8> input, bitset<8> controller){// ALU ONLY TAKES VALUES AS INPUTS, ONLY VALUES.
            if (controller == 0b00000000 || controller == 0b00000001){ //0 or 1
                bitset<1> cin = (controller == 0b00000001) ? flags[0] : 0;
                for (int i = 0; i <= 7; i++) {
                    if (A[i] == 0b0 && input[i] == 0b0 && (flags[0] == 0b0)) {
                        A[i] = 0b0;
                        if (cin == 1) A[i] = 0b1;
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
int main() {
    eightfive cpu;
    resetCPU(cpu);
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
            return 0;
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
    return 0;
}
