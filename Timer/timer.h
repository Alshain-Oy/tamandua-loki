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
 
