#include <unistd.h>
#include <string>
#include <cstdlib>

#include "WhileOperation.h"
#include "Utilities.h"
#include "Processing.h"

using namespace std;

WhileOperation::WhileOperation()
    : Operation(opWhile) {
    condition = NULL;
    body = NULL;
}

bool WhileOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    condition = Condition::create(this, appInfo, paramList, paramIter);
    if (condition == NULL) {
        return false;
    }

    body = new list<Node *>();
    
    return Processing::convertParamsToNodes(appInfo, body, paramList, paramIter, opWhile, NULL);
}

string WhileOperation::toString() {
    return Operation::toString() + " " + condition->toString();
}

bool WhileOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    bool ok = true;

    appInfo->prtReport("Starting while loop with condition:" + condition->toString());
    while (ok && condition->execute(this, appInfo) && !appInfo->breaking) {
        appInfo->prtReport("Starting loop body");
        ok = Processing::executeNodes(appInfo, body);
    }

    if (appInfo->breaking) {
        appInfo->breaking = false;
    }

    return ok;
}

string WhileOperation::help() {
    string hStr;
    hStr = "while <conditional-expression>";
    hStr = hStr + "\nendwhile";
    hStr = hStr + "\n\tRepeatedly execute all operations between 'while' and 'endwhile'";
    hStr = hStr + "\n\tso long as <conditional-Expression> is true";
    hStr = hStr + "\n\t<conditional-expression> is true if the expression evaluates";
    hStr = hStr + "\n\tto non-zero.";
    hStr = hStr + "\n\t<conditional-expression> must be enclosed in \" characters";
    hStr = hStr + "\n\tif it contains spaces or other special character.";
    hStr = hStr + "\n\tFormulated as a standard integer expression using:";
    hStr = hStr + "\n\t- Parentheses: ( and )";
    hStr = hStr + "\n\t- Unary Operators: + - ! ~";
    hStr = hStr + "\n\t- Binary Operators: * / % + - >> << > >= < <= == != & | ^ && ||";
    hStr = hStr + "\n\t- Integer constant values";
    hStr = hStr + "\n\t- <varref> which will be replaced by actual value at run time.";
    hStr = hStr + "\n\t  A <varref> is one of:";
    hStr = hStr + "\n\t  - <variable-name> = current value of variable";
    hStr = hStr + "\n\t    as assigned in an 'assign' operation ";
    hStr = hStr + "\n\t    or 0 if variable has never been assigned";
    hStr = hStr + "\n\t  - $n or $nn = current value of pin n or nn";
    hStr = hStr + "\n\t  - $! = value of latest result set by earlier commands";
    hStr = hStr + "\n\t  - $? = status of last executed command: 0=error, 1=ok";
    hStr = hStr + "\n\t  - $[<file-name>] = file <file-name> exists; 0=no, 1=yes";
    
    return hStr;
}
