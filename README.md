# iPort80
WIP emulator that tries to replicate Intel's 8085 microprocessor

I'm sticking with the original 8085A manual and will be implementing architecture and instructions as such.

As of now emulator is functional with its limited instruction set. 

## Important Information
* First 16KB of memory is reserved for ROM, Please only start writing in memory from 4001H.
* Currently you need to manually tinker with the main function to read the output. Proper outputting to a text file is the next priority.
## How to use?
Just specify your input.txt location when emulator asks for it.

Format for input.txt


```
START: {STARTING_ADDRESS}
HEX_OP_CODE
HEX_OP_CODE
..
..
..
76
```

Examples:
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
## Implementation Priorities(In Order)
* Output to a text file of all the things that changed.
* Complete instruction set of 8085.
* Debugging Capabilities.

Program covered under GPLv3, devporter007.