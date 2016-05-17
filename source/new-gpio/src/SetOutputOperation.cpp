#include "SetOutputOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"

using namespace std;

SetOutputOperation::SetOutputOperation()
    : PinOperation() {
    opType = opSetOutput;
    pinAllAllowed = true;
}

bool SetOutputOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    PinOperation::execute(appInfo);
            
    int firstPin = pinNumber;
    int lastPin = pinNumber;
    bool isok = true;
    if (pinNumber == -1) {
        firstPin = 0;
        lastPin = 30;
    } 
    for (int pinN = firstPin; (pinN <= lastPin) && isok; pinN++) {
        if (GPIOAccess::isPinUsable(pinN)) {
            GPIOPin * pin = new GPIOPin(pinN);
            appInfo->prtReport("Setting pin:" + to_string(pinN) + " to output");
            pin->setDirection(GPIO_OUTPUT);
            GPIO_Result r = pin->getLastResult();
            bool isok = r == GPIO_OK;
            if (!isok) {
                appInfo->prtError(opType, "Failed to set pin to output");
            }
            delete pin;
        }
    }
    return isok;
}

string SetOutputOperation::help() {
    string hStr;
    hStr = "set-output <pin-number>";
    hStr = hStr + "\n\tSets given <pin-number> or all pins to be output pins";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26 or all";
    
    return hStr;
}
