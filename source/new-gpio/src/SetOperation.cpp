#include "SetOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"

using namespace std;

SetOperation::SetOperation()
    : PinOperation() {
    opType = opSet;
    pinAllAllowed = true;
    value = 0;
}

bool SetOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (PinOperation::build(appInfo, paramList, paramIter)) {
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No value specified for '" + mapFromOpType(opType) + "'");
            return false;
        }
        bool isok = true;
        if (!getInteger(**paramIter, value)) {
            isok = false;
        } else {
            if ((value != 0) && (value != 1)) {
                isok = false;
            }
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid value for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }
    } else {
        return false;
    }
    
    (*paramIter)++;
    return true;
}

string SetOperation::toString() {
    return PinOperation::toString() + " Value:" + to_string(value);
}

bool SetOperation::execute(AppInfo * appInfo) {
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
            if (appInfo->setdir) {
                appInfo->prtReport("Setting pin:" + to_string(pinN) + " to output");
                pin->setDirection(GPIO_OUTPUT);
            }
            appInfo->prtReport("Setting pin:" + to_string(pinN) + " to value:" + to_string(value));
            pin->set(value);
            GPIO_Result r = pin->getLastResult();
            bool isok = r == GPIO_OK;
            if (!isok) {
                appInfo->prtError(opType, "Failed to set pin");
            }
            delete pin;
        }
    }
    return isok;
}

string SetOperation::help() {
    string hStr;
    hStr = "set <pin-number> <value>";
    hStr = hStr + "\n\tSets given <pin-number> or all pins to the given value";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26 or all";
    hStr = hStr + "\n\t<value> must be one of: 0 or 1";
    
    return hStr;
}
