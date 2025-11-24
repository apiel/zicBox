/** Description:
This header file defines a useful, self-contained utility function designed to safely convert a decimal number (a float) into a formatted text string.

### Purpose and Functionality

The primary goal of this code is to give developers absolute control over how a number appears when it needs to be displayed as text. For example, converting the raw number `3.1415926` into the formatted text `"3.14"`.

The function takes two pieces of information: the numerical value itself and an integer that specifies the exact number of digits desired after the decimal point (the precision).

### How the Code Works

The conversion process is handled internally using advanced C++ tools called "string streams," which act like invisible, temporary text buffers.

1.  **Internal Buffer:** A temporary writing space is created.
2.  **Formatting Rules:** The code instructs this temporary buffer to use "fixed" decimal notation (the standard way we write numbers, like 12.50).
3.  **Precision Setting:** It then applies the requested precision, setting the precise cutoff point for decimal places.
4.  **Conversion:** The number is inserted into the buffer, adhering to these rules.
5.  **Output:** Finally, the perfectly formatted text is extracted from the buffer and returned as a standard string.

This approach guarantees that numerical data is consistently and reliably presented in any required text output, avoiding formatting errors often encountered in simpler conversion methods.

sha: 33bc59eede9984793b88c4d46dd7460e493bbc34bfabeb7e23a29f229c9f2b52 
*/
#pragma once

#include <string>
#include <sstream>
#include <iomanip>

std::string fToString(float value, int precision)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}
