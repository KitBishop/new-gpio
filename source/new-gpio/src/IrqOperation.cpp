#include <cstdlib>

#include "IrqOperation.h"
#include "Utilities.h"
#include "GPIOAccess.h"
#include "GPIOPin.h"
#include "ForkAccess.h"

using namespace std;

class GPIO_Irq_Command_Handler_Object : public GPIO_Irq_Handler_Object {
public:
    GPIO_Irq_Command_Handler_Object(const char * com) {
        strcpy(this->cmd, com);
    }

    void handleIrq(int pinNum, GPIO_Irq_Type type) {
        if (strstr(cmd, "[debug]") == cmd) {
            char dbgcmd[300];
            sprintf(dbgcmd, "echo 'GPIO Irq Debug: Pin=%d Type=", pinNum);
            if (type == GPIO_IRQ_RISING) {
                strcat(dbgcmd, "Rising' && ");
            } else {
                strcat(dbgcmd, "Falling' && ");
            }
            strcat(dbgcmd, cmd + 7);
            system(dbgcmd);
        } else {
            system(cmd);
        }
    }
    
private:
    char cmd[200];
};

IrqOperation::IrqOperation()
    : PinOperation() {
    opType = opIrq;
    pinAllAllowed = false;
    irqType = GPIO_IRQ_NONE;
    cmd = "";
    debounceMS = 0;
}

bool IrqOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (PinOperation::build(appInfo, paramList, paramIter)) {
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No type specified for '" + mapFromOpType(opType) + "'");
            return false;
        }
        if ((*paramIter)->compare("rising") == 0) {
            irqType = GPIO_IRQ_RISING;
        } else if ((*paramIter)->compare("falling") == 0) {
            irqType = GPIO_IRQ_FALLING;
        } else if ((*paramIter)->compare("both") == 0) {
            irqType = GPIO_IRQ_FALLING;
        } else {
            appInfo->prtError(opType, "Invalid type for '" + mapFromOpType(opType) + "':" + **paramIter);
            return false;
        }

        (*paramIter)++;

        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No command specified for '" + mapFromOpType(opType) + "'");
            return false;
        }
        
        cmd = **paramIter;
        
        (*paramIter)++;

        if (*paramIter != paramList->end()) {
            if (getLongInteger(**paramIter, debounceMS)) {
                if (debounceMS < 0) {
                    appInfo->prtError(opType, "Invalid debounce for '" + mapFromOpType(opType) + "':" + **paramIter);
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

string IrqOperation::toString() {
    string oinf = PinOperation::toString() + " Type:";
    if (irqType == GPIO_IRQ_RISING) {
        oinf += "rising";
    } else if (irqType == GPIO_IRQ_FALLING) {
        oinf += "falling";
    } else {
        oinf += "both";
    }
    oinf += " Command:'" + cmd + "' DebounceMS:" + to_string(debounceMS);
    
    return oinf;
}

bool IrqOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    PinOperation::execute(appInfo);

    string cmdex = appInfo->replaceVars(cmd);
    
    // Continuous PWM output requires a separate process
    pid_t pid = fork();

    if (pid == 0) {
        // child process, run the pwm
        GPIOPin * pin = new GPIOPin(pinNumber);
        if (appInfo->setdir) {
            appInfo->prtReport("Setting pin:" + to_string(pinNumber) + " to input");
            pin->setDirection(GPIO_INPUT);
        }

        string oinf = "Enabling IRQ handling on pin:" + to_string(pinNumber) + " Type:";
        if (irqType == GPIO_IRQ_RISING) {
            oinf += "rising";
        } else if (irqType == GPIO_IRQ_FALLING) {
            oinf += "falling";
        } else {
            oinf += "both";
        }
        oinf += " Command:'" + cmdex + "' DebounceMS:" + to_string(debounceMS);
        appInfo->prtReport(oinf);

        GPIO_Irq_Command_Handler_Object * irqHandlerObj = new GPIO_Irq_Command_Handler_Object(cmdex.c_str());
        pin->setIrq(irqType, irqHandlerObj, debounceMS);
        GPIO_Result r = pin->getLastResult();
        bool isok = r == GPIO_OK;
        if (!isok) {
            appInfo->prtError(opType, "Failed to enable IRQ handling");
        } else {
            // Ensure child stays alive since IRQ is running
            while (true) {
                sleepMicro(1000000LL);
            }
        }
        delete pin;
        ForkAccess::stop(pinNumber);
    }
    else {
        // parent process
        string forkInf = "IRQ: Type:";
        if (irqType == GPIO_IRQ_RISING) {
            forkInf += "rising";
        } else if (irqType == GPIO_IRQ_FALLING) {
            forkInf += "falling";
        } else {
            forkInf += "both";
        }
        forkInf += " Command:'" + cmdex + "' DebounceMS:" + to_string(debounceMS);
        
        ForkAccess::noteInfo(pinNumber, pid, forkInf);
    }
    
    return true;
}

string IrqOperation::help() {
    string hStr;
    hStr = "irq <pin-number> <irq-type> <command> <optional-debounce>";
    hStr = hStr + "\n\tStarts a separate process to respond to interrupts";
    hStr = hStr + "\n\ton given <pin-number> and perform given <command> on interrupt";
    hStr = hStr + "\n\t<pin-number> must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    hStr = hStr + "\n\t<irq-type> is the type of interrupt to catch";
    hStr = hStr + "\n\tMust be one of:";
    hStr = hStr + "\n\t\trising - interrupt occurs on rising edge";
    hStr = hStr + "\n\t\tfalling - interrupt occurs on falling edge";
    hStr = hStr + "\n\t\tboth - interrupt occurs on both rising and falling edges";
    hStr = hStr + "\n\t<command> is the command to be performed on interrupt";
    hStr = hStr + "\n\tMust be enclosed in \" characters if it contains spaces";
    hStr = hStr + "\n\tor other special character";
    
    hStr = hStr + "\n\tThe <command> may contain any number of <varsub>s which will be";
    hStr = hStr + "\n\treplaced by actual values at the time the irq operation is";
    hStr = hStr + "\n\tactually invoked.";
    hStr = hStr + "\n\tA <varsub> is any sequence like {<varref>}";
    hStr = hStr + "\n\twhere '<varref>' is one of:";
    hStr = hStr + "\n\t  - <variable-name> = current value of variable";
    hStr = hStr + "\n\t    as assigned in an 'assign' operation ";
    hStr = hStr + "\n\t    or not substituted if variable has never been assigned";
    hStr = hStr + "\n\t  - $n or $nn = current value of pin n or nn";
    hStr = hStr + "\n\t  - $! = value of latest result set by earlier commands";
    hStr = hStr + "\n\t  - $? = status of last executed command: 0=error, 1=ok";
    hStr = hStr + "\n\t  - $[<file-name>] = file <file-name> exists; 0=no, 1=yes";
    
    hStr = hStr + "\n\t<optional-debounce> is an optional parameter that gives a";
    hStr = hStr + "\n\tdebounce time in milliseconds.  Any interrupts that occur within";
    hStr = hStr + "\n\tthis time of a previous interrupt will be ignored.";
    hStr = hStr + "\n\tUsed to cater for noisy mechanical signals";
    hStr = hStr + "\n\tIf present, must be greater than or equal to 0";
    hStr = hStr + "\n\tWhen absent or 0, no debounce testing is applied";
    hStr = hStr + "\n\tThe separate process runs until the 'irqstop' operation";
    hStr = hStr + "\n\tis performed on the same <pin-number>";
    
    return hStr;
}
