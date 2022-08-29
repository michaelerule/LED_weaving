https://electronics.stackexchange.com/questions/37859/how-to-increase-current-drive-of-charlieplexed-led-array

https://archive.ednasia.com/www.ednasia.com/ART_8800525712_1000004_AN_4ca8e558.HTM
https://www.edn.com/charlieplexing-at-high-duty-cycle/


Build a driver that 
 - Has 3 states: positive, negative, high-impedence
 - Can be toggled between these 3 states by a control line with the same three states
 - Allows high current draw for either the positive, negative, or both
 - Guarantees safe current limits on the control line
 - Can be switched relatively quickly


This thing will probably involve transistors, hopefully just one per driving line.
Try to minimize components. 


General notes on types of transistors: 

Helpful: https://electronicsclub.info/transistors.htm
More helpful: https://www.electronicshub.org/transistors-classification-and-types/#JFET_(JunctionField_Effect_Transistor)

I don't know much about transistors, but most of the ones lying around are either BJT or MOSFET. 
Let's consider MOSFETS

https://en.wikipedia.org/wiki/MOSFET

MOSFET TLDR

- Three terminals: 
  - GATE controls things
  - DRAIN (always +) and SOURCE (always -) move current.
- Gate acts like a capacitor; 
  - Finite switching time, large transient currents needed.
  - Some circuits protect μC pins by using 2nd BJT to drive MOSFET gate
- N channel
  - (+) bias on GATE→SOURCE opens channel; (-) bias closes it 
    - Pull-down to keep closed
  - When open, channel allows (+) at DRAIN to flow to (-) at SOURCE
  - So-called "body diode" ALWAYS allows (+) at SOURCE to flow to (-) at DRAIN
  - Unclear but seems like you want to use this to switch the CATHODE of LEDs
  - Load upstream seems to imply V_GS < V_DS; Is this important?
    - This is called a "Low-side switch"
  - Operating Modes:
    - V_GS < V_th: subthreshold current; exponential in V_GS
- P channel 
  - SOURCE is positive (Vcc)
  - Pull-up GATE to stay closed
  - DRAIN controls flow of positive power



Try: Partial drive: large positive current

 - Return path diode bypasses, default operation when negative
 - 


Homework exercises: 

Drive LED using NPN, PNP, N-channel MOSFET, P-channel MOSFET, both as high-side and low-side drivers. 
Compare and contrast each approach. 



https://electronics.stackexchange.com/questions/151326/bjt-driving-an-led-above-or-below/151328
suggests NPN should be used as low-side driver. Why is high-side driver unsafe? 
Answer: the Base-to-Emitter ΔV controls the current flow. Having load downstream of NPN requires a higher base voltage. 
When is this bad?
 - When driving base to higher voltage is costly or unsafe
When is this good? 
 - If the downstream load is diode-like (fixed Δv), it could be used to createa synthetic higher threshold
 - It could be used to more strongly inhibit the transistor. 

Other notes: BJT should be turned "off" by pulling μC controller pin negative. OR there should be a pull-down resistor. Maybe? 

So from an easy to understand point of view the left-hand circuit is best. However, the right-hand circuit does have some advantages. Mainly because the base current IB is set by two resistors it may be possible to remove one of them (the base resistor) and just have the one resistor in the circuit to limit both the base current and the LED's current. That can save on parts, and if you have a lot of these circuits in you design that reduction in parts can soon add up.


This is for current control with a fixed LED load, however. We actually want a fixed *voltage* that can handle arbitrary currents. 








