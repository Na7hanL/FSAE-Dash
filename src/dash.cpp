#include "dash.h"
#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>
#include "CFA10100_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include <iostream>
#include <String>


// here is a changes
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
uint16_t rpm;

uint16_t greenR;
uint16_t greenG;
uint16_t greenB;
uint16_t redR;
uint16_t redG;
uint16_t redB;
uint16_t yellowR;
uint16_t yellowG;
uint16_t yellowB;

bool back = false;
bool testing = true;
int printRPM;
// screen is 800px wide; 1rpm = 800/10500 px
// convert then use;

// need to take in rpm and represent it as 800/10500, then round down in pixel calculations

void Initialize_Dash(void)
{

    dashY1 = 330;
    dashY2 = 480;
    rpm = 0;
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
    if (rpm <= b1Bound)
    {
        b1x2 = rpm;
        b2x2 = b1Bound;
    }

    // updates bars in second rpm range
    if (rpm <= b2Bound)
    {
        b2x2 = rpm;
        b3x2 = b2Bound;
    }
    else if (rpm > b2Bound)
    {
        b2x2 = b2Bound;
    }
    else
    {
        b2x2 = b1Bound;
    }

    // updates bars in third rpm range
    if (rpm > b2Bound)
    {
        b3x2 = rpm;
    }
    else
    {
        b3x2 = b2Bound;
    }

    printRPM = rpm * 10500 / 800;

    if (testing)
    {
        if (rpm > LCD_WIDTH)
        {
            back = true;
        }
        if (rpm <= 0)
        {
            back = false;
        }
        if (!back)
        {
            rpm += 10;
        }
        else
        {
            rpm = 40;
            back = false;
        }
    }
}

uint16_t Add_Dash_To_Display_List(uint16_t FWol)
{
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

    FWol = EVE_PrintF(FWol, 300, 200, 25, EVE_OPT_CENTER, "%3d", printRPM);

    // This is where the Gear functions get printed out *****************
    //FWol = gearN(FWol);
    //FWol = gear1(FWol);
    FWol = gear2(FWol);
    //FWol = gear3(FWol);
    //FWol = gear4(FWol);
    //FWol = gear5(FWol);
  

    return (FWol);
}

uint16_t gearN(uint16_t FWolE)
{

    // segment 0
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Filled_Rectangle(FWolE, 600, 175, 625, 450);

    // segment 1
    //FWolE = EVE_Filled_Rectangle(FWolE, 600, 300, 625, 450);

    // segment 3
    FWolE=EVE_Filled_Rectangle(FWolE, 775 ,175 , 800, 450);

    // segment 4
   // FWolE=EVE_Filled_Rectangle(FWolE, 775 ,300 , 800, 450);

    // segment 7
    FWolE=EVE_Line(FWolE, 617.5 ,192.5 , 782.5, 432.5, 17.5);

    return FWolE;
}

uint16_t gear1(uint16_t FWolE)
{

    // the middle line
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Filled_Rectangle(FWolE, 685, 175, 710, 450);

    // the Base line
    FWolE = EVE_Filled_Rectangle(FWolE, 635, 425, 760, 450);

    // the flag
    // FWolE=EVE_Line(FWolE, 692.5, 192.5, 625, 285, 17.5);
    FWolE=EVE_Line(FWolE, 692.5, 192.5, 650, 270, 17.5);

    return FWolE;
}

uint16_t gear2(uint16_t FWolE)
{

    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    
    // the Base line
    FWolE = EVE_Filled_Rectangle(FWolE, 635, 425, 760, 450);

    // The connecter
    //FWolE = EVE_Filled_Rectangle(FWolE, 645, 400, 675, 425);
    
    // blah blah git test

    // Circle Stuff
    //FWolE=EVE_Line(FWolE, 782.5, 432.5, 617.5 ,192.5, 17.5);
    FWolE=EVE_Line(FWolE, 687.5, 325, 687.5, 325, 60);

    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(0, 0, 0));
    FWolE=EVE_Line(FWolE, 657.5, 345, 657.5, 345, 60);

    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE=EVE_Line(FWolE, 655, 440, 730, 347, 11);

    FWolE=EVE_Filled_Rectangle(FWolE, 627.5, 265, 687.5, 285);


    


    

    /*
    // the Circle
     FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));
    FWolE = EVE_Line(FWolE, 635, 425, 760, 450, 100);
    */

    //FWolE = EVE_Point(FWolE, 645, 400, 50);

    return FWolE;
}

uint16_t gear3(uint16_t FWolE)

{
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));

    // the Base line
    FWolE = EVE_Filled_Rectangle(FWolE, 635, 425, 760, 450);

    // Right line
    FWolE=EVE_Filled_Rectangle(FWolE, 735 ,175 , 760, 450);

    // the Top line
    FWolE = EVE_Filled_Rectangle(FWolE, 635, 175, 760, 200);

    // the Middle line
    FWolE = EVE_Filled_Rectangle(FWolE, 660, 300, 760, 325);

    return FWolE;
}

uint16_t gear4(uint16_t FWolE)
{
    FWolE = EVE_Cmd_Dat_0(FWolE, EVE_ENC_COLOR_RGB(255, 255, 255));

    // Right line
    FWolE=EVE_Filled_Rectangle(FWolE, 735 ,175 , 760, 450);

    // the Middle line
    FWolE = EVE_Filled_Rectangle(FWolE, 635, 300, 760, 325);

    // left line
    FWolE=EVE_Filled_Rectangle(FWolE, 635, 300, 660, 175);

    return FWolE;
}

/*

    1. touch screen
    2. Race screen needs batvolt, gear, shift light
    3. shift light start at 2000rpm
    X 4. Use rectangles to make gear numbers
    5. Diagnostic uses values from can v1.1 program in onedrive

*/