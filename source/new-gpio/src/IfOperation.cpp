#include <unistd.h>
#include <string>
#include <cstdlib>

#include "IfOperation.h"
#include "Utilities.h"
#include "Processing.h"

using namespace std;

IfOperation::IfOperation()
    : Operation(opIf) {
    condition = NULL;
    thenBody = NULL;
    elseBody = NULL;
}

bool IfOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    condition = Condition::create(this, appInfo, paramList, paramIter);
    if (condition == NULL) {
        return false;
    }

    thenBody = new list<Node *>();
    
    OperationType endOpType;
    bool ok;
    ok = Processing::convertParamsToNodes(appInfo, thenBody, paramList, paramIter, opIf, &endOpType);
    if (ok && (endOpType == opElse)) {
        elseBody = new list<Node *>();
        ok = Processing::convertParamsToNodes(appInfo, elseBody, paramList, paramIter, opElse, NULL);
    }
    
    return ok;
}

string IfOperation::toString() {
    return Operation::toString() + " " + condition->toString();
}

bool IfOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    appInfo->prtReport("Executing if with condition:" + condition->toString());
    bool ok = true;
    if (condition->execute(this, appInfo)) {
        appInfo->prtReport("Executing if then body");
        ok = Processing::executeNodes(appInfo, thenBody);
    } else if (elseBody != NULL) {
        appInfo->prtReport("Executing if else body");
        ok = Processing::executeNodes(appInfo, elseBody);
    }

    return ok;
}

string IfOperation::help() {
    string hStr;
    hStr = "if <conditional-expression>";
    hStr = hStr + "\nendif";
    hStr = hStr + "\nOr:";
    hStr = hStr + "\nif <conditional-expression>";
    hStr = hStr + "\nelse";
    hStr = hStr + "\nendif";
    hStr = hStr + "\n\tExecutes all operations between 'if' and 'else' or";
    hStr = hStr + "\n\t'endif' if the <conditional-expression> is true";
    hStr = hStr + "\n\tIf 'else' is used, executes all operations between 'else' and";
    hStr = hStr + "\n\t'endif' if the <conditional-expression> is false";
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
