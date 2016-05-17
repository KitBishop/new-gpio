#include "PwmStopOperation.h"
#include "Utilities.h"
#include "ForkAccess.h"

using namespace std;

PwmStopOperation::PwmStopOperation()
    : PinOperation() {
    opType = opPwmStop;
    pinAllAllowed = false;
}


bool PwmStopOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    appInfo->prtReport("Stopping PWM output on pin:" + to_string(pinNumber));
    
    ForkAccess::stop(pinNumber);

    return true;
}

string PwmStopOperation::help() {
    string hStr;
    hStr = "pwmstop <pin-number>";
    hStr = hStr + "\n\tStops any separate process that is currently performing PWM";
    hStr = hStr + "\n\toutput on given <pin-number>";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    
    return hStr;
}
