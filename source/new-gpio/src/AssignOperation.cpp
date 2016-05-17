#include <unistd.h>
#include <string>
#include <cstdlib>
#include <iostream>

#include "AssignOperation.h"
#include "Utilities.h"

using namespace std;

AssignOperation::AssignOperation()
    : Operation(opAssign) {
    assignName = "";
    assignExpr = "";
    expression = NULL;
}

bool AssignOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No variable specified for '" + mapFromOpType(opType) + "'");
        return false;
    }
    
    bool isOk = true;

    char c = (*paramIter)->at(0);
    
    if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) {
        int i;
        assignName = "";
        for (i = 0; (i < (*paramIter)->length()) && isOk; i++) {
            c = (*paramIter)->at(i);
            if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9'))) {
                assignName = assignName + c;
            } else {
                isOk = false;
            }
        }
    } else {
        isOk = false;
    }
    
    if (!isOk) {
        appInfo->prtError(opType, "Invalid variable name for '" + mapFromOpType(opType) + "':" + **paramIter);
        return false;
    }
    
    (*paramIter)++;

    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No expression specified for '" + mapFromOpType(opType) + "'");
        return false;
    }

    assignExpr = **paramIter;
    
    expression = Expression::create(opType, assignExpr, appInfo);
    
    if (expression == NULL) {
        return false;
    }

    (*paramIter)++;

    return true;
}

string AssignOperation::toString() {
    return Operation::toString()
            + " Variable:" + assignName
            + " Expression:" + assignExpr;
}

bool AssignOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    string s = "";
    s = s + "Assigning:" + assignName + "=" + assignExpr;
    appInfo->prtReport(s);
    
    long int exprVal;
    if (expression->eval(exprVal)) {
        appInfo->setVar(assignName, exprVal);
        appInfo->prtOutput(to_string(exprVal));
        appInfo->result = exprVal;
    } else {
        appInfo->prtError(opType, "Error evaluating expression:" + assignExpr);
        return false;
    }
    
    return true;
}

string AssignOperation::help() {
    string hStr;
    hStr = "assign <variable-name> <expression>";
    hStr = hStr + "\n\tAssigns the given expression to the given variable";
    hStr = hStr + "\n\t<variable-name> is the user variable to assign to.";
    hStr = hStr + "\n\tAny sequence of letters and digits starting with a letter.";
    hStr = hStr + "\n\tNames are case sensitive";
    hStr = hStr + "\n\t<expression> is the expression value to assign.";
    hStr = hStr + "\n\tMust be enclosed in \" characters if it contains spaces";
    hStr = hStr + "\n\tor other special character.";
    hStr = hStr + "\n\tFormulated as a standard integer expression using:";
    hStr = hStr + "\n\t- Parentheses: ( and )";
    hStr = hStr + "\n\t- Unary Operators: + - ! ~";
    hStr = hStr + "\n\t- Binary Operators: * / % + - >> << > >= < <= == != & | ^ && ||";
    hStr = hStr + "\n\t- Integer constant values";
    hStr = hStr + "\n\t- <varref> which will be replaced by actual value at run time.";
    hStr = hStr + "\n\t  A <varref> is one of:";
    hStr = hStr + "\n\t  - <variable-name> as defined above = current value of variable";
    hStr = hStr + "\n\t    or 0 if variable has never been assigned";
    hStr = hStr + "\n\t  - $n or $nn = current value of pin n or nn";
    hStr = hStr + "\n\t  - $! = value of latest result set by earlier commands";
    hStr = hStr + "\n\t  - $? = status of last executed command: 0=error, 1=ok";
    hStr = hStr + "\n\t  - $[<file-name>] = file <file-name> exists; 0=no, 1=yes";
    
    return hStr;
}
