#include "PulseInOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"

using namespace std;

PulseInOperation::PulseInOperation()
    : PinOperation() {
    opType = opPulseIn;
    pinAllAllowed = false;
    timeOutUS = 0;
    pulseLevel = 1;
}

bool PulseInOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (PinOperation::build(appInfo, paramList, paramIter)) {
        bool isok = true;

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

        (*paramIter)++;
        
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No time out specified for '" + mapFromOpType(opType) + "'");
            return false;
        }

        if (!getLongInteger(**paramIter, timeOutUS)) {
            isok = false;
        } else {
            if (timeOutUS < 0) {
                isok = false;
            }
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid time out for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }
    } else {
        return false;
    }
    
    (*paramIter)++;
    return true;
}

string PulseInOperation::toString() {
    return PinOperation::toString() + " Level:" + to_string(pulseLevel) + " TimeOut:" + to_string(timeOutUS);
}

bool PulseInOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    PinOperation::execute(appInfo);
            
    GPIOPin * pin = new GPIOPin(pinNumber);
    if (appInfo->setdir) {
        appInfo->prtReport("Setting pin:" + to_string(pinNumber) + " to input");
        pin->setDirection(GPIO_INPUT);
    }
    appInfo->prtReport("Performing pulse input on pin:" + to_string(pinNumber) + " Level:" + to_string(pulseLevel) + " TimeOut:" + to_string(timeOutUS));
    long int pulseInLen = pin->pulseIn(pulseLevel, timeOutUS);
    GPIO_Result r = pin->getLastResult();
    bool isok = r == GPIO_OK;
    if (!isok) {
        if (r == GPIO_TIME_OUT) {
            appInfo->prtError(opType, "Timeout on pulse input on pin");
        } else {
            appInfo->prtError(opType, "Failed to perform pulse in on pin");
        }
    } else {
        appInfo->prtOutput(to_string(pulseInLen));
        appInfo->result = pulseInLen;
    }
    delete pin;

    return isok;
}

string PulseInOperation::help() {
    string hStr;
    hStr = "pulsein <pin-number> <level> <timeout>";
    hStr = hStr + "\n\tWaits for and displays and returns duration of pulse input on";
    hStr = hStr + "\n\tgiven <pin-number>";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    hStr = hStr + "\n\t<level> is the pulse level. Must be one of: 0 or 1";
    hStr = hStr + "\n\t<timeout> is the time to wait for the pulse and its completion";
    hStr = hStr + "\n\tin micro-seconds.";
    hStr = hStr + "\n\t\tMust be >=0  If 0, timeout is indefinite";
    
    return hStr;
}
