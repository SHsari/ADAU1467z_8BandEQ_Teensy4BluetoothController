#ifndef SHELF_H
#define SHELF_H

#include "EQFilter.h"
#include <cmath>

class LowShelf : public EQFilter {
private: 
    double gain = 0;
    double Q = 1.0;
    double A;
    double inter;
public:
    void setCoefs() override {
        alpha = sin(w0) / (2*Q);
        inter = 2*sqrtf(A)*alpha;
        
        coef->b0 = A * ((A+1) - (A-1)*cosw0 + inter);
        coef->b1 = 2 * A * ((A-1) - (A+1)*cosw0);
        coef->b2 = A * ((A+1) - (A-1)*cosw0 - inter);
        coef->a0 = (A+1) + (A-1)*cosw0 + inter;
        coef->a1 = -2 * ((A-1) + (A+1)*cosw0);
        coef->a2 = (A+1) + (A-1)*cosw0 - inter;
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


class HighShelf : public EQFilter {
private:
    double gain = 0;
    double Q = 1.0;
    double A;
    double inter;
    
public:
    void setCoefs() override {
        alpha = sin(w0) / (2*Q);
        inter = 2*sqrtf(A)*alpha;
        

        coef->b0 = A * ((A+1) + (A-1)*cosw0 + inter);
        coef->b1 = -2 * A * ((A-1) + (A+1)*cosw0);
        coef->b2 = A * ((A+1) + (A-1)*cosw0 - inter);
        coef->a0 = (A+1) - (A-1)*cosw0 + inter;
        coef->a1 = 2 * ((A-1) - (A+1)*cosw0);
        coef->a2 = (A+1) - (A-1)*cosw0 - inter;

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