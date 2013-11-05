#include "uart.h"

#define UART_IDLE      (0)
#define UART_RECV      (1)
#define UART_SEND      (1)


static volatile uint8_t uart_rx_buf = 0;
static volatile uint8_t uart_tx_buf = 0;
static volatile uint8_t uart_cnt = 0;
static volatile uint8_t uart_txrd = 0;
static volatile uint8_t uart_state = 0;
static volatile uint8_t uart_has_received = 0;
static volatile uint8_t uart_error = 0;

/*static uint8_t uart_bits[12];*/

static volatile uint8_t uart_recv_buffer[16];
static volatile uint8_t uart_buffer_write_pos = 0;
static volatile uint8_t uart_buffer_read_pos = 0;



/*volatile uint8_t uart_interbit = 105;*/ /* 9600 baud */
/*static uint8_t uart_interbit = 47;*/ /* 19200 baud */

const uint8_t uart_interbit = 54; /* 19200 baud, old 54 */

const uint8_t uart_prescaler = _BV( CS01 );
/*
ISR(TIM0_COMPA_vect, ISR_NOBLOCK)
{
	
  

 if( uart_txrd ){
	if( (uint8_t) (uart_state & UART_SEND) ){
	   
	   PORTB = (uint8_t) ((uart_buf & 0x01)) | (1<<2);
	   uart_buf >>= 1;
	   PORTA |= 0x02;
	   
	   --uart_cnt; 
	   if( uart_cnt == 0 ){
		 uart_state = UART_IDLE;
		 PORTB |= (uint8_t) (1<<0);
		 }
	 }
	 else {
		 uart_txrd = 0;
		 TIMSK0 &= (uint8_t) ~(_BV( OCIE0A ));
		 PORTB &= (uint8_t) ~( 1 << 2 );
		 }

   }
 else{
    if( (uint8_t) (uart_state & UART_RECV) ){
		PORTA &= ~0x04;
		PORTA |= 0x02;
		
		uart_buf <<= 1;
      uart_buf |= (uint8_t) (PINB >> 1) & (uint8_t) 0x01;
      --uart_cnt; 

		PORTA &= ~0x08;
		PORTA |= ((uart_buf & 0x01) << 3);
		

        if( uart_cnt == 0 ){
			uart_state = UART_IDLE;
			}      
        
    }
    else{    
		TIMSK0 &= (uint8_t) ~(_BV( OCIE0A ));
		uart_recv_buffer[ uart_buffer_write_pos ] = uart_buf;
		uart_buffer_write_pos = (uart_buffer_write_pos + 1) % 16;
		
		uart_has_received = 1;
		
		uart_listen();
      }
     }
  OCR0A = uart_interbit;
  PORTA &= ~0x02;
}
*/

#define UART_STATE_IDLE       0

#define UART_RECEIVE_FIRST_DATA    3
#define UART_RECEIVE_LAST_DATA     17
#define UART_RECEIVE_STOP_1        19
#define UART_RECEIVE_FINISH        23

#define UART_TRANSMIT_FIRST_DATA   2
#define UART_TRANSMIT_LAST_DATA    16
#define UART_TRANSMIT_STOP_1       18
#define UART_TRANSMIT_STOP_2       20
#define UART_TRANSMIT_FINISH       22

#define UART_ERROR_FRAME			1


#define UART_TRANSMIT_DELAY        70
#define UART_RECEIVE_DELAY         76    
#define UART_PRESCALER				8

#define UART_WAIT_ONE             53
#define UART_WAIT_ONEHALF         (UART_WAIT_ONE + UART_WAIT_ONE/2)

static volatile uint8_t debug = 0;

ISR(TIM0_COMPA_vect, ISR_NOBLOCK)
{
	uint8_t bit_in = 0x00;
	/* wait one */
	OCR0A = UART_WAIT_ONE;
	
	PORTA |= 0x01;
	
	if( (PINB & 0x02) ){
		bit_in = 0x01;
		PORTA |= 0x08;
		}
	else { PORTA &= ~0x08; }
	
	PORTA &= ~0x01;
	
	
	PORTA &= ~0x02;
	
	if( uart_cnt & 0x01 ){ /* odd if receive */
		
		if( uart_cnt <= UART_RECEIVE_LAST_DATA ){
			uart_rx_buf >>= 1;
			if( bit_in ){
				uart_rx_buf |= 0x80;
				}  
			}
		
		else if( uart_cnt == UART_RECEIVE_STOP_1 ){
			if( !bit_in ){
				uart_error = UART_ERROR_FRAME;
				}
			
			uart_recv_buffer[ uart_buffer_write_pos ] = uart_rx_buf;
			uart_buffer_write_pos = ( uart_buffer_write_pos + 1 ) % 16;
			uart_has_received = 1;
			
			/* clear interrupt */
			GIFR &= ~_BV(PCIF1);
			
			/* start pin change interrupt */
			GIMSK |= _BV(PCIE1);
			
			
			}
		
		else if( uart_cnt == UART_RECEIVE_FINISH ){
			
			if( uart_txrd == 1 ){
				
				uart_txrd = 0;
				
				/* disable pin change interrupt */
				GIMSK &= (uint8_t) ~(_BV( PCIE1 ));
				
				 
				/* start bit */
				PORTB &= ~1; 
				
				/* bus write signal */
				PORTB |= 0x04;
				
				/* first bit */
				
				uart_cnt = UART_TRANSMIT_FIRST_DATA;
				
				/* stop uart timer */
				TCCR0B = ~uart_prescaler;
				
				/* clear uart timer */
				TCNT0 = 0;
				
				/* set timer to start transmit */
				OCR0A = UART_WAIT_ONE - (UART_TRANSMIT_DELAY/UART_PRESCALER);
				
				/* clear timer interrupt flag */
				TIFR0 = _BV(OCF0A);
				
				/* Start timer */
				TCCR0B = uart_prescaler;
				
				return;
				}
			
			else {
				/* stop uart timer */
				TCCR0B = ~uart_prescaler;
				
				
				/* disable uart timer interrupt */
				TIMSK0 &= ~_BV(OCIE0A);
				
				uart_cnt = UART_STATE_IDLE;
				
				return;
				}
			
			}
		
		}
		/* Transmit */
	else {
		uint8_t bit_out = 0x00;
		debug = 1;
		
		
		if( uart_cnt != UART_TRANSMIT_FIRST_DATA ){
			
			if( uart_cnt <= UART_TRANSMIT_STOP_1 ){
				uart_tx_buf >>= 1;
				}
			
			}
		
		if( uart_cnt <= UART_TRANSMIT_LAST_DATA ){
			
			if( uart_tx_buf & 0x01 ){
				bit_out = 0x01;
				}
			
			}
		else { 
			if( uart_cnt == UART_TRANSMIT_STOP_1 ){
			
				bit_out = 0x01;
				
				}
			
			else { 
				if( uart_cnt == UART_TRANSMIT_STOP_2 ){
					
					bit_out = 0x01;	
					
					}
				
				else {
					/* stop uart timer */
					TCCR0B = ~uart_prescaler;
					
					/* disable uart interrupt */
					TIMSK0 &= ~_BV(OCIE0A);
					
					uart_cnt = UART_STATE_IDLE;
					
					/* bus read signal */
					PORTB &= ~0x04;
					
					/* clear interrupt */
					GIFR &= ~_BV(PCIF1);
					
					/* enable pin change interrupt */
					GIMSK |= (_BV( PCIE1 ));

					
					uart_txrd = 0;
					
					}
				}
			}
		
		if( bit_out ){
			PORTB |= 0x01;
			}
		else {
			PORTB &= ~0x01;
			}
		
		
		}

	uart_cnt += 2;
}

ISR(PCINT1_vect)
{
	if( PINB & 0x02 ){ return; }
	
	PORTA |= 0x02;
	PORTA &= ~0x08;
	
	/*PORTA |= 0x04;*/
	/*
	GIMSK &= (uint8_t) ~(_BV( PCIE1 ));
	TCNT0 = 35;
	TIMSK0 |= (_BV( OCIE0A ));
	*/
	uart_cnt = UART_RECEIVE_FIRST_DATA;
	uart_rx_buf = 0x00;
	
	/* stop uart timer */
	TCCR0B = ~uart_prescaler;
	
	/* clear uart timer */
	TCNT0 = 0;
	
	/* set timer to receive first bit */
	OCR0A = UART_WAIT_ONEHALF - (UART_RECEIVE_DELAY/UART_PRESCALER);
	
	/* disable pin change interrupt */
	GIMSK &= (uint8_t) ~(_BV( PCIE1 ));
	
	/* clear timer interrupt flag */
	TIFR0 = _BV(OCF0A);
	
	/* enable timer interrupt */
	TIMSK0 |= _BV(OCIE0A);
	
	/* start uart timer */
	TCCR0B = uart_prescaler;
	
  
  
  
  
}




void uart_init( void ){
	cli();

	
	TCCR0A |= _BV( WGM01 ); /* CTC */
	TCCR0B = ~uart_prescaler;
	TCNT0 = 0;
	OCR0A = uart_interbit;
	
	TIMSK0 &= (uint8_t) ~(_BV( OCIE0A )); /* turn off */
	
	GIMSK &= (uint8_t) ~(_BV( PCIE1 ));
	PORTA &= ~0x04;
	
	PCMSK1 |= _BV( PCINT9 );
	sei();
	
	DDRB = (1 << 0) | (1 << 2);
	PORTB |= (1<<0);  
	
	uart_cnt = UART_STATE_IDLE;
	
	
				
	/* clear interrupt */
	GIFR &= ~_BV(PCIF1);
	
	/* enable pin change interrupt */
	GIMSK |= (_BV( PCIE1 ));
			
	
	}

/*
void uart_listen( void ){
	uint8_t recv_pin = (uint8_t) ( 1 << 2);
	
	while( uart_txrd != 0 ){}
	
	uart_buf = 0;
	uart_state = UART_RECV;
	
	
	
	uart_cnt = 11;
	
	GIMSK |= (uint8_t) _BV( PCIE1 );
	PORTB &= (uint8_t) ~recv_pin;

	

	
	uart_txrd = 0;
	PORTA &= (uint8_t) ~(0x01);
	}
*/

uint16_t uart_recv(void){
	uint8_t dummy = 0;
	uint8_t out;

	while( (uart_has_received == 0) && (uart_buffer_read_pos == uart_buffer_write_pos) ){
		dummy += 1;
		}
	PORTA |= (uint8_t) 0x01;

	uart_has_received = 0;
	
	/*
	uart_buf = 0;
	uart_buf |= uart_bits[ 3 ];
	uart_buf <<= 1;
	uart_buf |= uart_bits[ 4 ];
	uart_buf <<= 1;
	uart_buf |= uart_bits[ 5 ];
	uart_buf <<= 1;
	uart_buf |= uart_bits[ 6 ];
	uart_buf <<= 1;
	uart_buf |= uart_bits[ 7 ];
	uart_buf <<= 1;
	uart_buf |= uart_bits[ 8 ];
	uart_buf <<= 1;
	uart_buf |= uart_bits[ 9 ];
	uart_buf <<= 1;
	uart_buf |= uart_bits[ 10 ];
	*/
	
	/*out = (uart_recv_buffer[ uart_buffer_read_pos ] >> 1) & 0xFF;*/
	out = uart_recv_buffer[ uart_buffer_read_pos ];
	
	uart_buffer_read_pos = (uart_buffer_read_pos + 1) % 16;
	
	/*return ~out;*/
	
	return out;
	}
	


	
void uart_send( uint8_t data ){
/*
	uint8_t parity = (0x6996 >> ((data ^ ( data >> 4 ) ) & 0x0F ) ) & 0x01; 

	uint8_t recv_pin = (uint8_t) ( 1 << 2);
	uint16_t mask = 0x80;
	
	uart_buf = 0;
	uart_state = UART_SEND;
	
	GIMSK &= (uint8_t) ~(_BV( PCIE1 ));
	
	uart_buf |= 1;
	
	uart_buf |= parity | 1;
	uart_buf <<= 1;
	
	
	uart_buf <<= 8;
	
	uart_buf |= data;
	
	uart_buf <<= 1;
	
	
	
	uart_txrd = 1;
	uart_cnt = 11;
	
	
	TCNT0 = 0;
	OCR0A = (uart_interbit) >> 1;
	TIMSK0 |= (_BV( OCIE0A )); 
	PORTB = (1<<2);
	*/
	
	/* stop uart timer */
	TCCR0B = ~uart_prescaler;
	
	
	uart_tx_buf = data;
	uart_txrd = 1;
	
	/* disable pin change interrupt */
	GIMSK &= (uint8_t) ~(_BV( PCIE1 ));

	/* start bit */
	PORTB &= ~1; 
				
	/* bus write signal */
	PORTB |= 0x04;
				
	/* first bit */			
	uart_cnt = UART_TRANSMIT_FIRST_DATA;
	
	/* clear uart timer */
	TCNT0 = 0;
	
	/* set timer to start transmit */
	OCR0A = UART_WAIT_ONE - (UART_TRANSMIT_DELAY/UART_PRESCALER);
				
	
	/* clear timer interrupt flag */
	TIFR0 = _BV(OCF0A);
	
	/* enable timer interrupt */
	TIMSK0 |= _BV(OCIE0A);
	
				
	/* Start timer */
	TCCR0B = uart_prescaler;
	
	debug = 2;
	
	}


void uart_wait( void ){
	/*while( (uart_state != UART_IDLE) || (uart_txrd != 0 ) ){}*/
	/*while( (uart_cnt != UART_STATE_IDLE) || (uart_txrd != 0 ) ){*/
	while( (uart_cnt != UART_STATE_IDLE) && (uart_tx_buf != 0) ){}
	
	}

uint8_t uart_is_done( void ) {
	return uart_state == UART_IDLE;
	}


uint8_t uart_has_data( void ) {
	return uart_has_received || (uart_buffer_read_pos != uart_buffer_write_pos);
	}
