//  ADSR_Linear.h
//  Linear envelope generator
//
//  Copyright (C) 2018 Jeff Gregorio
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef ADSR_Linear_h
#define ADSR_Linear_h

#include <math.h>

class ADSR_Linear {
  
    // Envelope states
    enum {
      kADSRStateIdle = 0,
      kADSRStateAttack,
      kADSRStateDecay,
      kADSRStateSustain,
      kADSRStateRelease
    };
    
public:

    // Constructor/Destructor
    ADSR_Linear(int atk_len, int dec_len, float sus_level, int rel_len);
    ~ADSR_Linear(void);

    // State parameter setters
    void set_attack_length(int length);
    void set_decay_length(int length);
    void set_sustain_level(float level);
    void set_release_length(int length);

    // Global parameter setters
    void set_period(float period);  // Set all lengths to period/3

    // Directly settable parameters 
    bool sustain;       // sustain   ? Decay  -> Sustain :  Decay -> Release
    bool retrigger;     // retrigger ? Release -> Attack  : Release  -> Idle

    // Main i/o
    void gate(bool is_high);
    float render();
    float get_output() { return x; }

protected:
  
    // Current parameters and values
    int state;        // State (A/D/S/R/I)
    int phase;        // Sample index in current state
    int length;       // State length
    float slope;      // Slope
    float x;          // Output level
  
    // State parameters and values
    int atk_len, dec_len, rel_len;          // Lengths
    float x_sus;                            // Sustain level
    
    // Math
    float compute_slope(float x0, float x1, int length);
  
    // State logic
    void begin_idle();
    void begin_attack();
    void begin_decay();
    void begin_sustain();
    void begin_release();
    void next_state();
};

inline float ADSR_Linear::compute_slope(float x0, float x1, int length) {
    return (x1 - x0) / (float)length;
}

inline void ADSR_Linear::begin_idle() {
    state = kADSRStateIdle;
    x = 0;
}

inline void ADSR_Linear::begin_attack() {
    state = kADSRStateAttack;
    slope = compute_slope(x, 1.0f, atk_len);
    length = atk_len;
    phase = 0;
}

inline void ADSR_Linear::begin_decay() {
    state = kADSRStateDecay;
    slope = compute_slope(x, x_sus, dec_len);
    length = dec_len;
    phase = 0;
}

inline void ADSR_Linear::begin_sustain() {
    state = kADSRStateSustain;
    x = x_sus;
}

inline void ADSR_Linear::begin_release() {
    state = kADSRStateRelease;
    slope = compute_slope(x, 0.0f, rel_len);
    length = rel_len;
    phase = 0;
}

inline void ADSR_Linear::next_state() {
    switch (state) {
        case kADSRStateAttack:
            begin_decay();
            break;
        case kADSRStateDecay:
            if (sustain)  begin_sustain();
            else          begin_release();
            break;
        case kADSRStateRelease:
            if (retrigger)  begin_attack();
            else            begin_idle();
            break;
        default:
            break;
    }
}

inline void ADSR_Linear::set_attack_length(int length) {
    atk_len = length;
    if (state == kADSRStateAttack) {
        length -= phase;
        length = length > 1 ? length : 1;
        slope = compute_slope(x, 1.0f, length);
        this->length = atk_len;
    }
}

inline void ADSR_Linear::set_decay_length(int length) {
    dec_len = length;
    if (state == kADSRStateDecay) {
        length -= phase;
        length = length > 1 ? length : 1;
        slope = compute_slope(x, x_sus, length);
        this->length = dec_len;
    }
}

inline void ADSR_Linear::set_sustain_level(float level) {
    x_sus = level;
    if (state == kADSRStateDecay) {
        int length = dec_len - phase;
        length = length > 1 ? length : 1;
        slope = compute_slope(x, x_sus, length);
    }
}

inline void ADSR_Linear::set_release_length(int length) {
    rel_len = length;
    if (state == kADSRStateRelease) {
        length -= phase;
        length = length > 1 ? length : 1;
        slope = compute_slope(x, 0.0f, length);
        this->length = rel_len;
    }
}

inline void ADSR_Linear::set_period(float period) {
    period /= 3.0f;
    set_attack_length(floorf(period));
    set_decay_length(roundf(period));
    set_release_length(ceilf(period));
}

inline void ADSR_Linear::gate(bool is_high) {
    if (is_high)  begin_attack();
    else          begin_release();
}

inline float ADSR_Linear::render() {
    if (state == kADSRStateIdle || state == kADSRStateSustain)
        return x;
    x += slope;
    if (++phase >= length)
        next_state();
    return x;
}

#endif
