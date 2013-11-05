/*

   Copyright 2013 Alshain Oy

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.



*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include <avr/io.h>  
#include <avr/interrupt.h>
#include <inttypes.h>

typedef struct {
	uint16_t high, low;
	} tick_t;

void timer_init(void);

uint16_t timer_get_slow(void);
uint16_t timer_get_fast(void);


uint16_t timer_get_difference( uint16_t t0, uint16_t t1 );


tick_t timer_get_time(void);

uint16_t timer_compute_delta( tick_t *t0, tick_t *t1 );



#endif
 
