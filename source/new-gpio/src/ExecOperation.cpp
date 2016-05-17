#include <unistd.h>
#include <string>
#include <cstdlib>

#include "ExecOperation.h"
#include "Utilities.h"

using namespace std;

ExecOperation::ExecOperation()
    : Operation(opExec) {
    command = "";
}

bool ExecOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No command specified for '" + mapFromOpType(opType) + "'");
        return false;
    }

    command = **paramIter;

    (*paramIter)++;

    return true;
}

string ExecOperation::toString() {
    return Operation::toString()
            + " Command:" + command;
}

bool ExecOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    string cmdex = appInfo->replaceVars(command);
    
    appInfo->prtReport("Executing command:" + cmdex);
    
    appInfo->result = system(cmdex.c_str());
    
    return true;
}

string ExecOperation::help() {
    string hStr;
    hStr = "exec <command>";
    hStr = hStr + "\n\tExecutes the given command";
    hStr = hStr + "\n\t<command> is the command to execute.";
    hStr = hStr + "\n\tMust be enclosed in \" characters if it contains spaces";
    hStr = hStr + "\n\tor other special character";
    hStr = hStr + "\n\tThe <command> may contain any number of <varsub>s which will be";
    hStr = hStr + "\n\treplaced by actual values at run time.";
    hStr = hStr + "\n\tA <varsub> is any sequence like {<varref>}";
    hStr = hStr + "\n\twhere '<varref>' is one of:";
    hStr = hStr + "\n\t  - <variable-name> = current value of variable";
    hStr = hStr + "\n\t    as assigned in an 'assign' operation ";
    hStr = hStr + "\n\t    or not substituted if variable has never been assigned";
    hStr = hStr + "\n\t  - $n or $nn = current value of pin n or nn";
    hStr = hStr + "\n\t  - $! = value of latest result set by earlier commands";
    hStr = hStr + "\n\t  - $? = status of last executed command: 0=error, 1=ok";
    hStr = hStr + "\n\t  - $[<file-name>] = file <file-name> exists; 0=no, 1=yes";

    return hStr;
}
