# iPort80
```angular2html
  Copyright (C) 2024 devporter007
 
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see https://www.gnu.org/licenses/
 
```
iPort80 is a work-in-progress emulator that aims to accurately replicate the Intel 8085 microprocessor. The implementation strictly follows the original 8085A manual for architecture and instruction set accuracy.

## Important Information
* First 16KB of memory is reserved for ROM, Emulator by design wont allow you to write in ROM so write only from 4000H.
* Currently you need to manually tinker with the main function to read the output. Proper outputting to a text file is the next priority.
* Refer to this [manual](Intel(R)%208085%20User%20Manual.pdf) for 8085's architectural information and information about instruction sets.
## How to use?
Just specify your input.txt location when emulator asks for it.

Format for input.txt


```
DATA: {STARTING ADDRESS}
8_BIT_DATA
8_BIT_DATA
8_BIT_DATA
xx
xx
xx

DATA: {STARTING_ADDRESS_2}
8_BIT_DATA
8_BIT_DATA
8_BIT_DATA
xx
xx
xx

START: {STARTING_ADDRESS}
HEX_OP_CODE
HEX_OP_CODE
..
..
..
76
```
### Rules:
* Always keep the DATA sections at the top, they can be as much as you want.
* START section should always be kept at the bottom.
* OPCODES should always be given in base16(hexadecimal) format and should comply with the architectural requirements of 8085 processor.
* 76(HLT) should be present at the end of file otherwise emulator will run indefinitely.
## Examples:
* Example 1:
```
START: 4001
87 (ADD A)
76 (HLT)

```
    Output code:

    cpu.A = 0x15; (SET A)
    mainloop(cpu); (EXECUTE)
    cout << cpu.A; (CHECK OUTPUT)
* Example 2:
```angular2html
START: 4001
26 (MVI H,40H)
40
2E (MVI L, 10H)
10
36 (MVI M, 12H)
12
76 (HLT)
```
```
Output code:
cout << cpu.memory.read(0b0100000000010000);

Expected Outcome: 00010010
```

* Example 3: Showcasing usage of DATA sections
```angular2html
DATA: 4005(specifies the starting location of data, in this case being 4005H)
4E (random data)
3C (random data)

START: 4001
2A (LHLD 4005H)
05
40
76 (HLT)
```
```angular2html
Output Code:
cout << cpu.H << cpu.L;

Expected Outcome: 0011110001001110
```

More examples [here](docs/EXAMPLES.md).
## Implementation Priorities(In Order)
* Detailed documentation of emulator specifics and usage guidelines.
* Output to a text file of all the things that changed.
* Complete instruction set of 8085.
* Debugging Capabilities.

## Contributing
See [CONTRIBUTING.md](docs/CONTRIBUTING.md)
