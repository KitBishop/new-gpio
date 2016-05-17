#include "GetDirectionOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"

using namespace std;

GetDirectionOperation::GetDirectionOperation()
: PinOperation() {
    opType = opGetDirection;
    pinAllAllowed = false;
}

bool GetDirectionOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    GPIOPin * pin = new GPIOPin(pinNumber);
    appInfo->prtReport("Getting pin direction:" + to_string(pinNumber));
    GPIO_Direction dir = pin->getDirection();
    GPIO_Result r = pin->getLastResult();
    bool isok = r == GPIO_OK;
    if (!isok) {
        appInfo->prtError(opType, "Failed to get pin direction");
    } else {
        appInfo->prtOutput(to_string(dir));
        appInfo->result = dir;
    }
    delete pin;

    return isok;
}

string GetDirectionOperation::help() {
    string hStr;
    hStr = "get-direction <pin-number>";
    hStr = hStr + "\n\tReads, displays and returns the current direction";
    hStr = hStr + "\n\tof given <pin-number>";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    
    return hStr;
}
