# Raspberry Pi 2 port of Open Syringe Pump

Adapted from the original Open Syringe Pump project [1][2]

## Functionality

This application controls a stepper motor, which is connected to a syringe pump in order to deliver precise quantities of liquid from the syringe. In medical terminology, a "bolus" is a single dose of a drug or other medicinal preparation given all at once [6].

### Input

Input is provided through the serial interface using the following commands:
- an integer number sets the bolus size in micro-litres (uL)
- "+" pushes a bolus of the specified size
- "-" pulls a bolus of the specified size

All commands must be terminated by a newline character.

Input can also be provided through a set of trigger pins or an attached keypad. This functionality is not implemented for the Raspberry Pi in this version.

### Output

Output is provided through an attached LCD display. In this version, the information that would be displayed on the LCD is output via the serial port.


## Similarity to original

Apart from the unimplemented features, this port is functionally equivalent to the original application. 

The original code in the syringePump.ino file has been retained unmodified as far as possible in syringePump.c. The ported version contains 85 new or modified lines of code not present in the original*. This equates to less than 25% of the original code.

*calculated using:
comm -23 <(sort syringePump.c) <(sort syringePump.c.orig) | wc -l

The external functions used by this code (e.g. serial port control and GPIO operations etc.) naturally differ between the Arduino and Raspberry Pi platforms, and are excluded from the above metric.


## Critical functions

- void bolus(int direction) { ... } calculates the duration for which the motor step pin should be held at the HIGH level to push or pull a bolus of the specified size. It also sets the respective direction pin level and sets the motor step pin to HIGH for the calculated duration using a for loop.

- void readSerial() { ... } reads values from the serial input. This function may introduce a buffer overflow vulnerability due to the fixed length of the serialStr variable. 


[1]: https://hackaday.io/project/1838-open-syringe-pump "Open Syringe Pump (project page)"
[2]: https://github.com/naroom/OpenSyringePump/blob/master/syringePump/syringePump.ino "Open Syringe Pump (GitHub)"
[3]: https://www.raspberrypi.org/products/raspberry-pi-2-model-b/ "Raspberry Pi 2 Model B"
[4]: http://www.waveshare.com/dvk512.htm "DVK512, Raspberry Pi Expansion Board"
[5]: https://launchpad.net/gcc-arm-embedded "GCC ARM Embedded on Launchpad"
[6]: https://en.wikipedia.org/wiki/Bolus_(medicine) "Wikipedia: Bolus (medicine)"
