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

#ifndef _UART_H_
#define _UART_H_

#include <avr/io.h>  
#include <avr/interrupt.h>
#include <inttypes.h>


void uart_init(void);
void uart_listen(void);
uint16_t uart_recv(void);
void uart_send( uint8_t data );
void uart_wait( void );
uint8_t uart_is_done( void );

uint8_t uart_has_data( void );


#endif
