#ifndef _WAVEFORM_H_
#define _WAVEFORM_H_

class Waveform {
protected:
// could make a LUT and do like envelop to get value between 2 points
public:
// Param: edge sharpness for sine

// option1
// Param: point1 x
// Param: point1 y
// Param: point2 x
// Param: point2 y
// Param: width

// option2
// Param: top x
// Param: top w
// Param: bottom w

// option3
// Param: stairs number

// option4
// Param: width -> 0 would be a triangle
//              -> > 0 would be the % of the width on top vs bottom
//              -> < 0 would be the % of the width on bottom vs top
// Param: angle from sawtooth to triangle
};

#endif
