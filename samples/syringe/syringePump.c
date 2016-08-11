//Open Syringe Pump
//https://github.com/naroom/OpenSyringePump/blob/master/syringePump/syringePump.ino
//https://hackaday.io/project/1838-open-syringe-pump

// Controls a stepper motor via an LCD keypad shield.
// Accepts triggers and serial commands.
// To run, you will need the LCDKeypad library installed - see libraries dir.

// Serial commands:
// Set serial baud rate to 57600 and terminate commands with newlines.
// Send a number, e.g. "100", to set bolus size.
// Send a "+" to push that size bolus.
// Send a "-" to pull that size bolus.

#include "LiquidCrystal.h"
#include "util.h"
#include "kernel/kprintf.h"
#include "kernel/log.h"
#include "kernel/led.h"

#include "lib/cfa_stub.h"


/* -- Constants -- */
#define SYRINGE_VOLUME_ML 30.0
#define SYRINGE_BARREL_LENGTH_MM 80.0

#define THREADED_ROD_PITCH 1.25
#define STEPS_PER_REVOLUTION 200.0
#define MICROSTEPS_PER_STEP 16.0

#define SPEED_MICROSECONDS_DELAY 100 //longer delay = lower speed

#define	false	0
#define	true	1

#define	boolean	_Bool
#define three_dec_places( x ) ( (int)( (x*1e3)+0.5 - (((int)x)*1e3) ) )

long ustepsPerMM = MICROSTEPS_PER_STEP * STEPS_PER_REVOLUTION / THREADED_ROD_PITCH;
long ustepsPerML = (MICROSTEPS_PER_STEP * STEPS_PER_REVOLUTION * SYRINGE_BARREL_LENGTH_MM) / (SYRINGE_VOLUME_ML * THREADED_ROD_PITCH );

/* -- Pin definitions -- */
int motorDirPin = 2;
int motorStepPin = 3;

int triggerPin = 0;    //TODO check RPi pin-out before implementing
int bigTriggerPin = 0; //TODO check RPi pin-out before implementing

/* -- Keypad states -- */
int  adc_key_val[5] ={30, 150, 360, 535, 760 };

enum{ KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_SELECT, KEY_NONE};
int NUM_KEYS = 5;
int adc_key_in;
int key = KEY_NONE;

/* -- Enums and constants -- */
enum{PUSH,PULL}; //syringe movement direction
enum{MAIN, BOLUS_MENU}; //UI states

enum{INPUT, OUTPUT}; //GPIO directions
enum{HIGH, LOW}; //GPIO states

const int mLBolusStepsLength = 9;
float mLBolusSteps[9] = {0.001, 0.005, 0.010, 0.050, 0.100, 0.500, 1.000, 5.000, 10.000};

/* -- Default Parameters -- */
float mLBolus = 0.500; //default bolus size
float mLBigBolus = 1.000; //default large bolus size
float mLUsed = 0.0;
int mLBolusStepIdx = 3; //0.05 mL increments at first
//float mLBolusStep = mLBolusSteps[mLBolusStepIdx];
float mLBolusStep = 0.050;

long stepperPos = 0; //in microsteps
char charBuf[16];

//debounce params
long lastKeyRepeatAt = 0;
long keyRepeatDelay = 400;
long keyDebounce = 125;
int prevKey = KEY_NONE;
        
//menu stuff
int uiState = MAIN;

//triggering
int prevBigTrigger = HIGH;
int prevTrigger = HIGH;

//serial
char serialStr[80] = "";
boolean serialStrReady = false;
int serialStrLen = 0;

/* -- Initialize libraries -- */
LiquidCrystal lcd;

void setup(){
	/* LCD setup */
	lcd_begin(&lcd, 16, 2);
	lcd_clear(&lcd);

	lcd_print(&lcd, "SyringePump v2.0", 16);

	/* Triggering setup */
	pinMode(triggerPin, INPUT);
	pinMode(bigTriggerPin, INPUT);
	digitalWrite(triggerPin, HIGH); //enable pullup resistor
	digitalWrite(bigTriggerPin, HIGH); //enable pullup resistor
  
	/* Motor Setup */
	pinMode(motorDirPin, OUTPUT);
	pinMode(motorStepPin, OUTPUT);
  
	/* Serial setup */
	//Note that serial commands must be terminated with a newline
	//to be processed. Check this setting in your serial monitor if
	//serial commands aren't doing anything.
	Serial_begin(57600); //Note that your serial connection must be set to 57600 to work!
}

void checkTriggers();
void readSerial();
void processSerial();
void bolus(int direction);
void readKey();
void doKeyAction(unsigned int key);
void updateScreen();
int get_key(unsigned int input);
unsigned int tim;


static uint8_t user_data[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
static uint8_t quote_out[128];
static uint32_t quote_len;

extern btbl_entry_t __btbl_start;
extern btbl_entry_t __btbl_end;
extern ltbl_entry_t __ltbl_start;
extern ltbl_entry_t __ltbl_end;

void loop(){

	//check for LCD updates
	readKey();

	//look for triggers on trigger lines
	checkTriggers();

	//check serial port for new commands
	readSerial();
	if(serialStrReady){

		cfa_init((cfa_addr_t)&loop, (cfa_addr_t)0,
					(btbl_entry_t*)&__btbl_start, (btbl_entry_t*)&__btbl_end,
					(ltbl_entry_t*)&__ltbl_start, (ltbl_entry_t*)&__ltbl_end);

		processSerial();

		quote_len = sizeof(quote_out);
		cfa_quote(user_data, sizeof(user_data), quote_out, &quote_len);
	}
}

void checkTriggers(){
	//check low-reward trigger line
	int pushTriggerValue = digitalRead(triggerPin);
	if(pushTriggerValue == HIGH && prevTrigger == LOW){
		bolus(PUSH);
		updateScreen();
	}
	prevTrigger = pushTriggerValue;
    
	//check high-reward trigger line
	int bigTriggerValue = digitalRead(bigTriggerPin);
	if(bigTriggerValue == HIGH && prevBigTrigger == LOW){
		//push big reward amount
		float mLBolusTemp = mLBolus;
		mLBolus = mLBigBolus;
		bolus(PUSH);
		mLBolus = mLBolusTemp;

		updateScreen();
	}
	prevBigTrigger = bigTriggerValue;
}

void readSerial(){
	//pulls in characters from serial port as they arrive
	//builds serialStr and sets ready flag when newline is found
	while (Serial_available()) {
		char inChar = (char)Serial_read();
		if (inChar < 0x20) {
			serialStrReady = true;
		}
		else{
			serialStr[serialStrLen] = inChar;
			serialStrLen++;
		}
	}
}

void processSerial(){
	//process serial commands as they are read in
	if(serialStr[0] == '+'){
		bolus(PUSH);
		updateScreen();
	}
	else if(serialStr[0] == '-'){
		bolus(PULL);
		updateScreen();
	}
	else if(toUInt(serialStr, serialStrLen) != 0){
		int uLbolus = toUInt(serialStr, serialStrLen);
		mLBolus = (float)uLbolus / 1000.0;
		updateScreen();
	}
	else{
		Serial_write("Invalid command: [", 18);
		Serial_write(serialStr, serialStrLen);
		Serial_write("]\n", 2);
	}
	serialStrReady = false;
	serialStrLen = 0;
}

void bolus(int direction){
	//Move stepper. Will not return until stepper is done moving.
  
	//change units to steps
	long steps = (mLBolus * ustepsPerML);
	if(direction == PUSH){
		led_on();
		digitalWrite(motorDirPin, HIGH);
		steps = mLBolus * ustepsPerML;
		mLUsed += mLBolus;
	}
	else if(direction == PULL){
		led_off();
		digitalWrite(motorDirPin, LOW);
		if((mLUsed-mLBolus) > 0){
			mLUsed -= mLBolus;
		}
		else{
			mLUsed = 0;
		}
	}	

	float usDelay = SPEED_MICROSECONDS_DELAY; //can go down to 20 or 30
    
	for(long i=0; i < steps; i++){
		digitalWrite(motorStepPin, HIGH);
		delayMicroseconds(usDelay);
    
		digitalWrite(motorStepPin, LOW);
		delayMicroseconds(usDelay);
	}

}

void readKey(){
	//Some UI niceness here. 
	//When user holds down a key, it will repeat every so often (keyRepeatDelay).
	//But when user presses and releases a key,
	//the key becomes responsive again after the shorter debounce period (keyDebounce).

	adc_key_in = analogRead(0);
	key = get_key(adc_key_in); // convert into key press

	long currentTime = millis();
	long timeSinceLastPress = (currentTime-lastKeyRepeatAt);
        
	boolean processThisKey = false;
	if (prevKey == key && timeSinceLastPress > keyRepeatDelay){
		processThisKey = true;
	}
	if(prevKey == KEY_NONE && timeSinceLastPress > keyDebounce){
		processThisKey = true;
	}
	if(key == KEY_NONE){
		processThisKey = false;
	}
        
	prevKey = key;
        
	if(processThisKey){
		doKeyAction(key);
		lastKeyRepeatAt = currentTime;
	}
}

void doKeyAction(unsigned int key){
	if(key == KEY_NONE){
		return;
	}

	if(key == KEY_SELECT){
		if(uiState == MAIN){
			uiState = BOLUS_MENU;
		}
		else if(BOLUS_MENU){
			uiState = MAIN;
		}
	}

	if(uiState == MAIN){
		if(key == KEY_LEFT){
			bolus(PULL);
		}
		if(key == KEY_RIGHT){
			bolus(PUSH);
		}
		if(key == KEY_UP){
			mLBolus += mLBolusStep;
		}
		if(key == KEY_DOWN){
			if((mLBolus - mLBolusStep) > 0){
				mLBolus -= mLBolusStep;
			}
			else{
				mLBolus = 0;
			}
		}
	}
	else if(uiState == BOLUS_MENU){
		if(key == KEY_LEFT){
			//nothin'
		}
		if(key == KEY_RIGHT){
			//nothin'
		}
		if(key == KEY_UP){
			if(mLBolusStepIdx < mLBolusStepsLength-1){
				mLBolusStepIdx++;
				mLBolusStep = mLBolusSteps[mLBolusStepIdx];
			}
		}
		if(key == KEY_DOWN){
			if(mLBolusStepIdx > 0){
				mLBolusStepIdx -= 1;
				mLBolusStep = mLBolusSteps[mLBolusStepIdx];
			}
		}
	}

	updateScreen();
}

void updateScreen(){
	//build strings for upper and lower lines of screen
	char s1[80]; //upper line
	char s2[80]; //lower line
	int s1Len = 0;
	int s2Len = 0;
	
	if(uiState == MAIN){
		s1Len = sprintf(s1, "Used %d.%d mL", (int)mLUsed, three_dec_places(mLUsed));
		s2Len = sprintf(s2, "Bolus %d.%d mL", (int)mLBolus, three_dec_places(mLBolus));
	}
	else if(uiState == BOLUS_MENU){
		s1Len = sprintf(s1, "Menu> BolusStep");
		s2Len = sprintf(s2, "%d.%d", (int)mLBolusStep, three_dec_places(mLBolusStep));
	}

	//do actual screen update
	lcd_clear(&lcd);

	lcd_setCursor(&lcd, 0, 0);  //line=1, x=0
	lcd_print(&lcd, s1, s1Len);

	lcd_setCursor(&lcd, 0, 1);  //line=2, x=0
	lcd_print(&lcd, s2, s2Len);
}


// Convert ADC value to key number
int get_key(unsigned int input){
	int k;
	for (k = 0; k < NUM_KEYS; k++){
		if (input < (unsigned int)adc_key_val[k]){
    		return k;
		}
	}
	if (k >= NUM_KEYS){
	  k = KEY_NONE;     // No valid key pressed
	}
	return k;
}

/*
String decToString(float decNumber){
	//not a general use converter! Just good for the numbers we're working with here.
	int wholePart = decNumber; //truncate
	int decPart = round(abs(decNumber*1000)-abs(wholePart*1000)); //3 decimal places
		String strZeros = String("");
		if(decPart < 10){
			strZeros = String("00");
		}
		else if(decPart < 100){
			strZeros = String("0");
		}
	return String(wholePart) + String('.') + strZeros + String(decPart);
}
*/

//C-FLAT new code
void main(unsigned int r0, unsigned int r1, unsigned int atags) {
	(void)r0;
	(void)r1;
	(void)atags;
	info("Starting syringe pump");
	setup();
	while(1) {
		loop();
	}
}
