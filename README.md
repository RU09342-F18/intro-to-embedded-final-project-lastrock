#Problem
For people who have difficulty gauging the distance between the car and the object behind. Difficulty seeing behind with a large load in the trunk, or just hard to see the object behind you. 
#Solution
This project provides a simple way for a user to implement a backup sensor. This sensor would be relatively cheap and simple to set up in a vehicle.Making it the best choice for old vehicles since they usually do not have the capabilities to install back up cameras without a 1000$ investment.
# How it works
A ultrasonic sensor is placed near the back of the car preferably near the license plate where it can have clear visible access to any objects behind the vehicle. This sensor would use waves to see how far an object is then send it to the MSP430. Once the MSP430 processed that data it will be transmitted to the ESP8266 to be wirelessly transmitted to an MQTT server to be transmitted to another ESP8266 which would be seated near the driver as an indicator. This ESP8266 would be connected to set of 3 LEDs that would server as the indicators that show close the driver would be to an object. It will blink brighter and faster as you approach closer to the object.
# How to use it
All the components you will need for this design is a Ultrasonic sensor, MSP430F5529, MSP430G2553, Voltage divide, 2x ESP8266,
3x LEDs, and a breadboard. 
