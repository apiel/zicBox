#ifndef _HELPERS_FORMAT_H_
#define _HELPERS_FORMAT_H_

#include <string>
#include <sstream>
#include <iomanip>

std::string fToString(float value, int precision)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

#endif
