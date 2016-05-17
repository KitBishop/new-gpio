#include <unistd.h>
#include <string>

#include "ExpledOperation.h"
#include "Utilities.h"
#include "RGBLED.h"
#include "ForkAccess.h"
#include "TimeHelper.h"

using namespace std;

ExpledOperation::ExpledOperation()
    : Operation(opExpled) {
    redValue = 0;
    greenValue = 0;
    blueValue = 0;
}

bool ExpledOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No value specified for '" + mapFromOpType(opType) + "'");
        return false;
    }

    if ((*paramIter)->compare("rgb") != 0) {
        bool isok = true;
        if (((*paramIter)->substr(0,2).compare("0x") == 0) || ((*paramIter)->substr(0,2).compare("0X") == 0)) {
            long int ledVal;
            if (getLongInteger(**paramIter, ledVal)) {
                if ((ledVal < 0) || (ledVal > 0xffffff)) {
                    isok = false;
                } else {
                    redValue = (ledVal >> 16) & 0xff;
                    greenValue = (ledVal >> 8) & 0xff;
                    blueValue = ledVal & 0xff;
                    redValue = (redValue * 100) / 255;
                    greenValue = (greenValue * 100) / 255;
                    blueValue = (blueValue * 100) / 255;
                }
            } else {
                isok = false;
            }
        } else {
            isok = false;
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid value specified for '" + mapFromOpType(opType) + "' " + **paramIter);
            return false;
        }    

        (*paramIter)++;
    } else {
        (*paramIter)++;
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No red value specified for '" + mapFromOpType(opType) + "' rgb");
            return false;
        }
        bool isok = true;
        if (getInteger(**paramIter, redValue)) {
            if ((redValue < 0) || (redValue > 100)) {
                isok = false;
            }
        } else {
            isok = false;
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid red value specified for '" + mapFromOpType(opType) + "' " + **paramIter);
            return false;
        }    

        (*paramIter)++;
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No green value specified for '" + mapFromOpType(opType) + "' rgb");
            return false;
        }
        isok = true;
        if (getInteger(**paramIter, greenValue)) {
            if ((greenValue < 0) || (greenValue > 100)) {
                isok = false;
            }
        } else {
            isok = false;
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid green value specified for '" + mapFromOpType(opType) + "' " + **paramIter);
            return false;
        }    

        (*paramIter)++;
        if (*paramIter == paramList->end()) {
            appInfo->prtError(opType, "No blue value specified for '" + mapFromOpType(opType) + "' rgb");
            return false;
        }
        isok = true;
        if (getInteger(**paramIter, blueValue)) {
            if ((blueValue < 0) || (blueValue > 100)) {
                isok = false;
            }
        } else {
            isok = false;
        }
        if (!isok) {
            appInfo->prtError(opType, "Invalid blue value specified for '" + mapFromOpType(opType) + "' " + **paramIter);
            return false;
        }    

        (*paramIter)++;
    }

    return true;
}

string ExpledOperation::toString() {
    return Operation::toString()
            + " Red:" + to_string(redValue)
            + " Green:" + to_string(greenValue)
            + " Blue:" + to_string(blueValue);
}

bool ExpledOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    ForkAccess::stop(15);
    ForkAccess::stop(16);
    ForkAccess::stop(17);

    // Continuous Expled output requires a separate process
    pid_t pid = fork();

    if (pid == 0) {
        // child process, run the pwm
        RGBLED * led = new RGBLED();

        string s = "";
        appInfo->prtReport(s + "Starting Expled output:" +
            + " Red:" + to_string(redValue)
            + " Green:" + to_string(greenValue)
            + " Blue:" + to_string(blueValue));

        led->setColor(redValue, greenValue, blueValue);

        // Ensure child stays alive since Expled is running
        while (true) {
            sleepMicro(1000000LL);
        }

        delete led;
    }
    else {
        // parent process
        string forkInf = "";
        forkInf = forkInf + "EXPLED(red):" +
            + " Red:" + to_string(redValue)
            + " Green:" + to_string(greenValue)
            + " Blue:" + to_string(blueValue);        
        ForkAccess::noteInfo(17, pid, forkInf);

        forkInf = "";
        forkInf = forkInf + "EXPLED(green):" +
            + " Red:" + to_string(redValue)
            + " Green:" + to_string(greenValue)
            + " Blue:" + to_string(blueValue);        
        ForkAccess::noteInfo(16, pid, forkInf);

        forkInf = "";
        forkInf = forkInf + "EXPLED(blue):" +
            + " Red:" + to_string(redValue)
            + " Green:" + to_string(greenValue)
            + " Blue:" + to_string(blueValue);        
        ForkAccess::noteInfo(15, pid, forkInf);
    }
    
    return true;
}

string ExpledOperation::help() {
    string hStr;
    hStr = "expled <led-hex-value>";
    hStr = hStr + "\nOr:";
    hStr = hStr + "\nexpled rgb <red> <green> <blue>";
    hStr = hStr + "\n\tStarts a separate process to output given colour to";
    hStr = hStr + "\n\texpansion dock LED.";
    hStr = hStr + "\n\t<led-hex_value> specifies the LED colours in hex in the form:";
    hStr = hStr + "\n\t\t'0xrrggbb' where each colour component is in the hex";
    hStr = hStr + "\n\t\trange '00' (off) to 'ff' (fully on)";
    hStr = hStr + "\n\t<red> <green> <blue> specify the LED colours in decimal";
    hStr = hStr + "\n\t\tEach must be in the range 0 (off) to 100 (fully on)";
    hStr = hStr + "\n\tThe separate process runs until the 'expledstop' operation";
    hStr = hStr + "\n\tis performed";
    hStr = hStr + "\n\tNOTE: expled uses pins:15 (blue), 16 (green), 17 (red)";
    
    return hStr;
}
