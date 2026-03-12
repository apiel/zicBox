#pragma once

#include <cstring>

float applyEllipticQuarterBandFilter(float input, float amount, double* w)
{
    if (amount == 0.0f)
        return input;

    const double a01 = -9.1891604652189471;
    const double a02 = 40.177553696870497;
    const double a03 = -110.11636661771178;
    const double a04 = 210.18506612078195;
    const double a05 = -293.84744771903240;
    const double a06 = 308.16345558359234;
    const double a07 = -244.06786780384243;
    const double a08 = 144.81877911392738;
    const double a09 = -62.770692151724198;
    const double a10 = 18.867762095902137;
    const double a11 = -3.5327094230551848;
    const double a12 = 0.31183189275203149;

    const double b00 = 0.00013671732099945628;
    const double b01 = -0.00055538501265606384;
    const double b02 = 0.0013681887636296387;
    const double b03 = -0.0022158566490711852;
    const double b04 = 0.0028320091007278322;
    const double b05 = -0.0029776933151090413;
    const double b06 = 0.0030283628243514991;
    const double b07 = -0.0029776933151090413;
    const double b08 = 0.0028320091007278331;
    const double b09 = -0.0022158566490711861;
    const double b10 = 0.0013681887636296393;
    const double b11 = -0.00055538501265606384;
    const double b12 = 0.00013671732099945636;

    double tmp =
          input
        - ((a01*w[0] + a02*w[1]) + (a03*w[2] + a04*w[3]))
        - ((a05*w[4] + a06*w[5]) + (a07*w[6] + a08*w[7]))
        - ((a09*w[8] + a10*w[9]) + (a11*w[10] + a12*w[11]));

    double y =
          b00*tmp
        + ((b01*w[0] + b02*w[1]) + (b03*w[2] + b04*w[3]))
        + ((b05*w[4] + b06*w[5]) + (b07*w[6] + b08*w[7]))
        + ((b09*w[8] + b10*w[9]) + (b11*w[10] + b12*w[11]));

    memmove(&w[1], &w[0], 11 * sizeof(double));
    w[0] = tmp;

    float filtered = (float)y;

    // dry/wet
    return input + amount * (filtered - input);
}
