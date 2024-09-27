#include <bits/stdc++.h> // not recommended but for ease im using it for now.
using namespace std;
// aims to replicate arch design of 8085 with some changes
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
        bitset<8> flags; // S Z x AC x P x Cy
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
        void ALU(bitset<16> input, bitset<8> controller){// ALU ONLY TAKES VALUES AS INPUTS, ONLY VALUES.
            if (controller == 0b00000000){ //0
                /*  Carry flag (CY):
                    The carry flag is set to 1 if the addition result exceeds 16 bits. This indicates that a carry occurred from the most significant bit.
                    Result truncation:
                    This processor can only store 16 bits in a register, so the result is truncated to 16 bits. Only the least significant 8 bits of the result are kept, while the 17th bit (the carry) is discarded after setting the carry flag.
                    Auxiliary Carry flag (AC):
                    This flag is set if there's a carry from bit 7 to bit 8 during the addition operation.
                    Sign flag (S):
                    The sign flag is set based on the most significant bit of the 16-bit result (bit 15). If bit 15 is 1, the S flag is set; otherwise, it's reset.
                    Zero flag (Z):
                    The zero flag is set if the 16-bit result after truncation is zero; otherwise, it's reset.
                    Parity flag (P):
                    The parity flag is set based on the parity (even or odd) of the number of 1s in the 16-bit result.*/
                for (int i=0; i<=15; i++){
                    if (A[i] == 0b0 && input[i] == 0b0 && (flags[0] == 0b0)) {A[i] = 0b0;flags[0] = 0b0;}
                    else if (A[i] == 0b0 && input[i] == 0b0 && (flags[0] == 0b1)) {A[i] = 0b1;flags[0] = 0b0;}
                    else if ((A[i] == 0b0 && input[i] == 0b1 || A[i] == 0b1 && input[i] == 0b0) && (flags[0] == 0b0)) {A[i] = 0b1;flags[0] = 0b0;}
                    else if ((A[i] == 0b0 && input[i] == 0b1 || A[i] == 0b1 && input[i] == 0b0) && (flags[0] == 0b1)) {flags[0] = 0b1; A[i] = 0b0;}
                    else if (A[i] == 0b1 && input[i] == 0b1 && flags[0] == 0b0){A[i] = 0b0;flags[0] = 0b1;}
                    else if (A[i] == 0b1 && input[i] == 0b1 && flags[0] == 0b1){A[i] = 0b1;flags[0] = 0b1;}
                }
            }
            else if (controller == 0b00000001){//1

            }
            else if (controller == 0b00000010){//2

            }
            else if (controller == 0b00000011){//3

            }
            else if (controller == 0b00000100){//4

            }

        };
};

int main() {

    return 0;
}
