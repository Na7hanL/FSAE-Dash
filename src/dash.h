#ifndef __DASH_H__
#define __DASH_H__

#include <Arduino.h>

// green- rgb(32,252,3)
// red- rgb(252, 40, 3)


void Initialize_Dash(void);
uint16_t Add_Dash_To_Display_List(uint16_t FWol);
void updateData(void);

uint16_t gearN(uint16_t FWolE);
uint16_t gear1(uint16_t FWolE);
uint16_t gear2(uint16_t FWolE);
uint16_t gear3(uint16_t FWolE);
uint16_t gear4(uint16_t FWolE);
uint16_t gear5(uint16_t FWolE);

#endif