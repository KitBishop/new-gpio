#include "FreqOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"

using namespace std;

FreqOperation::FreqOperation()
    : PinOperation() {
    opType = opFreq;
    pinAllAllowed = false;
    sampleTimeMS = 0;
}

bool FreqOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (PinOperation::build(appInfo, paramList, paramIter)) {
        bool isok = true;

        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No sample time specified for '" + mapFromOpType(opType) + "'");
            return false;
        }

        if (!getLongInteger(**paramIter, sampleTimeMS)) {
            isok = false;
        } else {
            if (sampleTimeMS <= 0) {
                isok = false;
            }
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid sample time for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }
    } else {
        return false;
    }
    
    (*paramIter)++;
    return true;
}

string FreqOperation::toString() {
    return PinOperation::toString() + " SampleTimeMS:" + to_string(sampleTimeMS);
}

bool FreqOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    PinOperation::execute(appInfo);
            
    GPIOPin * pin = new GPIOPin(pinNumber);
    if (appInfo->setdir) {
        appInfo->prtReport("Setting pin:" + to_string(pinNumber) + " to input");
        pin->setDirection(GPIO_INPUT);
    }
    appInfo->prtReport("Getting frequency on pin:" + to_string(pinNumber) + " SampleTimeMS:" + to_string(sampleTimeMS));
    long int freq = pin->getFrequency(sampleTimeMS);
    bool isok = (pin->getLastResult() == GPIO_OK);
    delete pin;
    if (!isok) {
        appInfo->prtError(opType, "Failed to perform frequency input on pin");
    } else {
        appInfo->prtOutput(to_string(freq));
        appInfo->result = freq;
    }

    return isok;
}

string FreqOperation::help() {
    string hStr;
    hStr = "frequency <pin-number> <sampletime>";
    hStr = hStr + "\n\tObtains, displays and returns frequency of input on";
    hStr = hStr + "\n\tgiven <pin-number>";
    hStr = hStr + "\n\tOperates by counting the number of pulses during the sample time";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    hStr = hStr + "\n\t<sampletime> is the time in milliseconds over which the";
    hStr = hStr + "\n\tfrequency is taken. Must be >0";
    
    return hStr;
}
