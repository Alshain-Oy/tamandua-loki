#include "timer.h"


volatile uint16_t timer_overflows = 0;

ISR(TIM1_OVF_vect)
{
	timer_overflows += 1;
	}



void timer_init( void  ){
	cli();

	TCCR1A = 0 ; /* Overflow */
	TCCR1B = _BV( CS11 ); /* divider: 8 -> 1MHz */
	TCNT1 = 0;
	
	TIMSK1 |= _BV( TOIE1 ); /* turn on */
	
	sei();
	
	}

uint16_t timer_get_slow( void ){
	return timer_overflows;
	}

uint16_t timer_get_fast( void ){
	return TCNT1;
	}


uint16_t timer_get_difference( uint16_t t0, uint16_t t1 ){
	if( t1 < t0 ){
		return t1 + (uint16_t)((uint16_t)(65535) - t0);
		}
	return t1 - t0;
	}


tick_t timer_get_time(void) {
	
	tick_t out;
	
	out.high = timer_get_slow();
	out.low = timer_get_fast();
	
	return out;
	}

uint16_t timer_compute_delta( tick_t *t0, tick_t *t1 ){
	
	if( t1->high > t0->high + 1 ){
		return 0xffff;
		}
	else {
		if( t1->high > t0->high ){
			if( t1->low > t0->low ){
				return 0xffff;
				}
			else {
				return (0xffff - t0->low) - t1->low;
				}
			}
		else {
			return t1->low - t0->low;
			}
		}
	
	}
