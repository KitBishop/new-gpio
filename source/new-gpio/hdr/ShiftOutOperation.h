#ifndef ShiftOutOperation_H
#define ShiftOutOperation_H

#include "Operation.h"
#include "GPIOTypes.h"

class ShiftOutOperation : public Operation {
public:
    ShiftOutOperation();
    virtual string toString();
    
    virtual bool execute(AppInfo * appInfo);

    static string help();
    
    int dataPinN;
    int clockPinN;
    long int clockPeriodNS;
    GPIO_Bit_Order bitOrder;
    
    int value;

protected:    
    virtual bool build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter);
};

#endif
