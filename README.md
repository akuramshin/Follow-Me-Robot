# Follow-Me-Robot

This is a senior highschool project for an autonomous robot that will follow you (more your position) upon the click of a button. The main idea is that the user will have a smart-phone that will send its GPS coordinates over bluetooth to the robot, who will, with the use of a compas and a GPS module, drive to the received coordinates. 

Room for improvement would be to implement a way to make a constant stream of GPS coordinates to the robot (currently we send coordinates only when the button is pressed). Additionally, the robot has no knowledge of its surrounding so it will not avoid obstacles.

Usefull links I used to put together this project: 
  - https://www.hackster.io/hackerhouse/make-an-autonomous-follow-me-cooler-7ca8bc
  - https://developer.android.com/guide/topics/location/strategies.html
  - http://www.instructables.com/id/Android-Bluetooth-Control-LED-Part-2/
  - http://forum.arduino.cc/index.php?topic=91707.0

For this project I used:
  - AndyMark Tilerunner robot. Really any sort of robot will do.
  - Arduino Uno to controll the robot. The Arduino Uno is very cheap (especially the clones) and has all the capabilities required for         the project.
  - 2 vex pro victor 888 motor controllers. I only used these controllers because I had them lying around and they only require 1 PWN         pin to operate a motor (compared to 1 PWN and 2 other pins for an L298 controller). I do not recommend these controllers for this         particular project as they are really bulky.
  - The GPS reciever I used for the robot is the EM-506 (48 Channel). This reciever is quite pricey and the connector is too small for         jumper wires to fit, so I had to cut off the end and solder jumper wires to each connection individually. Nevertheless, the reciever       proved to lock on fast even indoors and is quite accurate.
  - I used the Wireless 4 Pin Bluetooth RF Transceiver Module RS232 With Backplane as the bluetooth module for my robot.
  - For the compass I bought the Triple-axis Accelerometer+Magnetometer (Compass) Board - LSM303. This module includes both and               accelerometer and magnetometer, but I only used the magnetometer pins from the board.
  - As the power source for my motors I used the AJC® Brand 12 Volt 7Ah Sealed Lead Acid Battery. This battery is quite large due to its       large capacity and is a bit overkill, but I had it available and it fit on my robot chassis.
  - To power the arduino I used a portable battery bank, like the one you can use to charge your phone.
  - I used an Android phone to run my app since I coded the app in Android Studios. The newer the phone, the better the GPS coordinates       provided by the phone will be.
    
The Defenitions.h file is where I defined all of my pins so change the values accordingly. FollowMe.ino is the source code for the Arduino for the movement. The "src" folder contains the java code and manifest files for the Android Studio app code.
