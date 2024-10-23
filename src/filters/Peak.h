#ifndef PEAK_H
#define PEAK_H

#include "EQFilter.h"
#include <cmath>

class Peak : public EQFilter {
private:
    double gain = 0;
    double Q = 1.0;
    double A = 1.0;

public:
    void setCoefs() override {
        alpha = sin(w0) / (2*Q);
        coef->a0 = 1 + alpha/A;
        coef->a1 = -2*cosw0;
        coef->a2 = 1-alpha/A;
        coef->b0 = 1 + alpha*A;
        coef->b1 = -2 * cosw0;
        coef->b2 = 1 - alpha*A;
        coef->normalize();
    }

    void setX(float x) override {
        freq = x;
        w0 = freq*pi2 / sampleRate;
        cosw0 = cos(w0);
    }
    void setY(float y) override {
        gain = y;
        A = pow(10, gain/40);
    }
    void setZ(float z) override {
        Q = z;
    }
};

#endif
