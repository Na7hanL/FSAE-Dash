#include "dash.h"
#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>
#include "CFA10100_defines.h"
#include "EVE_base.h"
#include "EVE_draw.h"
#include <iostream>
#include <String>

// Renamed rpm variable to rpm_a because it was causing issues with an already defined rpm function--- Maddux
uint16_t rpm_a, bat, afrtgt, clt, afr, gear, spd, sync;


void Initialize_Diag(void){
    uint16_t rpm_a, bat, afrtgt, clt, afr, gear, spd, sync = 0;
}

uint16_t Add_Diag_To_Display_List(uint16_t FWol){
    FWol = EVE_PrintF(FWol, 200, 200, 25, EVE_OPT_CENTER, "RPM:  %3d", rpm_a);

    FWol = EVE_PrintF(FWol, 200, 250, 25, EVE_OPT_CENTER, "BATTERY VOLTAGE:  %3d", bat);

    FWol = EVE_PrintF(FWol, 200, 300, 25, EVE_OPT_CENTER, "TARGET AFR:  %3d", afrtgt);

    FWol = EVE_PrintF(FWol, 200, 350, 25, EVE_OPT_CENTER, "COOLANT TEMP:  %3d", clt);

    FWol = EVE_PrintF(FWol, 400, 200, 25, EVE_OPT_CENTER, "AFR:  %3d", afr);

    FWol = EVE_PrintF(FWol, 400, 250, 25, EVE_OPT_CENTER, "GEAR:  %3d", gear);

    FWol = EVE_PrintF(FWol, 400, 300, 25, EVE_OPT_CENTER, "SPEED:  %3d", spd);

    FWol = EVE_PrintF(FWol, 400, 350, 25, EVE_OPT_CENTER, "SYNC:  %3d", sync);
}