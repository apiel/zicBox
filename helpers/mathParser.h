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

struct MathFunction
{
    const char *name;
    double (*func)(double);
};
MathFunction mathFunctions[] = {
    {"SIN", sin},
    {"COS", cos},
    {"TAN", tan},
    {"ASIN", asin},
    {"ACOS", acos},
    {"ATAN", atan},
    {"SINH", sinh},
    {"COSH", cosh},
    {"TANH", tanh},
    {"ASINH", asinh},
    {"ACOSH", acosh},
    {"ATANH", atanh},
    {"LN", log},
    {"LOG", log10},
    {"EXP", exp},
    {"SQRT", sqrt},
    {"SQR", [](double a) { return a * a; } },
    {"ROUND", round},
    {"INT", floor},
};
const uint8_t mathFunctionCount = sizeof(mathFunctions) / sizeof(MathFunction);

double evalExp6(double result);

char* getExpPtr()
{
    while (isspace(*expPtr)) { // skip over white space
        expPtr++;
    }
    return expPtr;
}

bool isDelimiter()
{
    return strchr("+-/*%^()", *getExpPtr());
}

char* setTokenTillDelimiter(char* temp)
{
    while (!isDelimiter() && (*getExpPtr()))
        *temp++ = toupper(*expPtr++);

    *temp = '\0';
    return temp;
}

void getToken()
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

void evalExp5(double& result)
{
    // printf("evalExp5 result in: %f\n", result);
    char op;
    op = 0;
    if (*token == '+' || *token == '-') {
        op = *token;
        getToken();
    }
    double val = result;
    result = evalExp6(val);
    if (op == '-') {
        result = -result;
    }
    // printf("evalExp5 result out: %f\n", result);
}

void evalOperator(double& result, uint8_t operatorIndex);

void evalNextOperator(double& result, uint8_t operatorIndex)
{
    uint8_t next = operatorIndex + 1;
    if (next >= operatorCount) {
        evalExp5(result);
        return;
    }
    evalOperator(result, next);
}

void evalOperator(double& result, uint8_t operatorIndex = 0)
{
    // printf("operatorIndex: %d result: %f\n", operatorIndex, result);
    evalNextOperator(result, operatorIndex);
    Operator op = operators[operatorIndex];
    while (*token == op.sign) {
        getToken();
        double valueB;
        evalNextOperator(valueB, operatorIndex);
        // printf("operatorIndex: %d valueB: %f\n", operatorIndex, valueB);
        result = op.calc(result, valueB);
    }
}

int8_t getFunctionIndex(char* func)
{
    for (int8_t i = 0; i < mathFunctionCount; i++) {
        if (!strcmp(func, mathFunctions[i].name))
            return i;
    }
    throw std::runtime_error("Unknown Function " + std::string(func));
}

double evalExp6(double result)
{
    bool isfunc = (tokenType == FUNCTION);
    int8_t funcIndex = -1;
    if (isfunc) {
        funcIndex = getFunctionIndex(token);
        getToken();
    }
    if ((*token == '(')) {
        getToken();
        evalOperator(result);
        if (*token != ')')
            throw std::runtime_error("Unbalanced Parentheses");
        if (isfunc) {
            result = mathFunctions[funcIndex].func(result);
        }
        getToken();
    } else if (tokenType == NUMBER) {
        result = atof(token);
        getToken();
    } else {
        throw std::runtime_error("Syntax Error " + std::string(token));
    }
    printf("evalExp6 result out: %f\n", result);
    return result;
}

double eval(char* exp)
{
    try {
        printf("evalExp: %s\n", exp);
        double result;
        expPtr = exp;
        getToken();
        if (!*token) {
            throw std::runtime_error("No Expression Present");
            return (double)0;
        }
        evalOperator(result);
        return result;
    } catch (const std::exception& e) {
        throw std::runtime_error("MathParser Error: " + std::string(e.what()));
    }
}

}

#endif