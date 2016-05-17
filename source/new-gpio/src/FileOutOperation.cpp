#include <unistd.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "FileOutOperation.h"
#include "Utilities.h"

using namespace std;

FileOutOperation::FileOutOperation()
    : Operation(opFileOut) {
    fileName = "";
    fileOutExpr = "";
    expression = NULL;
}

bool FileOutOperation::build(AppInfo * appInfo, list<string> * &paramList, list<string>::iterator * &paramIter) {
    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No file name specified for '" + mapFromOpType(opType) + "'");
        return false;
    }

    fileName = **paramIter;

    (*paramIter)++;

    if (*paramIter == paramList->end()) {
        appInfo->prtError(opType, "No expression specified for '" + mapFromOpType(opType) + "'");
        return false;
    }

    fileOutExpr = **paramIter;
    
    expression = Expression::create(opType, fileOutExpr, appInfo);
    
    if (expression == NULL) {
        return false;
    }

    (*paramIter)++;

    return true;
}

string FileOutOperation::toString() {
    return Operation::toString()
            + " FileName:" + fileName
            + " Expression:" + fileOutExpr;
}

bool FileOutOperation::execute(AppInfo * appInfo) {
    if (appInfo->breaking) {
        return true;
    }

    string s = "";
    s = s + "Writing expression:'" + fileOutExpr + "' to file:" + fileName;
    appInfo->prtReport(s);
    
    bool isOk = true;
    long int exprVal;
    if (expression->eval(exprVal)) {
        isOk = writeToFile(fileName, exprVal);
    } else {
        isOk = false;
    }
    
    if (!isOk) {
        appInfo->prtError(opType, "Error writing expression to file:" + fileOutExpr);
    }
    
    return isOk;
}

string FileOutOperation::help() {
    string hStr;
    hStr = "fileout <file-name> <expression>";
    hStr = hStr + "\n\tWrites the value of the expression as the first line";
    hStr = hStr + "\n\tof given file.";
    hStr = hStr + "\n\tAlong with 'filein', and 'filedelete' operations and";
    hStr = hStr + "\n\t$[<filename>] expression item, provides a rudimentary";
    hStr = hStr + "\n\tmechanism for communicating with other programs.";
    hStr = hStr + "\n\t<file-name> is the name of the file to write to.";
    hStr = hStr + "\n\tMust be enclosed in \" characters if it contains spaces";
    hStr = hStr + "\n\tor other special character.";
    hStr = hStr + "\n\t<expression> is the expression value to write.";
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

bool FileOutOperation::writeToFile(string fileName, long int fileVal) {
    ofstream myfile;

    myfile.open(fileName);
    
    if (myfile.is_open()) {
        myfile << fileVal;
        myfile << "\n";

        myfile.close();
        
        return true;
    }
    
    return false;
}
