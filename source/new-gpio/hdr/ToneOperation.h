#ifndef ToneOperation_H
#define ToneOperation_H

#include "PinOperation.h"

class ToneOperation : public PinOperation {
public:
    ToneOperation();
    virtual string toString();
    
    virtual bool execute(AppInfo * appInfo);

    static string help();
    
    long int freq;
    long int durationMS;
    
protected:    
    virtual bool build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter);
};

#endif
