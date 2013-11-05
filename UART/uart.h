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
