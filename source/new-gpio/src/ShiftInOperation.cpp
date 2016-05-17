#include <unistd.h>
#include <string>

#include "ShiftInOperation.h"
#include "Utilities.h"
#include "GPIOShiftIn.h"
#include "GPIOAccess.h"
#include "ForkAccess.h"

using namespace std;

ShiftInOperation::ShiftInOperation()
    : Operation(opShiftIn) {
    dataPinN = 0;
    clockPinN = 0;
    clockPeriodNS = 2000;
    bitOrder = GPIO_MSB_FIRST;
}

bool ShiftInOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
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

    return true;
}

string ShiftInOperation::toString() {
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
    
    return s;
}

bool ShiftInOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    ForkAccess::stop(dataPinN);
    ForkAccess::stop(clockPinN);

    string s = "Performing ShiftIn operation:";
                + " DataPin:" + to_string(dataPinN)
            + " ClockPin:" + to_string(clockPinN)
            + " ClockPeriodNS:" + to_string(clockPeriodNS);
    if (bitOrder == GPIO_MSB_FIRST) {
        s = s + " BitOrder:msb";
    } else {
        s = s + " BitOrder:lsb";
    }

    appInfo->prtReport(s);

    GPIOShiftIn * si = new GPIOShiftIn(dataPinN, clockPinN);
    si->setBitOrder(bitOrder);
    si->setClockPeriodNS(clockPeriodNS);
    int value = si->read();

    appInfo->prtOutput(to_string(value));
    appInfo->result = value;

    delete si;
    
    return true;
}

string ShiftInOperation::help() {
    string hStr;
    hStr = "shiftin <datapin> <clockpin> <clockperiodNS> <bitorder>";
    hStr = hStr + "\n\tPerforms a serial input of a value from a data pin";
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
    
    return hStr;
}
