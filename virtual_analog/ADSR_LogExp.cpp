//  ADSR.cpp
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

#include "ADSR_LogExp.h"

ADSR_LogExp::ADSR_LogExp(int atk_len, int dec_len, float sus_level, int rel_len, float eps) {
    v_sus = sus_level;
    begin_idle();
    set_attack(eps, atk_len);
    set_decay(eps, dec_len);
    set_release(eps, rel_len);
    sustain = false;
    retrigger = false;
}

ADSR_LogExp::~ADSR_LogExp(void) {
  
}
