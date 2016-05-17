#include <unistd.h>
#include <string>

#include "ShiftOutOperation.h"
#include "Utilities.h"
#include "GPIOShiftOut.h"
#include "GPIOAccess.h"
#include "ForkAccess.h"

using namespace std;

ShiftOutOperation::ShiftOutOperation()
    : Operation(opShiftOut) {
    dataPinN = 0;
    clockPinN = 0;
    clockPeriodNS = 2000;
    bitOrder = GPIO_MSB_FIRST;
    
    value = 0;
}

bool ShiftOutOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No data pin specified for '" + mapFromOpType(opType) + "'");
        return false;
    }
    if (!getInteger(**paramIter, dataPinN)) {
        appInfo->prtError(opType, "Invalid data pin number for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }
    if (!GPIOAccess::isPinUsable(dataPinN)) {
        appInfo->prtError(opType, "Invalid data pin number for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }        
    (*paramIter)++;
    
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No clock pin specified for '" + mapFromOpType(opType) + "'");
        return false;
    }
    if (!getInteger(**paramIter, clockPinN)) {
        appInfo->prtError(opType, "Invalid clock pin number for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }
    if (!GPIOAccess::isPinUsable(clockPinN)) {
        appInfo->prtError(opType, "Invalid clock pin number for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }        
    if (dataPinN == clockPinN) {
        appInfo->prtError(opType, "Invalid data and clock pin numbers must be different for '" + mapFromOpType(opType) + "'");
        return false;
    }
    (*paramIter)++;

    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No clock period specified for '" + mapFromOpType(opType) + "'");
        return false;
    }
    if (!getLongInteger(**paramIter, clockPeriodNS)) {
        appInfo->prtError(opType, "Invalid clock period for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }
    if (clockPeriodNS < 100) {
        appInfo->prtError(opType, "Invalid clock period for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }        
    (*paramIter)++;

    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No bit order specified for '" + mapFromOpType(opType) + "'");
        return false;
    }
    if ((*paramIter)->compare("msb")) {
        bitOrder = GPIO_MSB_FIRST;
    } else if ((*paramIter)->compare("lsb")) {
        bitOrder = GPIO_LSB_FIRST;
    } else {
        appInfo->prtError(opType, "Invalid bit order for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }
    (*paramIter)++;

    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No value specified for '" + mapFromOpType(opType) + "'");
        return false;
    }
    if (!getInteger(**paramIter, value)) {
        appInfo->prtError(opType, "Invalid value for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }
    if ((value < 0) || (value > 255)) {
        appInfo->prtError(opType, "Invalid value for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }        
    (*paramIter)++;

    return true;
}

string ShiftOutOperation::toString() {
    string s;
    s = Operation::toString()
            + " DataPin:" + to_string(dataPinN)
            + " ClockPin:" + to_string(clockPinN)
            + " ClockPeriodNS:" + to_string(clockPeriodNS);
    if (bitOrder == GPIO_MSB_FIRST) {
        s = s + " BitOrder:msb";
    } else {
        s = s + " BitOrder:lsb";
    }
    s = s + " Value:" + to_string(value);
    
    return s;
}

bool ShiftOutOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    ForkAccess::stop(dataPinN);
    ForkAccess::stop(clockPinN);

    string s = "Performing ShiftOut operation:";
                + " DataPin:" + to_string(dataPinN)
            + " ClockPin:" + to_string(clockPinN)
            + " ClockPeriodNS:" + to_string(clockPeriodNS);
    if (bitOrder == GPIO_MSB_FIRST) {
        s = s + " BitOrder:msb";
    } else {
        s = s + " BitOrder:lsb";
    }
    s = s + " Value:" + to_string(value);

    appInfo->prtReport(s);

    GPIOShiftOut * so = new GPIOShiftOut(dataPinN, clockPinN);
    so->setBitOrder(bitOrder);
    so->setClockPeriodNS(clockPeriodNS);
    so->write(value);

    delete so;
    
    return true;
}

string ShiftOutOperation::help() {
    string hStr;
    hStr = "shiftout <datapin> <clockpin> <clockperiodNS> <bitorder> <value>";
    hStr = hStr + "\n\tPerforms a serial output of a value on a data pin";
    hStr = hStr + "\n\tclocked by a clock pin";
    hStr = hStr + "\n\t<datapin> is the pin number for the data. Must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    hStr = hStr + "\n\t<clockpin> is the pin number for the clock. Must be one of:";
    hStr = hStr + "\n\t\t0,1,6,7,8,12,13,14,15,16,17,18,19,23,26";
    hStr = hStr + "\n\t<datapin> and <clockpin> must be different";
    hStr = hStr + "\n\t<clockperiodNS> is the period of the clock in nano-seconds";
    hStr = hStr + "\n\t\tMust not be less than 100";
    hStr = hStr + "\n\t<bitorder> is the order the bits are transferred";
    hStr = hStr + "\n\t\tMust be one of";
    hStr = hStr + "\n\t\t\tmsb - most significant bit first";
    hStr = hStr + "\n\t\t\tlsb - least significant bit first";
    hStr = hStr + "\n\t<value> is the value sent";
    hStr = hStr + "\n\t\tMust be >= 0 and <= 255";
    
    return hStr;
}
