//  ADSR.h
//  Virtual analog envelope generator with variable shape
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

#ifndef ADSR_LogExp_h
#define ADSR_LogExp_h

#include <math.h>

class ADSR_LogExp {

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
    ADSR_LogExp(int atk_len, int dec_len, float sus_level, int rel_len, float eps);
    ~ADSR_LogExp(void);

    // State parameter setters
    void set_attack(float eps, int length);
    void set_decay(float eps, int length);
    void set_sustain(float level);
    void set_release(float eps, int length);

    // Global parameter setters
    void set_period(float period);  // Set all lengths to period/3
    void set_shape(float eps);      // Set all shapes

    // Directly settable parameters 
    bool sustain;       // Enter Sustain after Decay, else Release
    bool retrigger;     // Reenter Attack after Release, else Idle

    // sustain   ? Decay  -> Sustain :  Decay -> Release
  
    // Main i/o
    void gate(bool is_high);
    float render();
    float get_output() { return v_c; }

    // State parameter getters
    int attack_length()   { return atk_len; }
    int decay_length()    { return dec_len; }
    int release_length()  { return rel_len; }
    float attack_shape()  { return atk_eps; }
    float decay_shape()   { return dec_eps; }
    float release_shape() { return rel_eps; }
    
protected:
  
    // Current parameters and values
    
    int phase;        // Sample index in current state
    int length;       // State length
    float coeff;      // Coefficient
    float v_exp;      // Decaying exponential (v_exp[n] = v_exp[n-1] * coeff)
    float v_0, v_T;   // Initial and target levels
    float v_c;        // Output ( v_T * (1 - v_exp) + v_0 * v_exp)

    // State in {Attack, Decay, Sustain, Release, Idle}
    int state;        
  
    // State parameters and values
    int atk_len, dec_len, rel_len;          // Lengths
    float atk_coeff, dec_coeff, rel_coeff;  // Coefficients
    float atk_eps, dec_eps, rel_eps;        // Shapes
    float v_sus;                            // Sustain level
    
    // Math
    float compute_coeff(float eps, int length);
    void init_ramp(float v0, float v1, float eps, float coeff, int length);
  
    // State logic
    void begin_idle();
    void begin_attack();
    void begin_decay();
    void begin_sustain();
    void begin_release();
    void next_state();
};


inline float ADSR_LogExp::compute_coeff(float eps, int length) {
//    return expf(logf(eps / (1 + eps)) / (float)length);
    return powf(eps/(1 + eps), 1/(float)length);
}

inline void ADSR_LogExp::init_ramp(float v0, float v1, float eps, float coeff, int length) {
    v_0 = v0;
    v_T = v1 + (v1 - v0) * eps;
    this->length = length;
    this->coeff = coeff;
    v_exp = 1;   
}

inline void ADSR_LogExp::begin_idle() {
    state = kADSRStateIdle;
    v_c = 0.0f;
}

inline void ADSR_LogExp::begin_attack() {
    state = kADSRStateAttack;
    init_ramp(v_c, 1.0f, atk_eps, atk_coeff, atk_len);
    phase = 0;
}

inline void ADSR_LogExp::begin_decay() {
    state = kADSRStateDecay;
    init_ramp(v_c, v_sus, dec_eps, dec_coeff, dec_len);
    phase = 0;
}

inline void ADSR_LogExp::begin_sustain() {
    state = kADSRStateSustain;
    v_c = v_sus;
}

inline void ADSR_LogExp::begin_release() {
    state = kADSRStateRelease;
    init_ramp(v_c, 0.0f, rel_eps, rel_coeff, rel_len);
    phase = 0;
}

inline void ADSR_LogExp::next_state() {
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

inline void ADSR_LogExp::set_attack(float eps, int length) {
    atk_len = length;
    atk_eps = eps;
    atk_coeff = compute_coeff(eps, length);
    if (state == kADSRStateAttack) {
        length -= phase;
        length = length > 1 ? length : 1;
        init_ramp(v_c, 1.0f, eps, compute_coeff(eps, length), atk_len);
    }
}

inline void ADSR_LogExp::set_decay(float eps, int length) {
    dec_len = length;
    dec_eps = eps;
    dec_coeff = compute_coeff(eps, length);
    if (state == kADSRStateDecay) {
        length -= phase;
        length = length > 1 ? length : 1;
        init_ramp(v_c, v_sus, eps, compute_coeff(eps, length), dec_len);
    }
}

inline void ADSR_LogExp::set_sustain(float level) {
    v_sus = level;
    if (state == kADSRStateDecay) {
        int length = dec_len - phase;
        length = length > 1 ? length : 1;
        init_ramp(v_c, v_sus, dec_eps, compute_coeff(dec_eps, length), dec_len);
    }
}

inline void ADSR_LogExp::set_release(float eps, int length) {
    rel_len = length;
    rel_eps = eps;
    rel_coeff = compute_coeff(eps, length);
    if (state == kADSRStateRelease) {
        length -= phase;
        length = length > 1 ? length : 1;
        init_ramp(v_c, 0.0f, eps, compute_coeff(eps, length), rel_len);
    }
}

inline void ADSR_LogExp::set_period(float period) {
    period /= 3.0f;
    set_attack(atk_eps, floorf(period));
    set_decay(dec_eps, roundf(period));
    set_release(rel_eps, ceilf(period));
}

inline void ADSR_LogExp::set_shape(float eps) {
    set_attack(eps, atk_len);
    set_decay(eps, dec_len);
    set_release(eps, rel_len);
}

inline void ADSR_LogExp::gate(bool is_high) {
    if (is_high)  begin_attack();
    else          begin_release();
}

inline float ADSR_LogExp::render() {
    if (state == kADSRStateIdle || state == kADSRStateSustain)
        return v_c;
    v_exp *= coeff;
    v_c = v_T * (1 - v_exp) + v_0 * v_exp;
    if (++phase >= length)
        next_state();
    return v_c;
}

#endif
