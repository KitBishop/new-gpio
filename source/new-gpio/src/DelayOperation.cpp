#include "DelayOperation.h"
#include "Utilities.h"
#include "TimeHelper.h"

#include <cstdlib>

using namespace std;

DelayOperation::DelayOperation()
    : Operation(opDelay) {
    durationMS = 0;
}

bool DelayOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No duration specified '" + mapFromOpType(opType) + "'");
        return false;
    }

    bool ok = getInteger(**paramIter, durationMS);
    if (!ok || (durationMS < 0)) {
        appInfo->prtError(opType, "Invalid duration for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }

    (*paramIter)++;

    return true;
}

string DelayOperation::toString() {
    return Operation::toString() + " DurationMS:" + to_string(durationMS);
}

bool DelayOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }
    appInfo->prtReport("Delaying for :" + to_string(durationMS) + " MS");
    
    sleepMilli(durationMS);

    return true;
}

string DelayOperation::help() {
    string hStr;
    hStr = "delay <delayMS>";
    hStr = hStr + "\n\tPauses execution for the given period";
    hStr = hStr + "\n\t<delayMS> is the delay period in milliseconds. Must be >=0";
    
    return hStr;
}
