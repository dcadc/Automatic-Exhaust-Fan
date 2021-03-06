# Automatic-Exhaust-Fan
A portable exhaust fan with sensing and auto-shutdown function   
Let's get rid of mushrooms growing in bathroom from now on!
   
 ![Automatic-Exhaust-Fan][closeup]   
   
## Features
* Avoid using AC adapter in a humid environment.   
* The circuit is simple enough to fit in a waterproof plastic food container.   
* Using a powerbank with low current shutoff function, the energy saving function can be simply implemented.   
* Built-in EEPROM Datalogger mode, measure your environment and change your own threshold value.   
* Could be built with DC-DC boost converter module LM2577 and generic 12V computer fans   
* Select your own parameter to be record or detect ( change in #define )
* Easily set up the turn-on mode through a 4pin mode jumper   
  ( Normal Mode / Serial debug on / Readout EEPROM / Clear EEPROM / Datalogger mode )   
  refer to the schematic below   
   
   
**Default Software Specifications**
   
|	 							|	Parameter		| Units |	Notes							|
|-------------------------------|:-----------------:|:-----:|-----------------------------------|
|	Arduino Board				|	Pro	Mini 16MHz	|	-	|	8MHz untested					|
|	Fan Refresh Cycle			|	4				|	sec	|	timer1	ISR						|
|	Data Logging Cycle			|	60				|	sec	|	timer1	ISR*15					|
|	DHT Sampling Cycle			|	1.5				|	sec	|	needs >1 second					|
|	Size of Average Data Set	|	10				|	-	|	only record the average data	|
   
   
**To be done**
* Adding a tiny I2C OLED (128*32) with sleep mode   
* Adding capacitive sensing pads / slider for a non-contact input interface    
* Adding a buzzer to notify when job is done   
* A modern but simple interface that doesn't need much intelligence while using   
   
   
## How to build one   

* Schematic :   
 ![Schematic][sch]   
   
   
* Hardware :   
 Follow the [schematic][sch-pdf] and [my photos][photos] , you can make it easily.   
   
* Software :   
 Open the sketch [/software/AutoExhaustFan][sw] using the Arduino IDE  
  - External libraries   
	 - [Sketch] → [Iclude library] → [Manage Libraries] and search for "DHT"  
	 - Select "DHT Sensor library by adafruit" and Click the [Install] button.  
	 - Another library could be found in  [Arduino Playground - DHT11Lib][libs] if you want to import it manually.   
  
  After installing the library, click the [upload] button on the IDE then you're done!   
  Remember to use the datalogger function and re-define your own threshold value.  
  
***
  
*Read this in other languages: [English](README.en.md), [正體中文](README.md).*  

   [sch]:		<https://rawgit.com/dcadc/Automatic-Exhaust-Fan/master/images/sch.png>
   [sch-pdf]:	<https://rawgit.com/dcadc/Automatic-Exhaust-Fan/master/hardware/AutoExhaustFan.pdf>
   [photos]:	<https://github.com/dcadc/Automatic-Exhaust-Fan/tree/master/images/photos>
   [closeup]:	<https://rawgit.com/dcadc/Automatic-Exhaust-Fan/master/images/photos/full_set.JPG>
   [sw]:		<https://github.com/dcadc/Automatic-Exhaust-Fan/tree/master/software/AutoExhaustFan>
   [libs]:		<https://playground.arduino.cc/Main/DHT11Lib>
