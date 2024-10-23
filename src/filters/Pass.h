#ifndef PASS_H
#define PASS_H

#include "EQFilter.h"
#include <cmath>

class LowPass : public EQFilter {
private: 
    double Q = 1.0;
public:
    void setCoefs() override {
        alpha = sin(w0) / (2*Q);
        coef->a0 = 1+alpha;
        coef->a1 = -2*cosw0;
        coef->a2 = 1-alpha;
        coef->b0 = (1-cosw0)/2;
        coef->b1 = 1-cosw0;
        coef->b2 = (1-cosw0)/2;
        coef->normalize();
    }

    void setX(float x) override {
        freq = x;
        w0 = freq*pi2 / sampleRate;
        cosw0 = cos(w0);
    }
    void setY(float y) override {
        Q = Calculate::passQwithGain(y);
    }
    void setZ(float z) override { }
};


class HighPass : public EQFilter {
private:
    double Q = 1.0;
public:
    void setCoefs() override {
        alpha = sin(w0) / (2*Q);

        coef->a0 = 1+alpha;
        coef->a1 = -2*cosw0;
        coef->a2 = 1-alpha;
        coef->b0 = (1+cosw0)/2;
        coef->b1 = -(1+cosw0);
        coef->b2 = (1+cosw0)/2;
        coef->normalize();
    }

    void setX(float x) override {
        freq = x;
        w0 = freq*pi2 / sampleRate;
        cosw0 = cos(w0);
    }
    void setY(float y) override {
        Q = Calculate::passQwithGain(y);
    }
    void setZ(float z) override {}
};

#endif