#include "PwmOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"
#include "ForkAccess.h"

using namespace std;

PwmOperation::PwmOperation()
    : PinOperation() {
    opType = opPwm;
    pinAllAllowed = false;
    freq = 0;
    duty = 0;
    durationMS = 0;
}

bool PwmOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
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

        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No duty specified for '" + mapFromOpType(opType) + "'");
            return false;
        }

        if (!getInteger(**paramIter, duty)) {
            isok = false;
        } else {
            if ((duty < 0) || (duty > 100)) {
                isok = false;
            }
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid duty for '" + mapFromOpType(opType) + "':" + **paramIter);
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

string PwmOperation::toString() {
    return PinOperation::toString() + " Frequency:" + to_string(freq) + " Duty:" + to_string(duty) + " DurationMS:" + to_string(durationMS);
}

bool PwmOperation::execute(AppInfo * appInfo) {
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
        appInfo->prtReport("Starting PWM to pin:" + to_string(pinNumber) + " Frequency:" + to_string(freq) + " Duty:" + to_string(duty) + " DurationMS:" + to_string(durationMS));
        pin->setPWM(freq, duty, durationMS);
        GPIO_Result r = pin->getLastResult();
        bool isok = r == GPIO_OK;
        if (!isok) {
            appInfo->prtError(opType, "Failed to start PWM");
        } else {
            // Ensure child stays alive while PWM is running
            while (pin->isPWMRunning()) {
                sleepMicro(1000000LL);
            }
        }
        delete pin;
        ForkAccess::stop(pinNumber);
    }
    else {
        // parent process
        string forkInf = "PWM: Frequency:" + to_string(freq) + " Duty:" + to_string(duty) + " DurationMS:" + to_string(durationMS);

        ForkAccess::noteInfo(pinNumber, pid, forkInf);
    }
    
    return true;
}

string PwmOperation::help() {
    string hStr;
    hStr = "pwm <pin-number> <frequency> <duty> <optional-duration>";
    hStr = hStr + "\n\tStarts a separate process to perform PWM output";
    hStr = hStr + "\n\ton given <pin-number> with given information";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    hStr = hStr + "\n\t<frequency> is the frequency of the PWM pulses";
    hStr = hStr + "\n\t\tMust be greater than 0";
    hStr = hStr + "\n\t<duty> is the duty cycle % of the PWM pulses";
    hStr = hStr + "\n\t\tMust be in the range 0 to 100";
    hStr = hStr + "\n\t<optional-duration> is an optional parameter that gives the";
    hStr = hStr + "\n\tduration of the output in milliseconds";
    hStr = hStr + "\n\t\tIf present, must be greater than or equal to 0";
    hStr = hStr + "\n\t\tWhen absent or 0, duration is indefinite";
    hStr = hStr + "\n\tThe separate process runs until the <optional-duration> expires";
    hStr = hStr + "\n\tor the 'pwmstop' operation is performed on the same <pin-number>";
    
    return hStr;
}
