#ifndef _RANGE_H_
#define _RANGE_H_

#ifndef range
#define range(x, _min, _max) ((x) < (_max) ? ((x) > (_min) ? (x) : (_min)) : (_max))
#endif

#endif
