# ECE4180B Final Project
## By Zachary Seletsky
---
### Overview
This repository details my final project for ECE4180 at Georgia Tech.

#### Mbed Code
All the mbed code is in the folder mbed-code. I used the following existing libraries: 

- MbedLibraryCollection by DanielCBailey
  - https://github.com/danielcbailey/MbedLibraryCollection#fc1305209ceffcbfbcca482c0349c74b0387b0af
- mpr121 library by Anthony Buckton

All other files were written by myself. 

#### Arduino Due Code (carSketch)
All Arduino Due code is in the folder carSketch. I wrote this code using the Arduino IDE and the following existing libraries:

- Servo by Michael Margolis
  - https://github.com/arduino-libraries/Servo.git
   
All other files were written by myself.

#### Physical Implementation
Below is a diagram depicting the implementation of the Arduino Due and Mbed NXP LPC1768 implementation. Note the 7.4V line from the battery for the car is connected to VIN.

![finalprojdiagram](https://github.gatech.edu/zseletsky3/ECE4180BFinalProject/assets/46628/19fde137-0650-45da-9367-45f88b11d809)


### Written Report

#### Overview
My embedded system is an RC car and RC controller. The purpose of this project is to create a toy RC car and controller from "scratch". The car was built using lego technic pieces and custom 3D-prints. The contoller is simply a breadboard with an external battery attached.

#### Components
The RC car uses the Arduino Due, a standard hobby motor and a motor driver (TB6612FNG) for the
powertrain, a servo (HS-422) for steering, a Bluetooth module (HC05) for communication with the RC controller. These components are necessary for driving the car, steering the car, and ultimately communicating with the controller. There were also three major mechanical components, which I tore out of a Lego Technic Ferrari 488, that were required to build the platform. The first two, the rack and pinion steering in combination with the rear differential (Lego nailed that), allowed for traditional right-left steering. The rack and pinion physically directed the front wheels, while the rear differential allowed the rear wheels to move at different speeds. When turning, the inside wheel travels a much shorter path than the outside wheel. If they were to move at the same speed, one of the wheels would lock up, leading to reduced traction, increased wear, and in my case, break the gear interface between the electric motor and the plastic lego gears. Also, its a rear differential in a toy RC car, which I believe is so awesome (thank you Lego). The third major mechanical component is the functional suspension. While this is not necessary for the car to travel on flat ground, it does enable increased traction "off-road" and a smoother ride when travelling at "high speeds". Currently this version does go much faster than a light jogger's speed, however there is plenty of room on the rear end to attach another motor (or two) to get it there.

The RC controller uses the mbed nxp 1768, a rotary pulse generator for steering input, a button for throttle, a button for braking, a switch for changing gears (forward-reverse), a capacitive keyboard (MPR121) for unlocking the controller upon powering it on, a uLCD screen (uLCD-144G2) for conveying information such as Bluetooth status, steering input, brake/gas input, and for displaying a phone-style lock/unlock screen, and a Bluetooth module (HC05) for communication with the RC car. The rotary pulse geneartor, throttle button, gear switch, and Bluetooth module are all necessary for contolling the RC car. The brake button, uLCD screen, and capacitive touch keyboard are additional components to enable non-essential features such as input feedback and security.

#### Problems Encountered
My biggest problem was getting the bluetooth to work. Originally, I was using a SparkFun BlueSMiRF and a generic HC-08 module. Initially, I spent a considerable amount of time writing a code interface for talking to the bluetooth modules in command mode (AT-mode) over my pc serial port. The BlueSMiRF was printing weird garabage over the monitor, and eventually, I was able to get the BlueSMiRF to print out some error codes. The codes indicating the BlueSMiRF was browning out due to incorrectly managing its onboard voltage. This led me down a rabbit hole that ended with me using capacitors to manage the incoming voltage, only to discover the BlueSMiRF was actualy incompatible with the HC-08 module once I was able to get it working properly. So I ordered an HC-05 off of amazon, which I also discovered was incompatible with both the HC-08 and the BlueSMiRF. While I was bewildered at the lack of clear documentation on this fact (I only found a couple obscure forums saying this), I immediately ordered another HC-05 module after confirming they are indeed compatible with each other. Luckily they were only $10 each. While I was waiting for the parts, I spent the time programming the car and controller seperately so that I could control the RC car from my pc serial monitor and I could output commands from my controller to my pc. Once I had the proper parts, I was already so familiar with the AT-command system that with only a few tweaks to my exisiting interface, I was able to directly program both controllers to pair with each other only and immediately upon powering on. Finally, I had solved my bluetooth problem and finished interfacing the RC car and controller.

Another issue I faced was using my 3D printer to print some of the larger parts. The first layer of my prints were fine, but the second layers on would displace the original layer and result in a large glop of 3D print waste attached to a well-printed single layer. After scouring online forums, I decided to decrease the speed of the printer and increase the temperature of the printing bed to facilitate better cohesion between the bed and the first layer of the print. While I was skeptical how adding heat would prevent the 3D print from devolving into a glob of hot 3D waste instead of the opposite, it worked perfectly.

#### Similarities to Existing Real World Embedded Systems
My project has a rack and pinion steering system, rear differential, suspension, electronic controls, a control input that uses rotational movement to steer, a seperate input for gas and for brake, and a simple digital interface for displaying important values and vehicle states. I argue it is extremely similar to embedded systems in real-world manufactured commercial vehicles. As well as existing RC car implementations.

#### Future Improvements
There are quite a few areas of improvements for this project. Firstly, the ultrasonic sensor, which is currently awkardly attached to the front of the car, has no functionality. I wanted to use it to implement a collision warning/prevention system, however the servo had to rotate the sensor rapidly back and forth for it to be effective and the sound it made was simply too annoying. As well, the sensor would only report distances in mulitples of 17cm, which was frustrating. While this may have been because of my programming, the idea of trying to debug another defective component was enough for me to sideline the feature. Secondly, I mentioned earlier how there is room for more motors on the back. If I were to continue with this project, this is the first addition I would make. It would require 3D printing a mount and motor-lego-gear interface, and wiring it to the exisitng motor driver. As well, the 7.4 volt battery pack should be able to support another motor without any modifications. Third, I would upgrade the controller's uLCD display asthetic to make it easier to read and provide more data such as bluetooth connection strength and battery charge (for both the controller and the car). Additionally, I would tune or replace the capicative keypad and buttons for a smoother, more responsive, user control interface. Another feature, as mentioned in my proposal, would be intalling an IMU on the car to report intertial data on the uLCD screen with a cool animation. The last feature I thought about adding is a microphone/speaker combo on the controller and car for two-way audio communication. Oh, I would also clean up the codebase. It is a little all over the place at the moment, with plenty of room for increased documentation and structure.

#### Conclusion
This project was a lot of fun for me. It required me to use skills in every area of embedded systems, from physical/mechanical aspects to power management to UI design. This required a type of creativity that is not common in academic technical spaces. Overall, I learned a lot from this project and I now have a comprehensive product that I can reference while searching for jobs to launch my professional career.
