# Automatic-Exhaust-Fan
A portable exhaust fan with sensing and auto-shutdown function


**Features**
* Avoid using AC adapter in a humid environment.   
* The circuit is simple enough to fit in a waterproof plastic food container.   
* Using a powerbank with low current shutoff function, the energy saving function can be simply implemented.   
* Built-in EEPROM Datalogger mode, measure your environment and change your own threshold value.   
* Could be built with DC-DC boost converter module LM2577 and generic 12V computer fans   
* Select your own parameter to be record or detect ( change in #define )
* Easily set up the turn-on mode through a 4pin mode jumper   
  ( Normal Mode / Serial debug on / Readout EEPROM / Clear EEPROM / Datalogger mode )   
  refer to the schematic below   
  
**To be done**
* Adding a tiny I2C OLED (128*32) with sleep mode   
* Adding capacitive sensing pads / slider for a non-contact input interface    
* Adding a buzzer to notify when job is done   
* A modern but simple interface that doesn't need much intelligence while using   
  
**How to build one**
* Schematic :   
 ![Schematic][sch]   
   
* Hardware :   
 Follow the [schematic][sch-pdf] and [my photos][photos] , you can make it easily.   
   
* Software :   
 Open the schetch [/software/AutoExhaustFan][sw] using the Arduino IDE  
  - External libraries   
	 - [Sketch] → [Iclude library] → [Manage Libraries] and search for "DHT"  
	 - Select "DHT Sensor library by adafruit" and Click the [Install] button.  
	 - Another library could be found in  [Arduino Playground - DHT11Lib][libs] if you want to import it manually.   
  
  After installing the library, click the [upload] button on the IDE then you're done!   
  Remember to use the datalogger function and re-define your own threshold value.  
  
***
  
*Read this in other languages: [English](README.en.md), [正體中文](README.md).*  

   [sch]: <https://rawgit.com/dcadc/Automatic-Exhaust-Fan/master/images/sch.png>
   [sch-pdf]: <https://rawgit.com/dcadc/Automatic-Exhaust-Fan/master/hardware/AutoExhaustFan.pdf>
   [photos]: <https://github.com/dcadc/Automatic-Exhaust-Fan/tree/master/images/photos>
   [sw]: <https://github.com/dcadc/Automatic-Exhaust-Fan/tree/master/software/AutoExhaustFan>
   [libs]: <https://playground.arduino.cc/Main/DHT11Lib>
