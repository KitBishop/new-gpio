#ifndef Expression_H
#define Expression_H

using namespace std;

#include <list>
#include <string>

#include "AppInfo.h"

enum TokenType {
    ttOperator,
    ttOperand
};

enum OperatorCategory {
    ocUnaryOp,
    ocMulOp,
    ocAddOp
};

enum OperatorName {
    // ocUnaryOp
    onUPlus,
    onUMinus,
    onLogNot,
    onBitNot,
    
    // ocMulOp
    onMul,
    onDiv,
    onRem,
    
    // ocAddOp
    onAdd,
    onSub,
    onShiftR,
    onShiftL,
    onLT,
    onGT,
    onLTE,
    onGTE,
    onEQ,
    onNE,
    onBitAnd,
    onBitOr,
    onBitXor,
    onLogAnd,
    onLogOr
};

enum OperandCategory {
    ocNum,
    ocVar,
    ocFile
};

typedef struct ExprToken{
    TokenType type;
    OperatorCategory operatorCategory;
    OperatorName operatorname;
    OperandCategory operandCategory;
    long int numberVal;
    string variableName;
} ExprToken;

class Expression {
public:
    static Expression * create(OperationType parOp, string expStr, AppInfo * appInf);

    bool eval(long int &val);

private:
    Expression(OperationType parOp, AppInfo * appInf);
    
    bool expr(string expStr, int &offset, char termChar);
    bool term(string expStr, int &offset);
    bool factor(string expStr, int &offset);
    bool baseitem(string expStr, int &offset);
    bool addop(ExprToken &addOpTk, string expStr, int &offset);
    bool mulop(ExprToken &mulOpTk, string expStr, int &offset);
    bool unaryop(ExprToken &unOpTk, string expStr, int &offset);

    list<ExprToken> rpnList;
    AppInfo * appInfo;
    OperationType parentOp;
    bool expError;
};

#endif
