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
const uint8_t operatorCount = 6;
Operator operators[operatorCount] = {
    { '+', [](double a, double b) { return a + b; } },
    { '-', [](double a, double b) { return a - b; } },
    { '*', [](double a, double b) { return a * b; } },
    { '/', [](double a, double b) { return a / b; } },
    { '%', [](double a, double b) { return fmod(a, b); } },
    { '^', [](double a, double b) { return pow(a, b); } },
};

void evalExp6(double& result);

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
    char op;
    op = 0;
    if ((tokenType == DELIMITER) && *token == '+' || *token == '-') {
        op = *token;
        getToken();
    }
    evalExp6(result);
    if (op == '-')
        result = -result;
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
    evalNextOperator(result, operatorIndex);
    Operator op = operators[operatorIndex];
    while (*token == op.sign) {
        getToken();
        double valueB;
        evalNextOperator(valueB, operatorIndex);
        result = op.calc(result, valueB);
    }
}

void applyFunction(double& result, char* func)
{
    if (!strcmp(func, "SIN"))
        result = sin(M_PI / 180 * result);
    else if (!strcmp(func, "COS"))
        result = cos(M_PI / 180 * result);
    else if (!strcmp(func, "TAN"))
        result = tan(M_PI / 180 * result);
    else if (!strcmp(func, "ASIN"))
        result = 180 / M_PI * asin(result);
    else if (!strcmp(func, "ACOS"))
        result = 180 / M_PI * acos(result);
    else if (!strcmp(func, "ATAN"))
        result = 180 / M_PI * atan(result);
    else if (!strcmp(func, "SINH"))
        result = sinh(result);
    else if (!strcmp(func, "COSH"))
        result = cosh(result);
    else if (!strcmp(func, "TANH"))
        result = tanh(result);
    else if (!strcmp(func, "ASINH"))
        result = asinh(result);
    else if (!strcmp(func, "ACOSH"))
        result = acosh(result);
    else if (!strcmp(func, "ATANH"))
        result = atanh(result);
    else if (!strcmp(func, "LN"))
        result = log(result);
    else if (!strcmp(func, "LOG"))
        result = log10(result);
    else if (!strcmp(func, "EXP"))
        result = exp(result);
    else if (!strcmp(func, "SQRT"))
        result = sqrt(result);
    else if (!strcmp(func, "SQR"))
        result = result * result;
    else if (!strcmp(func, "ROUND"))
        result = round(result);
    else if (!strcmp(func, "INT"))
        result = floor(result);
    else
        throw std::runtime_error("Unknown Function " + std::string(func));
}

void evalExp6(double& result)
{
    bool isfunc = (tokenType == FUNCTION);
    char temp_token[80];
    if (isfunc) {
        strcpy(temp_token, token);
        getToken();
    }
    if ((*token == '(')) {
        getToken();
        evalOperator(result);
        if (*token != ')')
            throw std::runtime_error("Unbalanced Parentheses");
        if (isfunc) {
            applyFunction(result, temp_token);
        }
        getToken();
    } else if (tokenType == NUMBER) {
        result = atof(token);
        getToken();
    } else {
        throw std::runtime_error("Syntax Error " + std::string(token));
    }
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