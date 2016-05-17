#include "PulseOutOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"

using namespace std;

PulseOutOperation::PulseOutOperation()
    : PinOperation() {
    opType = opPulseOut;
    pinAllAllowed = false;
    pulseLenUS = 100;
    pulseLevel = 1;
}

bool PulseOutOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (PinOperation::build(appInfo, paramList, paramIter)) {
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No pulse length specified for '" + mapFromOpType(opType) + "'");
            return false;
        }
        bool isok = true;
        if (!getLongInteger(**paramIter, pulseLenUS)) {
            isok = false;
        } else {
            if (pulseLenUS <= 0) {
                isok = false;
            }
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid pulse length for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }
        
        (*paramIter)++;
        
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No pulse level specified for '" + mapFromOpType(opType) + "'");
            return false;
        }

        if (!getInteger(**paramIter, pulseLevel)) {
            isok = false;
        } else {
            if ((pulseLevel != 0) && (pulseLevel != 1)) {
                isok = false;
            }
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid pulse level for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }
    } else {
        return false;
    }
    
    (*paramIter)++;
    return true;
}

string PulseOutOperation::toString() {
    return PinOperation::toString() + " Length:" + to_string(pulseLenUS) + " Level:" + to_string(pulseLevel);
}

bool PulseOutOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    PinOperation::execute(appInfo);
            
    GPIOPin * pin = new GPIOPin(pinNumber);
    if (appInfo->setdir) {
        appInfo->prtReport("Setting pin:" + to_string(pinNumber) + " to output");
        pin->setDirection(GPIO_OUTPUT);
    }
    appInfo->prtReport("Performing pulse output on pin:" + to_string(pinNumber) + " Length:" + to_string(pulseLenUS) + " Level:" + to_string(pulseLevel));
    pin->pulseOut(pulseLenUS, pulseLevel);
    GPIO_Result r = pin->getLastResult();
    bool isok = r == GPIO_OK;
    if (!isok) {
        appInfo->prtError(opType, "Failed to perform pulse out to pin");
    }
    delete pin;

    return isok;
}

string PulseOutOperation::help() {
    string hStr;
    hStr = "pulseout <pin-number> <duration> <level>";
    hStr = hStr + "\n\tOutputs a single pulse to given <pin-number> with supplied data";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    hStr = hStr + "\n\t<duration> is the length of pulse in micro-seconds. Must be >0";
    hStr = hStr + "\n\t<level> is the pulse level. Must be one of: 0 or 1";
    
    return hStr;
}
