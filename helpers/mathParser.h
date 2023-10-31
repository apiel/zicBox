#ifndef _MATH_MathParser_H_
#define _MATH_MathParser_H_

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <math.h>
#include <stdexcept>

using namespace std;

enum types { DELIMITER = 1,
    NUMBER,
    FUNCTION };

class MathParser {
    char* exp_ptr; // points to the expression
    char token[64]; // holds current token
    char tok_type; // holds token's type

    void eval_exp1(double& result)
    {
        char op;
        double temp;
        eval_exp3(result);
        while ((op = *token) == '+' || op == '-') {
            getToken();
            eval_exp3(temp);
            result = op == '+' ? result + temp : result - temp;
        }
    }

    void eval_exp3(double& result)
    {
        char op;
        double temp;
        eval_exp4(result);
        while ((op = *token) == '*' || op == '/') {
            getToken();
            eval_exp4(temp);
            result = op == '*' ? result * temp : result / temp;
        }
    }

    void eval_exp4(double& result)
    {
        double temp;
        eval_exp5(result);
        while (*token == '^') {
            getToken();
            eval_exp5(temp);
            result = pow(result, temp);
        }
    }

    void eval_exp5(double& result)
    {
        char op;
        op = 0;
        if ((tok_type == DELIMITER) && *token == '+' || *token == '-') {
            op = *token;
            getToken();
        }
        eval_exp6(result);
        if (op == '-')
            result = -result;
    }

    void eval_exp6(double& result)
    {
        bool isfunc = (tok_type == FUNCTION);
        char temp_token[80];
        if (isfunc) {
            strcpy(temp_token, token);
            getToken();
        }
        if ((*token == '(')) {
            getToken();
            eval_exp1(result);
            if (*token != ')')
                throw std::runtime_error("Unbalanced Parentheses");
            if (isfunc) {
                applyFunction(result, temp_token);
            }
            getToken();
        } else if (tok_type == NUMBER) {
            result = atof(token);
            getToken();
        } else {
            throw std::runtime_error("Syntax Error " + std::string(token));
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

    char* getExpPtr()
    {
        while (isspace(*exp_ptr)) { // skip over white space
            exp_ptr++;
        }
        return exp_ptr;
    }

    bool isDelimiter()
    {
        return strchr("+-/*%^=()", *getExpPtr());
    }

    char* setTokenTillDelimiter(char* temp)
    {
        while (!isDelimiter() && (*getExpPtr()))
            *temp++ = toupper(*exp_ptr++);

        *temp = '\0';
        return temp;
    }

    void getToken()
    {
        tok_type = 0;

        if (!*exp_ptr) { // at end of expression
            return;
        }

        char* temp = token;
        *temp = '\0';

        if (isDelimiter()) {
            tok_type = DELIMITER;
            *temp = *exp_ptr++; // advance to next char
        } else if (isalpha(*getExpPtr())) {
            tok_type = FUNCTION;
            temp = setTokenTillDelimiter(temp);
        } else if (isdigit(*getExpPtr()) || *getExpPtr() == '.') {
            tok_type = NUMBER;
            temp = setTokenTillDelimiter(temp);
        }

        // printf("token: %s type: %d\n", token, tok_type);
    }

public:
    MathParser()
    {
        int i;
        exp_ptr = NULL;
    }
    double eval_exp(char* exp)
    {
        try {
            printf("eval_exp: %s\n", exp);
            double result;
            exp_ptr = exp;
            getToken();
            if (!*token) {
                throw std::runtime_error("No Expression Present");
                return (double)0;
            }
            eval_exp1(result);
            return result;
        } catch (const std::exception& e) {
            throw std::runtime_error("MathParser Error: " + std::string(e.what()));
        }
    }
};

#endif