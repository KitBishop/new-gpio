#ifndef ShiftInOperation_H
#define ShiftInOperation_H

#include "Operation.h"
#include "GPIOTypes.h"

class ShiftInOperation : public Operation {
public:
    ShiftInOperation();
    virtual string toString();
    
    virtual bool execute(AppInfo * appInfo);

    static string help();
    
    int dataPinN;
    int clockPinN;
    long int clockPeriodNS;
    GPIO_Bit_Order bitOrder;
    
protected:    
    virtual bool build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter);
};

#endif
