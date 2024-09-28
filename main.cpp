#include <bits/stdc++.h> // not recommended but for ease im using it for now.
using namespace std;
// aims to replicate arch design of 8085 with some changes
// memory specifications: 64kx16 RAM;
// 16 bit registers.
// This design will follow the modular approach where each component is complete in itself.
// converts hex to int;
// Custom instructions are succeded with XP tag, eg: DIVXP, MULXP;
// 8 bit opcode as standard in 8085
class eightfive{
    // Registers, each to use 16 bits;
    public:
        bitset<16> A,B,C,D,H,L;
    private:
        bitset<16> W,Z;
    public:
        bitset<8> flags; // S(7) Z(6) x(5) AC(4) x(3) P(2) x(1) Cy(0)
        stack<bitset<16>> mainstack;
        bitset<16> programCounter;
        eightfive(){
            A = B = C = D = H = L = 0;
            programCounter = 0;
        } // constructor
        // This ALU is capable of performing the following things:
        // Addition, Substraction, Multiplication, Division, PoW.
        // controller is specifically mentioned here instead of OPCODE, the OPCODE will be specified based on registered used.
        // Self defined controller codes for the following instructions:
        // Addition:
        // ADD/ADI (0)
        // ADC/ACI (1)
        // Substraction:
        // SUB/SUI (2)
        // Multiplication
        // MULXP (3)
        // Division
        // DIVXP (4)
        // PoW
        // POWXP (5)
        void parity(){
            bitset<16> store(0b0000000000000000);
            for (int j=0; j<16; j++){
                store[0]^A[j];
            }
            if (store == 0b0000000000000001) flags[2] = false;
            else flags[2] = true;
        }
        void ALU(bitset<16> input, bitset<8> controller){// ALU ONLY TAKES VALUES AS INPUTS, ONLY VALUES.

            // ADC needs to be implemented.
            if (controller == 0b00000000 || controller == 0b00000010){ //0
                /*  Carry flag (CY) : The carry flag is set to 1 if the addition result exceeds 16 bits. This indicates that a carry occurred from the most significant bit.
                    Auxiliary Carry flag (AC):This flag is set if there's a carry from bit 7 to bit 8 during the addition operation.
                    Sign flag (S): The sign flag is set based on the most significant bit of the 16-bit result (bit 15). If bit 15 is 1, the S flag is set; otherwise, it's reset.
                    Zero flag (Z):The zero flag is set if the 16-bit result after truncation is zero; otherwise, it's reset.
                    Parity flag (P):The parity flag is set based on the parity (even or odd) of the number of 1s in the 16-bit result.*/
                for (int i=0; i<=15; i++){
                    if (A[i] == 0b0 && input[i] == 0b0 && (flags[0] == 0b0)) {A[i] = false;flags[0] = false;}
                    else if (A[i] == 0b0 && input[i] == 0b0 && (flags[0] == 0b1)) {A[i] = true;flags[0] = false;}
                    else if ((A[i] == 0b0 && input[i] == 0b1 || A[i] == 0b1 && input[i] == 0b0) && (flags[0] == 0b0)) {A[i] = true;flags[0] = false;}
                    else if ((A[i] == 0b0 && input[i] == 0b1 || A[i] == 0b1 && input[i] == 0b0) && (flags[0] == 0b1)) {A[i] = false;flags[0] = true; if (i == 7) flags[4] = true;}
                    else if (A[i] == 0b1 && input[i] == 0b1 && flags[0] == 0b0){A[i] = false;flags[0] = true;if (i == 7) flags[4] = true;}
                    else if (A[i] == 0b1 && input[i] == 0b1 && flags[0] == 0b1){A[i] = true;flags[0] = true;if (i == 7) flags[4] = true;}
                    if (i==15 && A[15]==0b1) flags[7] = true;
                    else if (i==15 && A[15] == 0b0) flags[7] = false;
                    if (i==15 && A == 0b0000000000000000) flags[6] = true;
                    else flags[6] = false;
                    if (i==15){
                        parity();
                    }
                }
            }

            else if (controller == 0b00000011){//3
                /*
                    Sign (S): Set if the result is negative (bit 15 is 1), reset otherwise
                    Zero (Z): Set if the result is zero, reset otherwise
                    Auxiliary Carry (AC): Set if there's a borrow from bit 7 to bit 8, reset otherwise
                    Parity (P): Set if the result has even parity, reset if odd
                    Carry (CY): Set if there's a borrow, reset otherwise
                 */
                // to substract stuff which is A = A - INPUT here, we take the 2s complement of INPUT and then perform ADD.
                input.flip();
                input = bitset<16>(input.to_ulong() + 1);
                // 2s complement done;
                ALU(input,0b00000000);

            }
            else if (controller == 0b00000100){//4

            }

        };
};
void printResult(eightfive &cpu) {
    cout << "A = " << hex << uppercase << cpu.A.to_ulong() << endl;
    for (int i = 7; i >= 0; i--) {
        cout << cpu.flags[i] << " ";
    }
    cout << endl;
    cout << "S(7) Z(6) x(5) AC(4) x(3) P(2) x(1) Cy(0)" << endl;
}

int main() {
    eightfive cpu;

    // Test Case 1: SUB
    cpu.A = 0x10;
    cpu.ALU(0xA, 0b00000011);
    cout << "After SUB (Test 1):" << endl;
    printResult(cpu);

    return 0;
}
