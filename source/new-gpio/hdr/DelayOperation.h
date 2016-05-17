#ifndef DelayOperation_H
#define DelayOperation_H

#include "Operation.h"

class DelayOperation : public Operation {
public:
    DelayOperation();
    virtual string toString();
  
    virtual bool execute(AppInfo * appInfo);

    static string help();

    int durationMS;

protected:    
    virtual bool build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter);
};



#endif
