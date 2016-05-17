#ifndef Option_H
#define Option_H

#include <string>

#include "Node.h"
#include "Operation.h"

enum OptOper {
    on,
    off,
    param
};

class Option : public Node {
public:
    Option(char optC, OptOper op, string * param);
    virtual string toString();

    virtual bool execute(AppInfo * appInfo);

    char optChar;
    OptOper oper;
    string * optParam;

    static Option * create(AppInfo * appInfo, string optStr);
    static string operationHelp(OperationType opType);
    static string optionHelp(char optC);
    static string atHelp();
    static string basicHelp(AppInfo * appInfo);
    static string progInfoHelp();
    static string sourcesHelp();
};

#endif
