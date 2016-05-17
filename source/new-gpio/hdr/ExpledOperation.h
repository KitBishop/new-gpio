#ifndef ExpledOperation_H
#define ExpledOperation_H

#include "Operation.h"

class ExpledOperation : public Operation {
public:
    ExpledOperation();
    virtual string toString();
    
    virtual bool execute(AppInfo * appInfo);

    static string help();
    
    int redValue;
    int greenValue;
    int blueValue;

protected:    
    virtual bool build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter);
};

#endif
