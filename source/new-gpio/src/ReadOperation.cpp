#include "ReadOperation.h"
#include "Utilities.h"
#include "GPIOPin.h"

using namespace std;

ReadOperation::ReadOperation()
    : PinOperation() {
    opType = opRead;
    pinAllAllowed = false;
}


bool ReadOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    PinOperation::execute(appInfo);

    GPIOPin * pin = new GPIOPin(pinNumber);
    if (appInfo->setdir) {
        appInfo->prtReport("Setting pin:" + to_string(pinNumber) + " to input");
        pin->setDirection(GPIO_INPUT);
    }
    appInfo->prtReport("Reading from pin:" + to_string(pinNumber));
    int v = pin->get();
    GPIO_Result r = pin->getLastResult();
    bool res = r == GPIO_OK;
    if (!res) {
        appInfo->prtError(opType, "Failed to read pin");
    } else {
        appInfo->prtOutput(to_string(v));
        appInfo->result = v;
    }
    delete pin;
    return res;
}

string ReadOperation::help() {
    string hStr;
    hStr = "read <pin-number>";
    hStr = hStr + "\n\tReads, displays and returns value of given <pin-number>";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    
    return hStr;
}
