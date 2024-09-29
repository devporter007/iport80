#include <bits/stdc++.h> // not recommended but for ease im using it for now.
using namespace std;
// aims to replicate arch design of 8085 with some changes
// memory specifications: 64k RAM+ROM(8bit);
// 8 bit registers.
// This design will follow the modular approach where each component is complete in itself.
// converts hex to int;
// Custom instructions are succeded with XP tag, eg: DIVXP, MULXP;
// 8 bit opcode as standard in 8085

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
        mem memory;
    public:
        bitset<8> A,B,C,D,H,L;
        eightfive(){
            A = B = C = D = H = L = 0;
            programCounter = 0;
        }
        bitset<8> flags; // S(7) Z(6) x(5) AC(4) x(3) P(2) x(1) Cy(0)
        stack<bitset<16>> mainstack;
        bitset<16> programCounter;

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
    cpu.A = 0b11111111;
    cpu.ALU(0b11111111,0b00000011);
    printResult(cpu);
    return 0;
}
