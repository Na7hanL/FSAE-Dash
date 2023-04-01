#include "dash.h"
#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>
#include "CFA10100_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include <iostream>
#include <Bounce2.h>
#include <FlexCAN_T4.h>
//#include <String>

uint16_t b1x1;
uint16_t b1x2;
uint16_t b1Bound;
uint16_t b2x1;
uint16_t b2x2;
uint16_t b2Bound;
uint16_t b3x1;
uint16_t b3x2;
uint16_t b3Bound;
uint16_t dashY1;
uint16_t dashY2;
uint16_t barRpm;

uint16_t greenR;
uint16_t greenG;
uint16_t greenB;
uint16_t redR;
uint16_t redG;
uint16_t redB;
uint16_t yellowR;
uint16_t yellowG;
uint16_t yellowB;

uint16_t rpm;
uint16_t bat;
uint16_t afrtgt;
uint16_t clt;
uint16_t afr;
uint16_t gear;
uint16_t spd;
uint16_t sync;

static CAN_message_t txmsg, rxmsg;

bool back = false;
bool testing = true;
int printRPM;
int gear_test_num = 0;

// other variables
const int U_L_RPM = 5000; //upper limit RPM for down shift rejection
const int DB_TIME = 40; //debounce time for paddles in ms
const byte REJTRY = 3; //how many tries it takes to shift incase of fail
const byte upShiftTime = 100; //100 ms
const byte dnShiftTime = 100; //100 ms
const byte nuShiftTime = 30; //30 ms

bool rejectFlag = false;
bool upState, dnState = true;
bool upNewState, dnNewState = true;
byte shiftTry = 1;
byte oldGear = 0;

Bounce upPaddle = Bounce();
Bounce dnPaddle = Bounce();

// right side outputs
const byte PUL = 15; //A1 or SEN1 in schematic
const byte DIR = 18; //A4 or SEN2 in schematic

// screen is 800px wide; 1rpm = 800/10500 px
// convert then use;

// need to take in rpm and represent it as 800/10500, then round down in pixel calculations
// every time a can message is received, sniff it and make sure it's something we want
void canSniff(const CAN_message_t &msg) {

  // switch statement better than nested ifs
  switch(msg.id) { // ID's 1520+ are Megasquirt CAN broadcast frames

    case 1520: // group 0
      rpm = (int)(word(msg.buf[6], rxmsg.buf[7]));
    break;
    
    case 1521: // group 1
      afrtgt = (int)(word(0x00, msg.buf[4]));
    break;

    case 1522: // group 2
      clt = (int)(word(msg.buf[6], msg.buf[7]));
    break;

    case 1523: // group 3
      bat = (int)(word(msg.buf[2], msg.buf[3]));
    break;

    case 1551: // group 31
      afr = (int)(word(0x00, msg.buf[0]));
    break;

    case 1553: // group 33
      gear = (int)(word(0x00, msg.buf[6]));
    break;

    case 1562: // group 42
      spd = (int)(word(msg.buf[0], msg.buf[1]));
    break;

    case 1563: // group 43
      sync = (int)(word(0x00, msg.buf[0]));
    break;

    default: // in the case of not a broadcast packet
    break;
  }
}


void Initialize_Dash(void)
{

    dashY1 = 0;
    dashY2 = 150;
    barRpm = 0;
    b1x1 = 0;
    b1x2 = 0;
    b1Bound = round(LCD_WIDTH * 0.61);
    b2x1 = 0;
    b2x2 = 0;
    b2Bound = round(LCD_WIDTH * 0.86);
    b3x1 = 0;
    b3x2 = 0;
    b3Bound = LCD_WIDTH;
    greenR = 32;
    greenG = 252;
    greenB = 3;
    redR = 252;
    redG = 40;
    redB = 3;
    yellowR = 248;
    yellowG = 252;
    yellowB = 13;

    rpm = 0;
    bat = 0;
    afrtgt = 0;
    clt = 0;
    afr = 0;
    gear = 0;
    spd = 0;
    sync = 0;

    


    // All this is from the from Shifter Code

    // left side inputs
    const byte SFT_UP = 16; //A2 or PDU in schematic
    const byte SFT_DN = 17; //A3 or PDD in schematic


    // moved this to the top to make sure it is in scope of cansniff
    // static CAN_message_t txmsg, rxmsg;


    FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;

    // set up shifting buttons
    upPaddle.attach(SFT_UP, INPUT_PULLUP);
    dnPaddle.attach(SFT_DN, INPUT_PULLUP);
    upPaddle.interval(DB_TIME);
    dnPaddle.interval(DB_TIME);

    // set up output pins
    pinMode(PUL, OUTPUT);
    pinMode(DIR, OUTPUT);

    // set up can bus
    can1.begin();
    can1.setBaudRate(500000);
    can1.enableMBInterrupts();  
    can1.onReceive(canSniff);

}


void updateData(void)
{

    /*
    if(rpm < b1Bound){
        b1x2 = rpm;
        b2x2 = b1Bound;
        b3x2 = b2Bound;
    }
    else if(rpm >= b1Bound && rpm < b2Bound){
            b1x2 = b1Bound;
            b3x2 = b2Bound;
            b2x2 = rpm - b1Bound;
    }
    else if(rpm >= b2Bound){
        b1x2 = b1Bound;
        b2x2 = b2Bound;
        b3x2 = rpm - b1Bound - b2Bound;
    }
    */

    // updates bars in first rpm range
    if (barRpm <= b1Bound)
    {
        b1x2 = barRpm;
        b2x2 = b1Bound;
    }

    // updates bars in second rpm range
    if (barRpm <= b2Bound)
    {
        b2x2 = barRpm;
        b3x2 = b2Bound;
    }
    else if (barRpm > b2Bound)
    {
        b2x2 = b2Bound;
    }
    else
    {
        b2x2 = b1Bound;
    }

    // updates bars in third rpm range
    if (barRpm > b2Bound)
    {
        b3x2 = barRpm;
    }
    else
    {
        b3x2 = b2Bound;
    }

    rpm = barRpm * 10500 / 800;

    if (testing)
    {
        if (barRpm > LCD_WIDTH)
        {
            back = true;
            if(gear_test_num < 5)
            {
                gear_test_num += 1;
            }
            else
            {
                gear_test_num = 0;
            }
        }
        if (barRpm <= 0)
        {
            back = false;
        }
        if (!back)
        {
            barRpm += 10;
        }
        else
        {
            barRpm = 40;
            back = false;
        }

        
    }
}

uint16_t Add_Diag_To_Display_List(uint16_t FWol){
    
    FWol = EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_RGB(greenR, greenG, greenB));
    
    FWol = EVE_Filled_Rectangle(FWol, 50, 50, LCD_WIDTH - 60, LCD_HEIGHT - 60);

    FWol = EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_RGB(0, 0, 0));

    FWol = EVE_Filled_Rectangle(FWol, 55, 55, LCD_WIDTH - 65, LCD_HEIGHT - 65);

    FWol = EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_RGB(greenR, greenG, greenB));

    // switched to bat temporarily
    FWol = EVE_PrintF(FWol, 250, 130, 25, EVE_OPT_CENTER, "RPM:  %3d", bat);

    FWol = EVE_PrintF(FWol, 195, 205, 25, EVE_OPT_CENTER, "BATT VOLT:  %3d", bat);

    FWol = EVE_PrintF(FWol, 211, 280, 25, EVE_OPT_CENTER, "TGT AFR:  %3d", afrtgt);

    FWol = EVE_PrintF(FWol, 247, 355, 25, EVE_OPT_CENTER, "AFR:  %3d", afr);

    FWol = EVE_PrintF(FWol, 567, 130, 25, EVE_OPT_CENTER, "CLT:  %3d", clt);

    FWol = EVE_PrintF(FWol, 553, 205, 25, EVE_OPT_CENTER, "GEAR:  %3d", gear);

    FWol = EVE_PrintF(FWol, 542, 280, 25, EVE_OPT_CENTER, "SPEED:  %3d", spd);

    FWol = EVE_PrintF(FWol, 550, 355, 25, EVE_OPT_CENTER, "SYNC:  %3d", sync);
    
    return FWol;
}

uint16_t Add_Dash_To_Display_List(uint16_t FWol, bool dashMode)
{
    if(dashMode){
        if (b3x2 > b2Bound)
        {
            FWol = EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_RGB(redR, redG, redB));
            FWol = EVE_Filled_Rectangle(FWol, b3x1, dashY1, b3x2, dashY2);
        }

        if (b2x2 > b1Bound)
        {
            FWol = EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_RGB(yellowR, yellowG, yellowB));
            FWol = EVE_Filled_Rectangle(FWol, b2x1, dashY1, b2x2, dashY2);
        }

        FWol = EVE_Cmd_Dat_0(FWol, EVE_ENC_COLOR_RGB(greenR, greenG, greenB));
        FWol = EVE_Filled_Rectangle(FWol, b1x1, dashY1, b1x2, dashY2);

        FWol = EVE_PrintF(FWol, 200, 200, 25, EVE_OPT_CENTER, "RPM: ");

        FWol = EVE_PrintF(FWol, 300, 200, 25, EVE_OPT_CENTER, "%3d", rpm);

        // This is where the Gear functions get printed out *****************

        if(gear_test_num == 0)
        {
            FWol = gearN(FWol);
        }
        else if(gear_test_num == 1)
        {
            FWol = gear1(FWol);
        }
        else if(gear_test_num == 2)
        {
        FWol = gear2(FWol);
        }
        else if(gear_test_num == 3)
        {
            FWol = gear3(FWol);
        }
        else if(gear_test_num == 4)
        {
            FWol = gear4(FWol);
        }
        else
        {
            FWol = gear5(FWol);
        }

    }
    else{
        
        FWol = Add_Diag_To_Display_List(FWol);
        

    }

    return (FWol);
    loop();
}   


uint16_t gearN(uint16_t FWolE)
{
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));

    // Left Line
    FWolE = EVE_Filled_Rectangle(FWolE, 627.5, 245, 650.5, 410);

    // Right Line
    FWolE=EVE_Filled_Rectangle(FWolE, 735, 245, 760, 410);

    // Middle Line
    FWolE=EVE_Line(FWolE, 645, 257.5, 741.5, 397.5, 12.5);

    return FWolE;
}

uint16_t gear1(uint16_t FWolE)
{

    // the middle line
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Filled_Rectangle(FWolE, 695, 205, 720, 450);

    // the Base line
    FWolE = EVE_Filled_Rectangle(FWolE, 640, 425, 775, 450);

    // the flag
    FWolE = EVE_Line(FWolE, 694, 217.5, 650, 270, 12.5);

    return FWolE;
}

uint16_t gear2(uint16_t FWolE)
{
    
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    
    // the Base line
    FWolE = EVE_Filled_Rectangle(FWolE, 640, 425, 775, 450);

    // Upper Loop
    FWolE = EVE_Line(FWolE, 697.5, 275, 697.5, 275, 70);
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(0, 0, 0));
    FWolE = EVE_Line(FWolE, 697.5, 275, 697.5, 275, 45);
    FWolE = EVE_Filled_Rectangle(FWolE, 625, 275, 725, 345);

    // Middle line 
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Line(FWolE, 630, 441, 730, 322.5, 12.5);

    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(0, 0, 0));
    FWolE = EVE_Filled_Rectangle(FWolE, 600, 400, 640, 460);
    return FWolE;
}

uint16_t gear3(uint16_t FWolE)

{
   // Upper Loop
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Line(FWolE, 697.5, 275, 697.5, 275, 70);
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(0, 0, 0));
    FWolE = EVE_Line(FWolE, 697.5, 275, 697.5, 275, 45);

    // bottom Loop
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Line(FWolE, 697.5, 395, 697.5, 395, 70);
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(0, 0, 0));
    FWolE = EVE_Line(FWolE, 697.5, 395, 697.5, 395, 45);
    FWolE = EVE_Filled_Rectangle(FWolE, 625, 275, 697.5, 380);

    return FWolE;
}

uint16_t gear4(uint16_t FWolE)
{
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));

    // Right line
    FWolE=EVE_Filled_Rectangle(FWolE, 735, 205, 760, 450);

    // the Middle line
    FWolE = EVE_Filled_Rectangle(FWolE, 627.5, 326, 775, 350);

    // Left line
    FWolE = EVE_Filled_Rectangle(FWolE, 627.5, 326, 650.5, 205);

    return FWolE;
}

uint16_t gear5(uint16_t FWolE)
{
    
    //Bottom loop
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Line(FWolE, 697.5, 395, 697.5, 395, 70);
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(0, 0, 0));
    FWolE = EVE_Line(FWolE, 697.5, 395, 697.5, 395, 45);

    FWolE = EVE_Filled_Rectangle(FWolE, 625, 275, 697.5, 395);

    // the Middle line
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Filled_Rectangle(FWolE, 627.5, 326, 698, 349);

    // Left line
    FWolE = EVE_Filled_Rectangle(FWolE, 627.5, 326, 650.5, 225);

    // Top Line
    FWolE = EVE_Filled_Rectangle(FWolE, 627.5, 245, 750, 225); 

    return FWolE;
}

void shiftUp() {
  oldGear = gear;
  
  //digitalWrite(C_SPAR, 1); // cut spark
  digitalWrite(DIR, 1); // set direction
  digitalWrite(PUL, 1); // shift motor
  delay(upShiftTime);   // keep shifting
  digitalWrite(PUL, 0); // stop shifting
  //digitalWrite(C_SPAR, 0); // resume spark
  
  if(oldGear == gear){ // if the shift didn't occur, aka gear before = gear after
    if(oldGear != 6){
      if(shiftTry < REJTRY){
        shiftTry = shiftTry + 1; //keep a count of how many tries it took
        shiftUp();
      } else {
        rejectFlag = true; // if it took REJTRY tries, send a reject signal
      }
    } else {
      rejectFlag = true; //if the gear is 6, GPOS can not be accurately determined
    }
  }
}

void shiftDown() {
  oldGear = gear;
  if(rpm < U_L_RPM) { //if RPM too high, don't even attempt to shift
    
    digitalWrite(DIR, 0);
    digitalWrite(PUL, 1);
    delay(dnShiftTime);
    digitalWrite(PUL, 0);
    
    if(oldGear == gear){ // if the shift didn't occur, aka gear before = gear after
      if(oldGear != 6){ 
        if(shiftTry < REJTRY){
          shiftTry = shiftTry + 1; //keep a count of how many tries it took
          shiftDown(); //try again
        } else {
          rejectFlag = true; // if it took REJTRY tries, send a reject signal
        }
      } else {
        rejectFlag = true; //if the gear is 6, GPOS can not be accurately determined
      }
    }
  } else {
    rejectFlag = true;
  }
}

void shiftNeutral() {
   // car can go to neutral if in 2nd or 1st gear
  while(gear > 2){
    shiftDown();
  }
  oldGear = gear;
  
  if (oldGear == 2){
    // neutral requires a half shift

    digitalWrite(DIR, 0); // shift down
    digitalWrite(PUL, 1);
    delay(nuShiftTime);
    digitalWrite(PUL, 0);
    
  } else if(oldGear == 1) {
    // neutral requires a half shift

    digitalWrite(DIR, 1); // shift up
    digitalWrite(PUL, 1);
    delay(nuShiftTime);
    digitalWrite(PUL, 0);
  }

  if (gear == oldGear){ //if the shift didn't work, try again
    if(oldGear != 6){
      if(shiftTry < REJTRY){
        shiftNeutral(); 
        shiftTry = shiftTry + 1; //keep a count of how many tries it took
      } else {
        rejectFlag = true; //if it took REJTRY tries, send a reject signal
      }
    } else {
      rejectFlag = true;
    }
  }
}

void loop() {

  // updates the states of the pins
  upPaddle.update();
  dnPaddle.update();

  //makes it so the rest of the code is executed only on a change in button state
  if (upPaddle.changed() || dnPaddle.changed()){
      
    // actual shifting up and down
    if(upPaddle.fell() && dnPaddle.fell()){
      shiftNeutral();
      shiftTry = 1;
    } else if(upPaddle.fell() && !dnPaddle.changed()){
      shiftUp();
      shiftTry = 1;
    } else if(!upPaddle.changed() && dnPaddle.fell()){
      shiftDown();
      shiftTry = 1;
    }
  
    // sends a signal to dash for rejected shift
    if(rejectFlag == true){
      //digitalWrite(FAIL, 1);
      delay(10);
      rejectFlag = false;
      //digitalWrite(FAIL, 0);
    }
  }

}

/*

    2. Race screen needs batvolt, gear, shift light
    3. shift light start at 2000rpm

*/