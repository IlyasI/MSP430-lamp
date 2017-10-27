# MSP430-lamp
Program to demonstrate a  unique 'lamp' implementation with the TI MSP430 Family Microcontrollers

##Functionality:
Pressing a button will turn the lamp on.
- Subsequent presses will cycle the lamp through three discrete brightness setting and turning the
lamp off.
- When the button has not been pressed for a short time, pressing the button will immediately turn
the lamp off. When the button is pressed after this, it will return to the last brightness state.
- The lamp allows user customization of each of the three brightness settings through a serial
interface.

![Lamp Specifications](https://github.com/adam-p/markdown-here/raw/master/src/common/images/icon48.png "Lamp Specifications")
