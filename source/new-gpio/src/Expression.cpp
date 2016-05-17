#include <stack>
#include <iostream>

#include "Expression.h"
#include "Utilities.h"

/*
ToDo:
Docs for:
Asssign
If
While
Exec        
(in Condition, Assign/Expression, Exec)
a. Proper variable names
b. Syntax for pin values

$? - last status
$! - last result 
$n or $nn - pin nn value
a**** - variable name        
*/

Expression::Expression(OperationType parOp, AppInfo * appInf) {
    appInfo = appInf;
    parentOp = parOp;
    rpnList = list<ExprToken>();
    expError = false;
}

Expression * Expression::create(OperationType parOp, string expStr, AppInfo * appInf) {
    Expression * exp = new Expression(parOp, appInf);

    int initialOffset = 0;

    if (exp->expr(expStr, initialOffset, '\0')) {
        if(exp->expError) {
            delete exp;
            return NULL;
        } else if (initialOffset != expStr.length()) {
            appInf->prtError(parOp, "Error in expression:'" + expStr + "' at offset:" + to_string(initialOffset));
            delete exp;
            return NULL;
        }
        return exp;
    } else {
        delete exp;
        return NULL;
    }
}

bool Expression::eval(long int &val) {
    stack<long int> valStack = stack<long int>();
    val = 0;
    
    for(auto tk : rpnList) {
        if (tk.type == ttOperand) {
            if (tk.operandCategory == ocNum) {
                valStack.push(tk.numberVal);
            } else if (tk.operandCategory == ocVar) {
                long int v = appInfo->getVar(tk.variableName);
                valStack.push(v);
            } else /* ocFile */ {
                long int v = appInfo->fileExists(tk.variableName);
                valStack.push(v);
            }
        } else {
            if(tk.operatorCategory == ocUnaryOp) {
                if (valStack.empty()) {
                    appInfo->prtError(parentOp, "Stack error evaluating expression");
                    return false;
                }
                long int v = valStack.top();
                valStack.pop();
                switch (tk.operatorname) {
                    case onUPlus:
                        valStack.push(v);
                        break;
                    case onUMinus:
                        valStack.push(-v);
                        break;
                    case onLogNot:
                        valStack.push(v == 0 ? 1 : 0);
                        break;
                    case onBitNot:
                        valStack.push(~v);
                        break;
                }
            } else {
                if (valStack.empty()) {
                    appInfo->prtError(parentOp, "Stack error evaluating expression");
                    return false;
                }
                long int v2 = valStack.top();
                valStack.pop();
                if (valStack.empty()) {
                    appInfo->prtError(parentOp, "Stack error evaluating expression");
                    return false;
                }
                long int v1 = valStack.top();
                valStack.pop();
                switch (tk.operatorname) {
                    case onMul:
                        valStack.push(v1 * v2);
                        break;
                    case onDiv:
                        if (v2 == 0) {
                            appInfo->prtError(parentOp, "Divide by zero evaluating expression");
                            return false;
                        }
                        valStack.push(v1 / v2);
                        break;
                    case onRem:
                        if (v2 == 0) {
                            appInfo->prtError(parentOp, "Divide by zero evaluating expression");
                            return false;
                        }
                        valStack.push(v1 % v2);
                        break;
                    case onAdd:
                        valStack.push(v1 + v2);
                        break;
                    case onSub:
                        valStack.push(v1 - v2);
                        break;
                    case onShiftR:
                        valStack.push(v1 >> v2);
                        break;
                    case onShiftL:
                        valStack.push(v1 << v2);
                        break;
                    case onLT:
                        valStack.push((v1 < v2) ? 1 : 0);
                        break;
                    case onGT:
                        valStack.push((v1 > v2) ? 1 : 0);
                        break;
                    case onLTE:
                        valStack.push((v1 <= v2) ? 1 : 0);
                        break;
                    case onGTE:
                        valStack.push((v1 >= v2) ? 1 : 0);
                        break;
                    case onEQ:
                        valStack.push((v1 == v2) ? 1 : 0);
                        break;
                    case onNE:
                        valStack.push((v1 != v2) ? 1 : 0);
                        break;
                    case onBitAnd:
                        valStack.push(v1 & v2);
                        break;
                    case onBitOr:
                        valStack.push(v1 | v2);
                        break;
                    case onBitXor:
                        valStack.push(v1 ^ v2);
                        break;
                    case onLogAnd:
                        valStack.push(((v1 != 0) && (v2 != 0)) ? 1 : 0);
                        break;
                    case onLogOr:
                        valStack.push(((v1 != 0) || (v2 != 0)) ? 1 : 0);
                        break;
                }
            }
        }
    }
    
    if (valStack.size() != 1) {
        appInfo->prtError(parentOp, "Stack error evaluating expression");
        return false;
    }
    val = valStack.top();
    return true;
}

bool Expression::expr(string expStr, int &offset, char termChar) {
    while ((offset < expStr.length()) && (expStr.at(offset) == ' ')) {
        offset++;
    }
    if ((offset >= expStr.length() || expError)) {
        return false;
    }
    if (term(expStr, offset)) {
        bool isOk = true;

        while (isOk) {
            ExprToken addOpTk;
            if (addop(addOpTk, expStr, offset)) {
                if (term(expStr, offset)) {
                    rpnList.push_back(addOpTk);
                } else {
                    isOk = false;
                }
            } else {
                isOk = false;
            }
        }

        if ((termChar == '\0') && (offset >= expStr.length())) {
            return true;
        }

        if ((termChar == ')') && (offset < expStr.length()) && (expStr.at(offset) == ')')) {
            offset++;
            return true;
        }

        appInfo->prtError(parentOp, "Error in expression:'" + expStr + "' at offset:" + to_string(offset));
        expError = true;
        
        return false;
    }

    return false;
}

bool Expression::addop(ExprToken &addOpTk, string expStr, int &offset) {
    while ((offset < expStr.length()) && (expStr.at(offset) == ' ')) {
        offset++;
    }
    if ((offset >= expStr.length() || expError)) {
        return false;
    }

    int origOff = offset;

    addOpTk.type = ttOperator;
    addOpTk.operatorCategory = ocAddOp;

    bool isOk = true;

    char c1 = expStr.at(offset);
    char c2;
    switch (c1) {
        case '+':
            offset++;
            addOpTk.operatorname = onAdd;
            break;

        case '-':
            offset++;
            addOpTk.operatorname = onSub;
            break;

        case '>':
            offset++;
            if (offset >= expStr.length()) {
                isOk = false;
            } else {
                c2 = expStr.at(offset);            
                switch (c2) {
                    case '>':
                        offset++;
                        addOpTk.operatorname = onShiftR;
                        break;

                    case '=':
                        offset++;
                        addOpTk.operatorname = onGTE;
                        break;

                    default:
                        addOpTk.operatorname = onGT;
                }
            }
            break;

        case '<':
            offset++;
            if (offset >= expStr.length()) {
                isOk = false;
            } else {
                c2 = expStr.at(offset);            
                switch (c2) {
                    case '<':
                        offset++;
                        addOpTk.operatorname = onShiftL;
                        break;

                    case '=':
                        offset++;
                        addOpTk.operatorname = onLTE;
                        break;

                    default:
                        addOpTk.operatorname = onLT;
                }
            }
            break;

        case '&':
            offset++;
            if (offset >= expStr.length()) {
                isOk = false;
            } else {
                c2 = expStr.at(offset);            
                switch (c2) {
                    case '&':
                        offset++;
                        addOpTk.operatorname = onLogAnd;
                        break;

                    default:
                        addOpTk.operatorname = onBitAnd;
                }
            }
            break;

        case '|':
            offset++;
            if (offset >= expStr.length()) {
                isOk = false;
            } else {
                c2 = expStr.at(offset);            
                switch (c2) {
                    case '|':
                        offset++;
                        addOpTk.operatorname = onLogOr;
                        break;

                    default:
                        addOpTk.operatorname = onBitOr;
                }
            }
            break;

        case '^':
            offset++;
            addOpTk.operatorname = onBitXor;
            break;

        case '=':
            offset++;
            if (offset >= expStr.length()) {
                isOk = false;
            } else {
                c2 = expStr.at(offset);            
                if (c2 == '=') {
                    offset++;
                    addOpTk.operatorname = onEQ;
                } else {
                    isOk = false;
                }
            }
            break;
            
        case '!':
            offset++;
            if (offset >= expStr.length()) {
                isOk = false;
            } else {
                c2 = expStr.at(offset);            
                if (c2 == '=') {
                    offset++;
                    addOpTk.operatorname = onNE;
                } else {
                    isOk = false;
                }
            }
            break;
            
        default:
            isOk = false;
            break;
    }

    if (!isOk) {
        offset = origOff;
    }
    return isOk;
}

bool Expression::term(string expStr, int &offset) {
    while ((offset < expStr.length()) && (expStr.at(offset) == ' ')) {
        offset++;
    }
    if ((offset >= expStr.length() || expError)) {
        return false;
    }
    if (factor(expStr, offset)) {

        bool isOk = true;

        while (isOk) {
            ExprToken mulOpTk;
            if (mulop(mulOpTk, expStr, offset)) {
                if (factor(expStr, offset)) {
                    rpnList.push_back(mulOpTk);
                } else {
                    isOk = false;
                }
            } else {
                isOk = false;
            }
        }

        return true;
    }

    return false;
}

bool Expression::mulop(ExprToken &mulOpTk, string expStr, int &offset) {
    while ((offset < expStr.length()) && (expStr.at(offset) == ' ')) {
        offset++;
    }
    if ((offset >= expStr.length() || expError)) {
        return false;
    }

    int origOff = offset;

    mulOpTk.type = ttOperator;
    mulOpTk.operatorCategory = ocMulOp;

    bool isOk = true;

    char c1 = expStr.at(offset);

    switch (c1) {
        case '*':
            offset++;
            mulOpTk.operatorname = onMul;
            break;

        case '/':
            offset++;
            mulOpTk.operatorname = onDiv;
            break;

        case '%':
            offset++;
            mulOpTk.operatorname = onRem;
            break;

        default:
            isOk = false;
            break;
    }

    if (!isOk) {
        offset = origOff;
    }
    return isOk;
}

bool Expression::factor(string expStr, int &offset) {
    while ((offset < expStr.length()) && (expStr.at(offset) == ' ')) {
        offset++;
    }
    if ((offset >= expStr.length() || expError)) {
        return false;
    }
    ExprToken unOpTk;
    bool haveUnOp = false;
    
    if (unaryop(unOpTk, expStr, offset)) {
        haveUnOp = true;
    }
    
    if (baseitem(expStr, offset)) {
        if (haveUnOp) {
            rpnList.push_back(unOpTk);
        }
        return true;
    }
    return false;
}

bool Expression::unaryop(ExprToken &unOpTk, string expStr, int &offset) {
    while ((offset < expStr.length()) && (expStr.at(offset) == ' ')) {
        offset++;
    }
    if ((offset >= expStr.length() || expError)) {
        return false;
    }

    int origOff = offset;

    unOpTk.type = ttOperator;
    unOpTk.operatorCategory = ocUnaryOp;

    bool isOk = true;

    char c1 = expStr.at(offset);

    switch (c1) {
        case '+':
            offset++;
            unOpTk.operatorname = onUPlus;
            break;

        case '-':
            offset++;
            unOpTk.operatorname = onUMinus;
            break;

        case '!':
            offset++;
            unOpTk.operatorname = onLogNot;
            break;

        case '~':
            offset++;
            unOpTk.operatorname = onBitNot;
            break;

        default:
            isOk = false;
            break;
    }

    if (!isOk) {
        offset = origOff;
    }
    return isOk;
}

bool Expression::baseitem(string expStr, int &offset) {
    while ((offset < expStr.length()) && (expStr.at(offset) == ' ')) {
        offset++;
    }
    if ((offset >= expStr.length() || expError)) {
        expError = true;
        return false;
    }

    bool isOk = true;
    char c1 = expStr.at(offset);
    if (c1 == '(') {
        offset++;
        return expr(expStr, offset, ')');
    } else if ((c1 >= '0') && (c1 <= '9')) {
        long int v = c1 - '0';
        offset++;
        bool done = false;
        while ((offset < expStr.length()) && !done) {
            char c2 = expStr.at(offset);
            if ((c2 >= '0') && (c2 <= '9')) {
                v = (v * 10) +(c2 - '0');
                offset++;
            } else {
                done = true;
            }
        }
        ExprToken numTk;
        numTk.type = ttOperand;
        numTk.operandCategory = ocNum;
        numTk.numberVal = v;
        rpnList.push_back(numTk);
    } else if (c1 == '$') {
        offset++;
        if (offset < expStr.length()) {
            char c2 = expStr.at(offset);
            if ((c2 == '!') || (c2 == '?')) {
                offset++;
                string onm = "";
                onm = onm + c1 + c2;
                ExprToken varTk;
                varTk.type = ttOperand;
                varTk.operandCategory = ocVar;
                varTk.variableName = onm;
                rpnList.push_back(varTk);
            } else if ((c2 >= '0') && (c2 <= '9')) {
                long int v = c2 - '0';
                offset++;
                bool done = false;
                while ((offset < expStr.length()) && !done) {
                    char c3 = expStr.at(offset);
                    if ((c3 >= '0') && (c3 <= '9')) {
                        v = (v * 10) +(c3 - '0');
                        offset++;
                    } else {
                        done = true;
                    }
                }
                string onm = "";
                onm = onm + c1 + to_string(v);
                ExprToken varTk;
                varTk.type = ttOperand;
                varTk.operandCategory = ocVar;
                varTk.variableName = onm;
                rpnList.push_back(varTk);
            } else if (c2 == '[') {
                string onm= "";
                offset++;
                bool done = false;
                while ((offset < expStr.length()) && !done) {
                    char c3 = expStr.at(offset);
                    if (c3 == ']') {
                        done = true;
                    } else {
                        onm = onm + c3;
                    }
                    offset++;
                }
                if (!done) {
                    isOk = false;
                } else {
                    ExprToken varTk;
                    varTk.type = ttOperand;
                    varTk.operandCategory = ocFile;
                    varTk.variableName = onm;
                    rpnList.push_back(varTk);
                }
            } else {
                isOk = false;
            }
        } else {
            isOk = false;
        }
    } else if (((c1 >= 'a') && (c1 <= 'z')) || ((c1 >= 'A') && (c1 <= 'Z'))) {
        string onm = "";
        onm = onm + c1;
        offset++;
        bool done = false;
        while ((offset < expStr.length()) && !done) {
            char c2 = expStr.at(offset);
            if (((c2 >= 'a') && (c2 <= 'z')) || ((c2 >= 'A') && (c2 <= 'Z')) || ((c2 >= '0') && (c2 <= '9'))) {
                onm = onm + c2;
                offset++;
            } else {
                done = true;
            }
        }
        ExprToken varTk;
        varTk.type = ttOperand;
        varTk.operandCategory = ocVar;
        varTk.variableName = onm;
        rpnList.push_back(varTk);
    } else {
        isOk =false;
    } 
    if (!isOk) {
        appInfo->prtError(parentOp, "Error in expression:'" + expStr + "' at offset:" + to_string(offset));
        expError = true;
    }
    
    return isOk;
}

/*
expr --> term { addop term }*
term --> factor { mulop factor }*
factor --> unaryop baseitem | baseitem
baseitem --> number | variable | ( expr )
number --> digit number | digit
digit --> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
variable --> { variableselector }
variableselector --> lower case letter a-z | upper case letter A-Z | ? | !
        
Operators higher in the chart have a higher precedence, meaning that the C compiler evaluates them first. Operators on the same line in the chart have the same precedence, and the "Associativity" column on the right gives their evaluation order.

Operator Precedence Chart
Operator Type       Precedence  Operator    Desc            Associativity   Category    Name
=============       ==========  ========    ====            =============   ========    ====
Unary Operators         1       +           unary plus      right-to-left   unaryop     uplus
                                -           unary minus                                 uminus
                                !           logical NOT                                 lognot
                                ~           bitwise NOT                                 bitnot
Binary Operators        2       *           multiplication  left-to-right   mulop       mul
                                /           division                                    div
                                %           remainder                                   rem
                        3       +           addition                        addop       add
                                -           subtraction                                 sub
                        4       >>          bit shift right                 shiftop     shiftr
                                <<          bit shift left                              shiftl
                        5       <           less than                       relop       lt
                                >           greater than                                gt
                                <=          less than or equal                          lte
                                >=          greater than or equal                       gte
                                ==          equal                                       eq
                                !=          not equal                                   ne
                        6       &           bitwise AND                     logop       bitand
                                ^           bitwise XOR                                 bitxor
                                |           bitwise OR                                  bitor
                                &&          logical AND                                 logand
                                ||          logical OR                                  logor
 */

/*
expr --> expr + term | term
term --> term * factor | factor
factor --> - ( expr ) | ( expr ) | signed-number
signed-number --> - number | number
number --> number digit | digit
digit --> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 */
