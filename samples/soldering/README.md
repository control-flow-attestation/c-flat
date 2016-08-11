# Raspberry Pi 2 port of Soldering Iron Controller for Hakko 907

Adapted from the original project [1][2]

## Functionality

This application controls the power applied to a connected soldering iron in an effor to keep the
temperature stable.

### Input

Input is entered through a serial interface connection in a way that emulates the input mechanisms
on the physical device.

There is a button that recognizes either short or long presses.
- 'k' presses the button down
- 'j' releases the button

There is also a rotary controller (turn knob) which can be turned by repeatedly tapping either 'h' or
'l'.

### Output

The physical devices has a four digit 7-led display, which is emulated with serial console outputs
(a new line is shown when any changes are made on the display).

    D>>> _0FF <<<D (btn mode: 0, rot: 435 [356, 752], iron: 256 ->-> 474)  (cur scr: ScreenMain)

The first section between the < & > brackets and Ds is the device led display, the rest is internal
states mainly for debugging.

#### Display output

The display is mostly self-explanatory, in regular work mode simply showing the current iron
temperature. When turning the knob it shows a number prefix with an 's' to indicate that the value
shown is the targeted value. NOTE: The default temperature unit is Fahrenheit.

The original implementation had heating cooling animations, which are here exchanged to simple
prefixing '\_' (cooling) or '^' (heating). The tune mode (used to calibrate the heat sensors) also
uses prefixes 'u' and 'l' to indicate upper and lower bounds.

#### Debug ouput
- btn mode: the most recent button state, where 0 is none, 1 is short press, 2 is long press
- rot: shows the current internal value and available range on the internal rotary controller
- iron: shows the current internal value and target-value for the soldering iron temperature.
- cur scr: show the currently active Screen (state)

## Similarity to original

The program structure and operation is essentially identical to original, although the main loop
includes some additional calls to handle input and output.

One big exception is the manual implementation of object creation and polymorphism. The explicit
object creation funcitons are executed only once before entering the main loop. Method calls and
object member variable acces also necessitated addition of explicit object pointers.

On very few occasions (which are documented in the code) unused/redundant statements we're removed
to get rid of compiler warnings. Also, binary values used for led animations have also been replaced
with simple character values for easier printing.

And finally, all hardware interfaces that weren't part of the original project have also been
replaced by dummies. These include:

    arduino.*, eeprom.*, LedControl.*, oo.h, serial.*, util.*

[1]: https://create.arduino.cc/projecthub/sfrwmaker/soldering-iron-controller-for-hakko-907-8c5866A "Soldering Iron Controller for Hakko 907"
[2]: https://github.com/sfrwmaker/soldering_controller "sfrwmaker/soldering_controller"
