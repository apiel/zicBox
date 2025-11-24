/** Description:
This C/C++ header file serves as a simple utility toolkit, providing a fundamental function used frequently in programming, especially in graphics, physics simulations, or user interface design.

**Overall Purpose**

The primary function of this file is to define a macro called `CLAMP`. This feature ensures that a numerical value is always kept within a defined, safe range. It acts as an automatic boundary setter.

**How the CLAMP Function Works**

The `CLAMP` function accepts three inputs: the number you want to check, a minimum limit (the floor), and a maximum limit (the ceiling).

The process is straightforward:

1.  If your input number is larger than the defined maximum limit, the function ignores the input number and returns the maximum limit instead.
2.  If your input number is smaller than the defined minimum limit, the function returns the minimum limit instead.
3.  If the number is already between the minimum and maximum limits, the function returns the original number unchanged.

In essence, `CLAMP` ensures a number cannot stray outside the specified boundary, preventing potential glitches or overflow errors in the main program.

**Safety Measures**

The file includes standard safeguards (`#pragma once` and `#ifndef`) to guarantee that this definition is only processed once by the compiler, regardless of how many times the file is included in a larger project. This prevents conflicts and duplication errors.

sha: 495a7738bc487e96f2ebc489f253965a28ff06f91c7eb47999805fc7784bfa22 
*/
#pragma once

#ifndef CLAMP
#define CLAMP(x, _min, _max) ((x) < (_max) ? ((x) > (_min) ? (x) : (_min)) : (_max))
#endif
