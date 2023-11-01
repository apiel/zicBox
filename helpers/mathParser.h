#ifndef _MATH_MathParser_H_
#define _MATH_MathParser_H_

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <math.h>
#include <stdexcept>

using namespace std;

namespace MathParser {

enum types { DELIMITER = 1,
    NUMBER,
    FUNCTION };

char* expPtr; // points to the expression
char token[64]; // holds current token
char tokenType; // holds token's type

struct Operator {
    char sign;
    double (*calc)(double, double);
};
Operator operators[] = {
    { '+', [](double a, double b) { return a + b; } },
    { '-', [](double a, double b) { return a - b; } },
    { '*', [](double a, double b) { return a * b; } },
    { '/', [](double a, double b) { return a / b; } },
    { '%', [](double a, double b) { return fmod(a, b); } },
    { '^', [](double a, double b) { return pow(a, b); } },
};
const uint8_t operatorCount = sizeof(operators) / sizeof(Operator);

struct MathFunction {
    const char* name;
    double (*func)(double);
};
MathFunction mathFunctions[] = {
    { "SIN", sin },
    { "COS", cos },
    { "TAN", tan },
    { "ASIN", asin },
    { "ACOS", acos },
    { "ATAN", atan },
    { "SINH", sinh },
    { "COSH", cosh },
    { "TANH", tanh },
    { "ASINH", asinh },
    { "ACOSH", acosh },
    { "ATANH", atanh },
    { "LN", log },
    { "LOG", log10 },
    { "EXP", exp },
    { "SQRT", sqrt },
    { "SQR", [](double a) { return a * a; } },
    { "ROUND", round },
    { "INT", floor },
};
const uint8_t mathFunctionCount = sizeof(mathFunctions) / sizeof(MathFunction);

double evalOperator(double result, uint8_t operatorIndex = 0);

int8_t getFunctionIndex(char* func)
{
    for (int8_t i = 0; i < mathFunctionCount; i++) {
        if (!strcmp(func, mathFunctions[i].name)) {
            return i;
        }
    }
    throw std::runtime_error("Unknown Function " + std::string(func));
}

char* getExpPtr()
{
    while (isspace(*expPtr)) { // skip over white space
        expPtr++;
    }
    return expPtr;
}

bool isOperator()
{
    char c = *getExpPtr();
    for (int8_t i = 0; i < operatorCount; i++) {
        if (c == operators[i].sign) {
            return true;
        }
    }
    return false;
}

bool isDelimiter()
{
    char c = *getExpPtr();
    if (c == '(' || c == ')') {
        return true;
    }
    return isOperator();
}

char* setTokenTillDelimiter(char* temp)
{
    while (*getExpPtr() && !isDelimiter()) {
        *temp++ = toupper(*expPtr++);
    }
    *temp = '\0';
    return temp;
}

void setToken()
{
    tokenType = 0;

    if (!*expPtr) { // at end of expression
        return;
    }

    char* tokenPtr = token;
    *tokenPtr = '\0';

    if (isDelimiter()) {
        tokenType = DELIMITER;
        *tokenPtr = *expPtr++; // advance to next char
    } else if (isalpha(*getExpPtr())) {
        tokenType = FUNCTION;
        tokenPtr = setTokenTillDelimiter(tokenPtr);
    } else if (isdigit(*getExpPtr()) || *getExpPtr() == '.') {
        tokenType = NUMBER;
        tokenPtr = setTokenTillDelimiter(tokenPtr);
    }
}

double evalApply(double result)
{
    char isNegativeValue = *token == '-';
    if (isNegativeValue) {
        setToken();
    }

    int8_t funcIndex = -1;
    if (tokenType == FUNCTION) {
        funcIndex = getFunctionIndex(token);
        setToken();
    }
    if ((*token == '(')) {
        setToken();
        result = evalOperator(result);
        if (*token != ')')
            throw std::runtime_error("Unbalanced Parentheses");
        if (funcIndex > -1) {
            result = mathFunctions[funcIndex].func(result);
        }
        setToken();
    } else if (tokenType == NUMBER) {
        result = atof(token);
        setToken();
    } else {
        throw std::runtime_error("Syntax Error " + std::string(token));
    }

    return (isNegativeValue) ? -result : result;
}

double evalNextOperator(double result, uint8_t operatorIndex)
{
    uint8_t next = operatorIndex + 1;
    if (next >= operatorCount) {
        return evalApply(result);
    }
    return evalOperator(result, next);
}

double evalOperator(double value, uint8_t operatorIndex)
{
    double valueA = evalNextOperator(value, operatorIndex);
    Operator op = operators[operatorIndex];
    while (*token == op.sign) {
        setToken();
        double valueB = evalNextOperator(0, operatorIndex);
        valueA = op.calc(valueA, valueB);
    }
    return valueA;
}

double eval(char* exp)
{
    try {
        printf("evalExp: %s\n", exp);
        double result;
        expPtr = exp;
        setToken();
        if (!*token) {
            throw std::runtime_error("No Expression Present");
            return (double)0;
        }
        return evalOperator(result);
    } catch (const std::exception& e) {
        throw std::runtime_error("MathParser Error: " + std::string(e.what()));
    }
}

}

#endif