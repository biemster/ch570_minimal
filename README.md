# ch570_minimal
WCH CH570 minimal example without SDK.\
Just the pointer #defines in CH570SFR.h from the official SDK, a linker script and the startup assembly.
The C source flashes the LED on pin 8 of the WeAct module, and sleeps in between.

# compile
For compilation GCC `riscv64-elf` is tested, and WCH MounRiver Studio should work too.

# flash
The supplied `ch570_config.py` and `ch570_wchlink.py` are still a work in progress.\
`chprog.py` from the awesome https://github.com/wagiminator/MCU-Flash-Tools can be used to flash `minimal.bin`
by adding `{'name': 'CH570', 'id': 0x1370, 'code_size': 245760, 'data_size':     0},` to the list of devices in that file.