#include "PinOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "ForkAccess.h"

#include <cstdlib>

using namespace std;

PinOperation::PinOperation()
    : Operation(opInvalid) {
    pinNumber = 0;
    pinAllAllowed = false;
}

bool PinOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No pin number specified for '" + mapFromOpType(opType) + "'");
        return false;
    }
    if (((*paramIter)->compare("all") == 0) && pinAllAllowed) {
        pinNumber = -1;
    } else {
        if (!getInteger(**paramIter, pinNumber)) {
            appInfo->prtError(opType, "Invalid pin number for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }
        if (!GPIOAccess::isPinUsable(pinNumber)) {
            appInfo->prtError(opType, "Invalid pin number for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }        
    }
    if (pinNumber != -1) {
        if (!GPIOAccess::isPinUsable(pinNumber)) {
            appInfo->prtError(opType, "Unusable pin number for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }        
    }
    (*paramIter)++;
    return true;
}

string PinOperation::toString() {
    return Operation::toString() + " PinNumber:" + ((pinNumber == -1) ? "All" : to_string(pinNumber));
}

bool PinOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    int firstPin = pinNumber;
    int lastPin = pinNumber;
    if (pinNumber == -1) {
        firstPin = 0;
        lastPin = 30;
    } 
    for (int pinN = firstPin; pinN <= lastPin; pinN++) {
        if (GPIOAccess::isPinUsable(pinN)) {
            ForkAccess::stop(pinN);
        }
    }
    
    return true;
}