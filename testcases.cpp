// Test 1: Basic subtraction
cpu.A = 0x05;
cpu.ALU(0x90, 0x03);
cout << "After SUB (Test 1: 5 - 3):" << endl;
printResult(cpu);

// Test 2: Subtraction with zero
cpu.A = 0x08;
cpu.ALU(0x90, 0x00);
cout << "After SUB (Test 2: 8 - 0):" << endl;
printResult(cpu);

// Test 3: Subtraction resulting in zero
cpu.A = 0x05;
cpu.ALU(0x90, 0x05);
cout << "After SUB (Test 3: 5 - 5):" << endl;
printResult(cpu);

// Test 4: Subtraction with carry (borrow)
cpu.A = 0x03;
cpu.ALU(0x90, 0x05);
cout << "After SUB (Test 4: 3 - 5):" << endl;
printResult(cpu);

// Test 5: Subtraction with maximum values
cpu.A = 0xFF;
cpu.ALU(0x90, 0x01);
cout << "After SUB (Test 5: 255 - 1):" << endl;
printResult(cpu);

// Test 6: Subtraction affecting sign flag
cpu.A = 0x80;
cpu.ALU(0x90, 0x01);
cout << "After SUB (Test 6: 128 - 1):" << endl;
printResult(cpu);

// Test 7: Subtraction resulting in negative zero
cpu.A = 0x00;
cpu.ALU(0x90, 0x80);
cout << "After SUB (Test 7: 0 - 128):" << endl;
printResult(cpu);

// Test 8: Subtraction causing overflow
cpu.A = 0x7F;
cpu.ALU(0x90, 0xFF);
cout << "After SUB (Test 8: 127 - (-1)):" << endl;
printResult(cpu);

// Test 9: Subtraction of A from itself
cpu.A = 0x42;
cpu.ALU(0x97, 0x00);  // 0x97 is the opcode for SUB A
cout << "After SUB (Test 9: A - A):" << endl;
printResult(cpu);



if (controller == 0b00000000 || controller == 0b00000001) { // 0 for ADD, 1 for ADC
bool useCarry = (controller == 0b00000001);
bool initialCarry = useCarry ? flags[0] : 0;
bool carry = initialCarry;
bitset<16> result;
for (int i = 0; i <= 15; i++) {
bool sum = A[i] ^ input[i] ^ carry;
bool newCarry = (A[i] & input[i]) | (A[i] & carry) | (input[i] & carry);
result[i] = sum;
carry = newCarry;
if (i == 7 && carry) flags[4] = 0b1;
}
flags[0] = carry;
//(S)
flags[7] = result[15];
// (Z)
flags[6] = (result == 0);
// (P)
flags[2] = (result.count() % 2 == 0) ? 0b1 : 0b0;
A = result;
}