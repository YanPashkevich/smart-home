# Smart home system
___
### Smart home system device for controlling a relay (for example, turning on a light) and monitoring the door opening status using a reed switch

The state of the system, as well as its control, occurs using the Telegram messenger

A modification is planned using a voice assistant developed in Python
___
>Device used: NodeMcu V3.0 (esp8266)

**NodeMCUv3.0.ino** - upload to board using Arduino IDE

**commands.txt** - the file contains commands and their descriptions for control system (for reading by Telegram bot)

**links.txt** - the file contains links that can help when repeating the project
___
### Required Libraries
- ESP8266WiFi (for work with WiFi NodeMcu V3.0 (esp8266))
- UniversalTelegramBot (for work with Telegram bot)
___
### WARING
#### **A resistor is needed in the circuit, at which the state of the reed switch will be read by NodeMcu V3.0 (depends on the length of the wires to this connected reed switch)**

>**My system did not work at 10 kOhm (R on scheme). System started at 5 kOhm and below**

![Logo](https://github.com/YanPashkevich/smart-home/blob/master/Scheme.png?raw=true "sheme")

