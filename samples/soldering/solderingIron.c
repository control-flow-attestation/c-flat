/*
 * Copyright (c) 2016 Aalto University
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "kernel/init.h"
#include "kernel/led.h"
#include "kernel/log.h"
#include "kernel/misc.h"
#include "kernel/perf.h"
#include "sm/smcall.h"
#include "sm/sm.h"
#include "arm.h"

#include "constants.h"
#include "serial.h"
#include "arduino.h"

#include "Iron.h"
#include "Display.h"
#include "Button.h"
#include "Buzzer.h"
#include "Config.h"
#include "Encoder.h"
#include "Screen.h"
#include "ScreenMain.h"
#include "ScreenWork.h"
#include "ScreenError.h"
#include "ScreenPower.h"
#include "ScreenConfig.h"
#include "ScreenTune.h"

#include "lib/cfa_stub.h"

void createGlobals(void);
void setup(void);
void loop(void);
void rotEncChange(void);
void rotPushChange(void);

// These are additions to debug the Pi stuff
void debug_printScreenType(void *const p);
void debug_printScreenSwitch(char *const string, void *const from, void *const to);

static uint8_t user_data[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
static uint8_t quote_out[128];
static uint32_t quote_len;

extern btbl_entry_t __btbl_start;
extern btbl_entry_t __btbl_end;
extern ltbl_entry_t __ltbl_start;
extern ltbl_entry_t __ltbl_end;

void main(unsigned int r0, unsigned int r1, unsigned int atags)
{
    (void)r0;
    (void)r1;
    (void)atags;

    // run global "constructors"
    createGlobals();

    setup();
    while(1) {
        loop();
    }
}

//=================================== End of class declarations ================================================

Display disp;
Encoder rotEncoder;
Button rotButton;
Iron iron;

Config ironCfg;
Buzzer simpleBuzzer;

ScreenMain offScr;
ScreenWork wrkScr;
ScreenError errScr;
ScreenPower powerScr;
ScreenConfig cfgScr;
ScreenTune tuneScr;

void *pCurrentScreen = &offScr;

void createGlobals(void) {
    createDisplay(&disp, M_DIN, M_CLK, M_CS, M_INTENSITY);
    createEncoderD(&rotEncoder, R_MAIN_PIN, R_SECD_PIN);
    createButtonD(&rotButton, R_BUTN_PIN);
    createIron(&iron, HEATERPIN, PROBEPIN);

    createConfig(&ironCfg);
    createBuzzer(&simpleBuzzer, BUZZERPIN);

    createScreenMain(&offScr, &iron, &disp, &rotEncoder, &simpleBuzzer, &ironCfg);
    createScreenWork(&wrkScr, &iron, &disp, &rotEncoder, &simpleBuzzer);
    createScreenError(&errScr, &disp);
    createScreenPower(&powerScr, &iron, &disp, &rotEncoder);
    createScreenConfig(&cfgScr, &iron, &disp, &rotEncoder, &ironCfg);
    createScreenTune(&tuneScr, &iron, &disp, &rotEncoder, &simpleBuzzer, &ironCfg);
}

// the setup routine runs once when you press reset:
void setup() {
    initDisplay(&disp);
    clear(&disp);

    // Load configuration parameters
    initConfig(&ironCfg);
    bool is_cfg_valid = load(&ironCfg);

    uint16_t temp_min, temp_max;
    getCalibrationData(&ironCfg, &temp_max, &temp_min);

    if (is_cfg_valid) {
        byte dBright = getBrightness(&ironCfg);
        brightness(&disp, dBright);
    }

    initIron(&iron, temp_max, temp_min);
    uint16_t temp = Config_temp(&ironCfg);
    setTemp(&iron, temp);

    // Initialize rotary encoder
    initButton(&rotButton);
    delay(500);
    attachInterrupt(digitalPinToInterrupt(R_MAIN_PIN), rotEncChange,   CHANGE);
    attachInterrupt(digitalPinToInterrupt(R_BUTN_PIN), rotPushChange,  CHANGE);

    // Initialize SCREEN hierarchy
    offScr.parent.next    = (void*) &wrkScr;
    offScr.parent.nextL   = (void*) &cfgScr;
    wrkScr.parent.next    = (void*) &offScr;
    wrkScr.parent.nextL   = (void*) &powerScr;
    errScr.parent.next    = (void*) &offScr;
    errScr.parent.nextL   = (void*) &offScr;
    powerScr.parent.nextL = (void*) &wrkScr;
    cfgScr.parent.next    = (void*) &tuneScr;
    cfgScr.parent.nextL   = (void*) &offScr;
    cfgScr.parent.main    = (void*) &offScr;
    tuneScr.parent.nextL  = (void*) &cfgScr;

    initScreen(pCurrentScreen);
}

void rotEncChange(void) {
    //serialWriteString("rotEncChange triggered\n\r");
    changeEncoderINTR(&rotEncoder);
}

void rotPushChange(void) {
    //serialWriteString("rotPushChange triggered\n\r");
    changeButtonINTR(&rotButton);
}

// The main loop
void loop() {
    int16_t old_pos = read(&rotEncoder);
    bool iron_on = isOn(&iron);

    if ((pCurrentScreen == &wrkScr) && !iron_on) {  // the soldering iron failed
        serialWriteString("failure\r\n");
        debug_printScreenSwitch("failure", pCurrentScreen, (void*)&errScr);
        pCurrentScreen = &errScr;
        initScreen(pCurrentScreen);
    }

    void* nxt = returnToMain(pCurrentScreen);
    if (nxt != pCurrentScreen) {                // return to the main screen by timeout
        serialWriteString("timeout return\r\n");
        pCurrentScreen = nxt;
        initScreen(pCurrentScreen);
    }

    byte bStatus = intButtonStatus(&rotButton);

    switch (bStatus) {
        case 2:                                   // long press;
            nxt = menu_long(pCurrentScreen);

            if (nxt != pCurrentScreen) {
                debug_printScreenSwitch("long press", pCurrentScreen, nxt);
                pCurrentScreen = nxt;
                initScreen(pCurrentScreen);
            } else {
                if (isSetup(pCurrentScreen)) {
                    resetTimeout(pCurrentScreen);
                }
            }
            break;
        case 1:                                   // short press
            nxt = menu(pCurrentScreen);
            if (nxt != pCurrentScreen) {
                debug_printScreenSwitch("short press", pCurrentScreen, nxt);
                pCurrentScreen = nxt;
                initScreen(pCurrentScreen);
            } else {
                if (isSetup(pCurrentScreen)) {
                    resetTimeout(pCurrentScreen);
                }
            }
            break;
        case 0:                                   // Not pressed
        default:
            break;
    }

    // PORT: Emulation tick must be done here to ensure rotEncoder changes between pos and oldPos
    // reads!
    emulate_tick();

    int16_t pos = read(&rotEncoder);
    if (old_pos != pos) {
        rotaryValue(pCurrentScreen, pos);
        old_pos = pos;
        if (isSetup(pCurrentScreen)) {
            resetTimeout(pCurrentScreen);
        }
    }

    showScreen(pCurrentScreen);

    
    cfa_init((cfa_addr_t)&loop, (cfa_addr_t)0,
            (btbl_entry_t*)&__btbl_start, (btbl_entry_t*)&__btbl_end,
            (ltbl_entry_t*)&__ltbl_start, (ltbl_entry_t*)&__ltbl_end);

    perf_cc_reset();
    perf_cc_enable();

    keepTemp(&iron);

    perf_cc_disable();

    if (perf_cc_overflow()) {
        info("invalid measurement, cycle counter overflowed");
    } else {
        info("cycle count: %u", perf_cc_read());
    }

    quote_len = sizeof(quote_out);
    cfa_quote(user_data, sizeof(user_data), quote_out, &quote_len);

    Display_show(&disp);
    delay(10);

    // PORT: Advance and display all the emulated stuff here
    if (emulate_State(&disp.parent, bStatus, &rotEncoder, &iron)) {
        serialWriteString("  (cur scr: ");
        debug_printScreenType(pCurrentScreen);
        serialWriteString(")");
        serialWriteNewLine();
    }
}

void debug_printScreenSwitch(char *const string, void *const from, void *const to) {
    serialWriteString("Switching: ");
    debug_printScreenType(from);
    serialWriteString("-->");
    serialWriteString(string);
    serialWriteString("-->");
    debug_printScreenType(to);
    serialWriteNewLine();
}

void debug_printScreenType(void *const p) {
    if (p == (void*)&offScr) {
        serialWriteString("ScreenMain");
    } else if (p == (void*)&wrkScr) {
        serialWriteString("ScreenWork");
    } else if (p == (void*)&errScr) {
        serialWriteString("ScreenErr");
    } else if (p == (void*)&powerScr) {
        serialWriteString("ScreenPower");
    } else if (p == (void*)&cfgScr) {
        serialWriteString("ScreenConfig");
    } else if (p == (void*)&tuneScr) {
        serialWriteString("ScreenTune");
    } else {
        serialWriteString("Unknown");
    }
}

