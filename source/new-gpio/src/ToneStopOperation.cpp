#include "ToneStopOperation.h"
#include "Utilities.h"
#include "ForkAccess.h"

using namespace std;

ToneStopOperation::ToneStopOperation()
    : PinOperation() {
    opType = opToneStop;
    pinAllAllowed = false;
}


bool ToneStopOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    appInfo->prtReport("Stopping Tone output on pin:" + to_string(pinNumber));
    
    ForkAccess::stop(pinNumber);

    return true;
}

string ToneStopOperation::help() {
    string hStr;
    hStr = "tonestop <pin-number>";
    hStr = hStr + "\n\tStops any separate process that is currently performing tone";
    hStr = hStr + "\n\toutput on given <pin-number>";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    
    return hStr;
}
