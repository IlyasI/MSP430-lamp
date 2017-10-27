# MSP430-lamp
Program to demonstrate a  unique 'lamp' implementation with the TI MSP430 Family Microcontrollers

## Functionality:
- Pressing a button will turn the lamp on.
- Subsequent presses will cycle the lamp through three discrete brightness setting and turning the
lamp off.
- When the button has not been pressed for a short time, pressing the button will immediately turn
the lamp off. When the button is pressed after this, it will return to the last brightness state.
- The lamp allows user customization of each of the three brightness settings through a serial
interface.

![Lamp Specifications](https://github.com/IlyasI/MSP430-lamp/blob/master/LampFunctionality.png "Lamp Specifications")

First, the push button is connected via Pin 1.3 on the MSP430. I used an external push button which was connected to P1.3.
When the button is pressed, it first goes through a software debounce using the watchdog timer. 
This ensures that a single button press is not registered as multiple presses by the MSP430. 

I chose to detect the rising edge of the button press in my design, meaning holding the button will act exactly like a single press. 
It is also possible to use a hardware debounce here, although I did not choose that route.
Once the bounce is over, the button press goes through the ‘Set State’ block, which follows the specifications of the lamp to decide which discrete brightness setting is next.

The initial state is State 0, this is when the lamp is off. Each time the button is pressed, the system decides
what the next state should be based on Timer_A. If the timer has reached the time limit in any state,
pressing the button will immediately return the lamp to State 0. Pressing the button after this occurs will
return the lamp to its previous state. This is accomplished with three variables, NextState, LastState, and
CycleComplete.

When the time limit is reached, the system will set CycleComplete to 0, indicating that it has timed out
and not completed the full cycle (0->1->2->3->0). NextState of course is set to 0. LastState is used to
remember which state to return to.
If the time limit has not been reached, the system will simply proceed to the next state (0->1->2->3->0…). 
In this case, CycleComplete is set to 1 in state 3, indicating to State 0 to proceed to State 1, not return to State 3.

Once the current state has been set, Timer_A on the MSP430 is used to control the brightness of the lamp
by using pulse width modulation. By choosing a PWM frequency greater than human eye response time,
the lamp will look like it is on continuously. Lowering the duty cycle will make the lamp seem to dim.
Each brightness state has its own duty cycle.

As expected from the lamp functionality, the user can adjust each state’s brightness setting. First the user
should set the lamp to the brightness state intended to be modified. Then, by sending either a ‘+’ or ‘-‘
through the MSP430 serial port, the user will adjust the stored duty cycle for that state and subsequently
the brightness of the lamp. This functionality through the serial port is achieved with the hardware UART
on the MSP430.

The output pin 1.6 is connected to the AC Lamp through a circuit that drives the lamp with an external
AC power supply. This is often accomplished with a relay or MOSFET to simultaneously control the bulb
with the MSP430 and power it with the external AC power supply.
