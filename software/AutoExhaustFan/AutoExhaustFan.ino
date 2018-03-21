#include <EEPROM.h>
#include <dht11.h>

// Bitwise operation macro
#define _CONFIG(x)				(bit_is_set(config, x))
#define bit_is_set(sfr, bit)	(_SFR_BYTE(sfr) & _BV(bit))
#define cbi(sfr, bit)			(_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit)			(_SFR_BYTE(sfr) |= _BV(bit))
#define tbi(sfr, bit)			(_SFR_BYTE(sfr) ^= _BV(bit))	//for bit toggling
// Code / string conversion
#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

// Real PIN define
#define LED_PIN			13
#define LED_PIN_PORT	PORTB	// To accelerate the toggling time
#define LED_PIN_PBIT	5		// To accelerate the toggling time

#define DHT11_DATA_PIN	3
#define JP_PIN_1		4
#define JP_PIN_2		5
#define PCH_MOSFET_PIN	11

// Config Flags
#define TIMER__LED_SW	1		// Enable LED
#define PCH_MOSFET_SW	2		// Device to be controlled
#define SERIAL_DBG_JP	3		// Jumper: Enable debug message over serial
#define EEPRO_READ_JP	4		// Jumper: Readout from EEPROM
#define EEPROM_CLR_JP	5		// Jumper: Clear EEPROM
#define EEPROM_LOG_JP	6		// Jumper: Datalogger Mode
#define NO_JOB_SWITCH	8		// Jumper: No Job

#define DATASET_SIZE	10		// The amount of data to be averaged
#define DATA_THRESHOLD	28.0	// Threshold to turn off the fan
#define TURN_OFF_CONDITION		(data_array[DATASET_SIZE] < DATA_THRESHOLD)
#define VARIABLE_TO_DETECT	 	hum	//temp, hum, dew
#define BLANK_F_RECORD	0.0f	// The initial blank value in the data array or EEPROM
#define hi_Z			-1		// state detection being used b jumperDetect()


dht11 DHT11;

byte config = 0x00;				// Global config flags: control jobs directly through bits

//16000000/1024*4-1
const int 				top_timer1 = 62499;			// 4 sec when Prescaler == 1024
volatile unsigned long	msec_timer1;				// punch in time for timer1 ISR
unsigned long			msec_capture;				// punch in time for capture routine in main loop
unsigned long			msec_serial;				// punch in time for serial routine in main loop

float hum, tmp, dew;
float data_array[DATASET_SIZE+1] = {BLANK_F_RECORD};// +1 because the last element is for calculated value

short record_idx = 0; 			// EEPROM address pointer, being used by the record function
byte data_array_idx = 0; 		// only being used by the main loop capture routine


short hex_fill_zero(int hex, short z){
	// This function calculates the number of leading zeros in hex format string
	hex |= 1;
	while( hex && (--z > 0) )hex = hex >> 4;
	return z;
}

double dewPointFast(double humidity, double celsius){
	// Arduino Playground - DHT11Lib
	// https://playground.arduino.cc/Main/DHT11Lib
	// delta max = 0.6544 wrt dewPoint()
	// 6.9 x faster than dewPoint()
	// reference: http://en.wikipedia.org/wiki/Dew_point
	const double a = 17.271;
	const double b = 237.7;
	double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
	double Td = (b * temp) / (a - temp);
	return Td;
}

float dewTmp(float RH, float Tmp){
	// dewTmp function from ohiyooo2
	// http://ohiyooo2.pixnet.net/blog/post/398343103
	const float b=17.67;
	const float c=243.5;
	float gamma=log(RH/100)+b*Tmp/(c+Tmp);
	return c*gamma/(b-gamma);
}

void clear(){
	// clear EEPROM
	int eeAddress;
	float f = BLANK_F_RECORD;	// reset all value to -1
	for(eeAddress = 0; eeAddress < EEPROM.length(); eeAddress+=sizeof(f)){
		EEPROM.put(eeAddress, f);		
	}
	return 0;
}

void readout(){
	// readout the EEPROM in  CSV-like format	
	int eeAddress;
	float value = 0.00f;
	for(eeAddress = 0; eeAddress < EEPROM.length(); eeAddress+=sizeof(value)){
		EEPROM.get(eeAddress, value);
		Serial.print(eeAddress);
		Serial.print(F("\t"));
		Serial.print(value, 4);
		Serial.println();
	}
	dumpEEP();				//untested function, simulated in dev-c++ only
	return 0;
}

void dumpEEP(){
	// readout the EEPROM in HEX format	
	int eeAddress;
	byte value	= 0x00;
	byte col	= 16;
	String firstrow =  String(F("\nAddr:\t"));
	for(int i = 0; i < col; i++)
		firstrow = String(firstrow+'0' + String(i, HEX) + ' ');
	Serial.println(firstrow);
	for(eeAddress = 0; eeAddress < EEPROM.length(); eeAddress+=sizeof(value)){
		String Otherrows = "";
		if(!(eeAddress%col)){
			Otherrows = String("\n");
			for(int i = 0; i < hex_fill_zero(eeAddress, 4); i++ )
				Otherrows = String(Otherrows + '0');
			Otherrows = String(Otherrows + String(eeAddress, HEX)+'\t');
		}
		
		EEPROM.get(eeAddress, value);
		if(hex_fill_zero(value, 2))
			Otherrows = String(Otherrows + '0');
		Otherrows = String(Otherrows + String(value, HEX) + ' ');
		Serial.print(Otherrows);
	}
	return 0;
}

void record(){
	// log the captured data into EEPROM
	if(record_idx < EEPROM.length()){
		float value = data_array[DATASET_SIZE];
		EEPROM.put(record_idx, value);
		record_idx+=sizeof(value);
	}
	else{
		cbi(config, EEPROM_LOG_JP);
		cbi(config, TIMER__LED_SW);
		digitalWrite(11, HIGH);
	}
	return 0;
}

void average(){
	// average the data captured in the array and store the averaged value in the last element
	// this function ignores blank element when averaging
	byte avg_idx, blank_fields;
	data_array[DATASET_SIZE] = 0;
	blank_fields = 0;
	for(avg_idx = 0; avg_idx < DATASET_SIZE; avg_idx++){
		if(data_array[avg_idx] != BLANK_F_RECORD)
			data_array[DATASET_SIZE] += data_array[avg_idx];
		else
			blank_fields++;
	}
	data_array[DATASET_SIZE]/=(DATASET_SIZE-blank_fields);
	return 0;
}

void setMyTimerOne( ){
	cli();						// disable Global Interrupt
	TCCR1A = 0;
	TCCR1B = (1<<WGM12);		// CTC mode; Clear Timer on Compare
	
	// prescaler = CPU clock/1024 (101)
	sbi(TCCR1B, CS10);
	cbi(TCCR1B, CS11);
	sbi(TCCR1B, CS12);
	/////////
	
	OCR1A = top_timer1;			// TOP count for CTC, associated to prescaler value
	TCNT1=0;					// counter reset 
	
	sbi(TIMSK1, OCIE1A);		// enable CTC for TIMER1_COMPA_vect
	sei();						// enable Global Interrupt
}

short jumperDetect(byte pin){
	// detects the jumper in tri-state
	short result = -2;			// unexpected error code
	pinMode(pin, INPUT);		// caution: never switch to output
	if(digitalRead(pin))		// should be pulled down by external resistor which R > Pull-up resistor
	result = HIGH;				// low impedance HIGH level detected
	else{						// LOW level detected, determine the impedance
		pinMode(pin, INPUT_PULLUP);	// pull up the pin with low current, a voltage divider formed.
		if(digitalRead(pin))	// test if the level was pulled up
		result = hi_Z;			// impedance was easily dominated by the pull-up resistor, a floating ping detected
		else
		result = LOW;			// low impedance LOW level detected 
	}
	pinMode(pin, INPUT);		// disconnect the pull-up resistor
	return result;				// 1 for HIGH, 0 for LOW, -1 for high-Z(floating)
}

void jumperAssign(byte pin, byte L_job, byte H_job, byte Z_job){
	// reads the real jumper state and assign the config bit
	switch (jumperDetect(pin)){
	case HIGH:	sbi(config, L_job);
		break;
	case  LOW:	sbi(config, H_job);
		break;
	case hi_Z:	sbi(config, Z_job);
		break;
	default:	return -1;
		break;
	}
	return 0;
}

void configExecute(){
	// call the jumper-selected routine
	// initiate other switches
	if(_CONFIG(SERIAL_DBG_JP)){
		Serial.println(F("Welcome to the console!"));
		

#ifdef DHT11LIB_VERSION
		Serial.print(F("DHT11 LIBRARY VERSION: "));
		Serial.println(DHT11LIB_VERSION);
#endif
		Serial.print(F("D4: "));
		Serial.println(jumperDetect(4));
		Serial.print(F("D5: "));
		Serial.println(jumperDetect(5));
	}
	if(_CONFIG(EEPROM_CLR_JP)){
		clear();
		while(1);
	}
	if(_CONFIG(EEPRO_READ_JP)){
		if(_CONFIG(SERIAL_DBG_JP))readout();
		while(1);
	}
	if(_CONFIG(EEPROM_LOG_JP)){
		clear();
	}
	if(_CONFIG(PCH_MOSFET_SW)){
		digitalWrite(PCH_MOSFET_PIN, LOW);
	}
	return 0;
}

void setup(){
	pinMode(DHT11_DATA_PIN, INPUT);
	pinMode(JP_PIN_1, INPUT);
	pinMode(JP_PIN_2, INPUT);
	pinMode(LED_PIN, OUTPUT);
	pinMode(PCH_MOSFET_PIN, OUTPUT);	
	
	digitalWrite(PCH_MOSFET_PIN, HIGH);	//turn OFF Pch-MOSFET
	sbi(config, TIMER__LED_SW);
	sbi(config, PCH_MOSFET_SW);		//turn ON Pch-MOSFET

	//synchronize the config register and the jumper state
	jumperAssign(JP_PIN_1, EEPROM_CLR_JP, SERIAL_DBG_JP, NO_JOB_SWITCH);
	jumperAssign(JP_PIN_2, EEPRO_READ_JP, EEPROM_LOG_JP, NO_JOB_SWITCH);
	
	//setup timer1 for 4 second ISR use
	setMyTimerOne();
	
	Serial.begin(9600);
	
	//execute jumper selected routines, initialize config bit settings
	configExecute();
	msec_capture = millis();
	msec_serial = millis();
}

void loop(){
	if( millis() > msec_capture + 1500 ){
		msec_capture = millis();
		int chk = DHT11.read(DHT11_DATA_PIN);	
		
		if(_CONFIG(SERIAL_DBG_JP)){
			Serial.println(F("\n"));

#ifdef DHT11LIB_VERSION
			Serial.print(F("Read sensor: "));
			switch (chk){
			case DHTLIB_OK: 
				Serial.println(F("OK")); 
				break;
			case DHTLIB_ERROR_CHECKSUM: 
				Serial.println(F("Checksum error")); 
				break;
			case DHTLIB_ERROR_TIMEOUT: 
				Serial.println(F("Time out error")); 
				break;
			default: 
				Serial.println(F("Unknown error")); 
				break;
			}
#endif
		}

		hum = DHT11.humidity;
		tmp = DHT11.temperature;
		dew = dewPointFast(tmp, hum);
		
		data_array[data_array_idx] = VARIABLE_TO_DETECT;
		data_array_idx++;
		if(data_array_idx > (DATASET_SIZE-1))data_array_idx = 0;
		
		average();
		
	}

	if(_CONFIG(SERIAL_DBG_JP)){
		if( millis() > msec_serial + 1500 ){
		msec_serial = millis();
		Serial.print(F("D4: \t\t\t"));
		Serial.println(jumperDetect(4));
		Serial.print(F("D5: \t\t\t"));
		Serial.println(jumperDetect(5));
		
		Serial.print(F("Humidity (%): \t\t"));
		Serial.println((float)hum, 2);
		Serial.print(F("Temperature (C): \t"));
		Serial.println((float)tmp, 2);
		Serial.print(F("Dew Point (C): \t\t"));
		Serial.println((float)dew, 2);
		Serial.print(F("Averaged "));
		Serial.print(F(STR(VARIABLE_TO_DETECT)));
		Serial.print(F(": \t\t"));
		Serial.println(data_array[DATASET_SIZE], 2);
		
		Serial.print(F("OUTPUT: \t\t"));
		Serial.println(bit_is_set(PORTB, 3)?"OFF":"ON");
		Serial.print(F("Detecting parameter: \t\t"));
		Serial.println(F(STR(VARIABLE_TO_DETECT)));
		Serial.print(F("Turn-off condition: \t\t"));
		Serial.println(F(STR(TURN_OFF_CONDITION)));
		Serial.print(F(">>Last Capture Time: \t\t"));
		Serial.println(msec_capture);
		Serial.print(F(">>Last Interrupt Time: \t\t"));
		Serial.println(msec_timer1);
		}	
	}



}
ISR(TIMER1_COMPA_vect){
	static unsigned int count15 = 0;
	msec_timer1 = millis();									//punch in 
	++count15;												// 4 sec * 15 times = 1 minute
	if(count15 == 15){
		if(_CONFIG(EEPROM_LOG_JP))record();					// log data down
		count15=0;
	}	
	if(_CONFIG(TIMER__LED_SW))
		tbi(LED_PIN_PORT, LED_PIN_PBIT);					// Toggle the LED, PB5 = Arduino pin 13
	
	if(_CONFIG(PCH_MOSFET_SW))
		if(!_CONFIG(EEPROM_LOG_JP))	// to avoid switching off the fan in logging mode
			if(TURN_OFF_CONDITION)		// condition to turn the fan off
				digitalWrite(PCH_MOSFET_PIN, HIGH);
}