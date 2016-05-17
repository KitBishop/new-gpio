#include "IrqStopOperation.h"
#include "Utilities.h"
#include "ForkAccess.h"

using namespace std;

IrqStopOperation::IrqStopOperation()
    : PinOperation() {
    opType = opIrqStop;
    pinAllAllowed = false;
}


bool IrqStopOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    appInfo->prtReport("Stopping IRQ handling on pin:" + to_string(pinNumber));
    
    ForkAccess::stop(pinNumber);

    return true;
}

string IrqStopOperation::help() {
    string hStr;
    hStr = "irqstop <pin-number>";
    hStr = hStr + "\n\tStops any separate process that is currently performing IRQ";
    hStr = hStr + "\n\thandling on given <pin-number>";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    
    return hStr;
}
