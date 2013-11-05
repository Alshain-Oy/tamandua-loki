#define F_CPU 8000000UL		/* Clock Frequency = 8Mhz */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "Timer/timer.h"
#include "UART/uart.h"
#include "Modbus/modbus.h"

modbus_message_t msg;

int main(){

	uint8_t i = 0;
	uint8_t modbus_address = 65;
	uint8_t retval;
	uint8_t has_message = 0;
	
	/*tick_t*/ uint16_t t_last, t_current;
	
	uint8_t buffer[16];
	uint8_t bytes = 0;
	
	timer_init();
	uart_init();
	
	modbus_reset( &msg );
	
	DDRA = 0xFF;
	PORTA = 1;
	
	/*uart_listen();*/
	
	t_last = timer_get_fast();/*timer_get_time();*/
	while( 1 ){
		
		if( uart_has_data()  ){
			buffer[ bytes ] = uart_recv();
			
			++bytes;
			
			t_last = timer_get_fast();/*timer_get_time();*/
			/*uart_listen();*/
			
			}
		t_current = timer_get_fast();
		
		if( (t_current - t_last > 800) && (bytes > 0) ){
			
			
			/* 0xFF, 0x0F, 0xF0, 0x7f */
			PORTA &= ~0x04;
			
			if( buffer[0] == 0xFF ){
				PORTA |= 0x04;
				}
			
			_delay_us(100);
			PORTA &= ~0x04;
			_delay_us(100);
			
			if( buffer[1] == 0x0F ){
				PORTA |= 0x04;
				}
			
			_delay_us(100);
			PORTA &= ~0x04;
			_delay_us(100);
			
			if( buffer[2] == 0xF0 ){
				PORTA |= 0x04;
				}
			
			_delay_us(100);
			PORTA &= ~0x04;
			_delay_us(100);
			
			if( buffer[3] == 0x7F ){
				PORTA |= 0x04;
				}
			
			_delay_us(100);
			PORTA &= ~0x04;
			_delay_us(100);
			
			
			_delay_ms(0.3);
			
			buffer[0] = 0x41;
			buffer[1] = 0x42;
			buffer[2] = 0x43;
			buffer[3] = 0x44;
			buffer[4] = 0x45;
			
			
			/*PORTA |= 0x02;*/
			
			for( i = 0 ; i < bytes ; ++i ){
				uart_send( buffer[i] );
				
				PORTA |= 0x04;
				uart_wait();
				PORTA &= ~0x04;
				_delay_us(80);
				}
			_delay_us(300);
			
			/*uart_listen();*/
			
			
			/*PORTA &= ~(0x02);*/
			
			bytes = 0;
			}



		}
	
	
	
	
	return 0;
	}
