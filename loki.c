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
	uint8_t modbus_address = 0x17;
	uint8_t retval;
	uint8_t has_message = 0;
	uint8_t recvd_byte;
	
	/*tick_t*/ uint16_t t_last, t_current;
	
	/*uint8_t buffer[16];*/
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
			PORTA |= 0x10;
			recvd_byte = uart_recv();
			modbus_append_byte( &msg, recvd_byte );
			
			PORTA &= ~0x10;
			++bytes;
			
			t_last = timer_get_fast();/*timer_get_time();*/
			/*uart_listen();*/
			
			}
		t_current = timer_get_fast();
		
		if( (t_current - t_last > 2100) && (bytes > 0) ){
			
			retval = modbus_handle_message( &msg, modbus_address );
			
			if( retval == MODBUS_OK ){
				for( i = 0 ; i < msg.length ; ++i ){
					uart_send( msg.buffer[i] );
					
					PORTA |= 0x04;
					uart_wait();
					PORTA &= ~0x04;
					_delay_us(80);
					}
				
				}
			else {
				uart_send( 0xDE );
				PORTA |= 0x04;
				uart_wait();
				PORTA &= ~0x04;
				_delay_us(80);
				
				uart_send( 0xAD );
				PORTA |= 0x04;
				uart_wait();
				PORTA &= ~0x04;
				_delay_us(80);
				
				}
				
			_delay_us(300);
			bytes = 0;
			
			modbus_reset( &msg );
			
			}



		}
	
	
	
	
	return 0;
	}
