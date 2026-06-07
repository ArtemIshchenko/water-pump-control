#pragma once

#include <Arduino.h>
#include "DisplayLink.h"
#include "HousePump.h"
#include "McuManage.h"
#include "WaterSupplyPump.h"

class PumpSystem {
public:
    void init();
    void update();

private:
    DisplayLink _display;
    HousePump _housePump;
    WaterSupplyPump _waterSupplyPump;
    McuManage _mcuManage;
};
