#pragma once

#ifndef CLAMP
#define CLAMP(x, _min, _max) ((x) < (_max) ? ((x) > (_min) ? (x) : (_min)) : (_max))
#endif
