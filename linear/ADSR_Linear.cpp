//  ADSR_Linear.cpp
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

#include "ADSR_Linear.h"

ADSR_Linear::ADSR_Linear(int atk_len, int dec_len, float sus_level, int rel_len) {
    x_sus = sus_level;
    begin_idle();
    set_attack_length(atk_len);
    set_decay_length(dec_len);
    set_release_length(rel_len);
    sustain = false;
    retrigger = false;
}

ADSR_Linear::~ADSR_Linear(void) {
  
}
