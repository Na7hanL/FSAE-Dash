#include "dash.h"
#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>
#include "CFA10100_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include <iostream>
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

bool back = false;
bool testing = true;
int printRPM;
int gear_test_num = 0;
// screen is 800px wide; 1rpm = 800/10500 px
// convert then use;

// need to take in rpm and represent it as 800/10500, then round down in pixel calculations

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

/*

    2. Race screen needs batvolt, gear, shift light
    3. shift light start at 2000rpm

*/