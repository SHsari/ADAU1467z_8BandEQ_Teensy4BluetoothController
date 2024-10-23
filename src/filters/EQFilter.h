#ifndef EQFILTER_H
#define EQFILTER_H
#include <cmath>

const float minLogFrequency = 1.301029995663981; // log10(20)
const float logFrequencyRange = 3.0;  // log10(20000) - log10(20)
const float maxdB = 18.0;
const float dBRange = maxdB * 2;

double sampleRate = 44100.0;
double pi2 = M_PI * 2;

namespace Calculate {
    static double freq(float normX) {
        return pow(10, (minLogFrequency + normX * logFrequencyRange));
    }

    static double gain(float normY) {
        return normY * 2 * maxdB - maxdB;
    }

    static double passQ(float normY) {
        return pow(10, (normY * dBRange - maxdB) / 20);
    }

    static double peakQ(float normZ) {
        return pow(10, normZ * 2.50515 - 1);
    }

    static double shelfQ(float normZ) {
        return normZ * 4.6 + 0.4;
    }

    static double passQwithGain(float gain) {
        float normValue = gain / dBRange + 0.5;
        return pow(10, (normValue * dBRange - maxdB) / 20);
    }
};


struct Biquad {
    double a0 = 1;
    double a1 = 0;
    double a2 = 0;
    double b0 = 1;
    double b1 = 0;
    double b2 = 0;
    
    void normalize() {
        a1 = -a1/a0; a2 = -a2/a0;
        b0 = b0/a0; b1 = b1/a0;
        b2 = b2/a0; a0 = 1;
    }

    void print() {
        Serial.print("biquad_Print()_ a1, a2 inverted\n a1: ");
        Serial.print(a1);
        Serial.print(",  a2: "); Serial.print(a2);
        Serial.print(",  b0: "); Serial.print(b0);
        Serial.print(",  b1: "); Serial.print(b1);
        Serial.print(",  b2: "); Serial.print(b2);
        Serial.print("  end\n");
    }

};


class EQFilter {
public:
    float freq;
    double w0, cosw0;
    double alpha;

    Biquad *coef;

    void setBiquad(Biquad & biquad) {
        coef = &biquad;
    }

    virtual void setCoefs() {}
    virtual void setX(float x) {}
    virtual void setY(float y) {}
    virtual void setZ(float z) {}
};

#endif