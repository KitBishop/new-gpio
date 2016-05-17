#include "ToneOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"
#include "ForkAccess.h"

using namespace std;

ToneOperation::ToneOperation()
    : PinOperation() {
    opType = opTone;
    pinAllAllowed = false;
    freq = 0;
    durationMS = 0;
}

bool ToneOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (PinOperation::build(appInfo, paramList, paramIter)) {
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No frequency specified for '" + mapFromOpType(opType) + "'");
            return false;
        }
        bool isok = true;
        if (!getLongInteger(**paramIter, freq)) {
            isok = false;
        } else {
            if (freq <= 0) {
                isok = false;
            }
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid frequency for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }

        (*paramIter)++;

        if (*paramIter != paramList->end()) {
            if (getLongInteger(**paramIter, durationMS)) {
                if (durationMS < 0) {
                    appInfo->prtError(opType, "Invalid duration for '" + mapFromOpType(opType) + "':" + **paramIter);
                    return false;
                }

                (*paramIter)++;
            }
        }
    } else {
        return false;
    }
    
    return true;
}

string ToneOperation::toString() {
    return PinOperation::toString() + " Frequency:" + to_string(freq) + " DurationMS:" + to_string(durationMS);
}

bool ToneOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    PinOperation::execute(appInfo);

    // Continuous PWM output requires a separate process
    pid_t pid = fork();

    if (pid == 0) {
        // child process, run the pwm
        GPIOPin * pin = new GPIOPin(pinNumber);
        if (appInfo->setdir) {
            appInfo->prtReport("Setting pin:" + to_string(pinNumber) + " to output");
            pin->setDirection(GPIO_OUTPUT);
        }
        appInfo->prtReport("Starting Tone to pin:" + to_string(pinNumber) + " Frequency:" + to_string(freq) + " DurationMS:" + to_string(durationMS));
        pin->setTone(freq, durationMS);
        GPIO_Result r = pin->getLastResult();
        bool isok = r == GPIO_OK;
        if (!isok) {
            appInfo->prtError(opType, "Failed to start Tone");
        } else {
            // Ensure child stays alive while Tone is running
            while (pin->isToneRunning()) {
                sleepMicro(1000000LL);
            }
        }
        delete pin;
        ForkAccess::stop(pinNumber);
    }
    else {
        // parent process
        string forkInf = "TONE: Frequency:" + to_string(freq) + " DurationMS:" + to_string(durationMS);

        ForkAccess::noteInfo(pinNumber, pid, forkInf);
    }
    
    return true;
}

string ToneOperation::help() {
    string hStr;
    hStr = "tone <pin-number> <frequency> <optional-duration>";
    hStr = hStr + "\n\tStarts a separate process to output a continous";
    hStr = hStr + "\n\ttone to given <pin-number> with given information";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    hStr = hStr + "\n\t<frequency> is the frequency of the tone";
    hStr = hStr + "\n\t\tMust be greater than 0";
    hStr = hStr + "\n\t<optional-duration> is an optional parameter that gives the";
    hStr = hStr + "\n\tduration of the output in milliseconds";
    hStr = hStr + "\n\t\tIf present, must be greater than or equal to 0";
    hStr = hStr + "\n\t\tWhen absent or 0, duration is indefinite";
    hStr = hStr + "\n\tThe separate process runs until the <optional-duration> expires";
    hStr = hStr + "\n\tor the 'tonestop' operation is performed on the same pin";
    hStr = hStr + "\n\tNOTE: equivalent to doing a 'pwm' operation with a <duty> of 50%";
    
    return hStr;
}
