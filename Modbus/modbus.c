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

#include "modbus.h"

uint16_t modbus_global_event_counter = 0;


uint16_t modbus_compute_crc( modbus_message_t *msg ){
	uint16_t i, j;
	uint16_t out = 0xFFFF;
	for( i = 0 ; i < msg->length - 2 ; ++i ){
		out ^= (uint16_t)msg->buffer[i];
		for( j = 0 ; j < 8 ; ++j ){
			if( (out & 0x1) != 0 ){
				out >>= 1;
				out ^= 0xA001;
				}
			else {
				out >>= 1;
				}
			}
		
		}
	
	return out;
	}


void modbus_reset( modbus_message_t *msg ){
	uint16_t i;
	for( i = 0 ; i < MAX_LENGTH ; ++i ){
		msg->buffer[0] = 0;
		}
	msg->length = 0;
	msg->target_length = 0;
	}


void modbus_append_byte( modbus_message_t *msg, uint8_t byte ){
	if( msg->length < MAX_LENGTH ){
		msg->buffer[ msg->length] = byte;
		++msg->length;
		}
	}

void modbus_append_word( modbus_message_t *msg, uint16_t word ){
	if( msg->length < MAX_LENGTH - 2 ){
		msg->buffer[ msg->length] = (word >> 8);
		++msg->length;
		msg->buffer[ msg->length] = word & 0xFF;
		++msg->length;
		}
	}

void modbus_append_crc( modbus_message_t *msg ){
	uint16_t crc;
	
	msg->length += 2;
	crc = modbus_compute_crc( msg );
	msg->buffer[msg->length - 2] = crc >> 8;
	msg->buffer[msg->length - 1] = crc & 0xFF;
	}

uint16_t modbus_extract_word( modbus_message_t *msg, uint8_t pos ){
	uint16_t out = 0;
	
	out = msg->buffer[pos] << 8;
	out |= msg->buffer[pos + 1];
	
	return out;
	}

uint8_t modbus_validate_function( modbus_message_t *msg ){
	if( msg->length < 2 ){ return MODBUS_INVALID; }
	
	switch( msg->buffer[1] ){
		case READ_COILS:
		case READ_HOLDING_REGISTERS:
		case READ_INPUT_REGISTERS:
		case WRITE_SINGLE_COIL:
		case WRITE_SINGLE_REGISTER:
		case READ_EXCEPTION_STATUS:
		case DIAGNOSTICS:
		case GET_COMM_COUNTER:
		case GET_COMM_EVENT_LOG:
		case WRITE_MULTIPLE_COILS:
		case WRITE_MULTIPLE_REGISTERS:
		case REPORT_SERVER_ID:
		case WRITE_MASK_REGISTER:
			return MODBUS_OK;
		break;
		default:
			return MODBUS_INVALID;
		break;
		}
		
	return MODBUS_INVALID;
	}



uint8_t modbus_get_target_length( modbus_message_t *msg ){
	uint8_t len = 0;
	if( msg->length > 3 ){
		switch( msg->buffer[1] ){
			case READ_COILS:
				/*count = (msg->buffer[4] << 8) | msg->buffer[5];*/
				len = 8; /* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + COUNT(2) + CRC(2) */
			break;
			
			case READ_HOLDING_REGISTERS:
				len = 8; /* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + COUNT(2) + CRC(2) */
			break;
			
			case READ_INPUT_REGISTERS:
				len = 8; /* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + COUNT(2) + CRC(2) */
			break;
			case WRITE_SINGLE_COIL:
				len = 8; /* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + COUNT(2) + CRC(2) */
			break;
			case WRITE_SINGLE_REGISTER:
				len = 8; /* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + COUNT(2) + CRC(2) */
			break;
			
			case READ_EXCEPTION_STATUS:
				len = 4;/* SLAVEADDR(1) + FUNCTION(1) + CRC(2) */
			break;
			case DIAGNOSTICS:
				len = 8; /* SLAVEADDR(1) + FUNCTION(1) + SUBFUNC(2) + DATA(2) + CRC(2) */
			break;
			
			case GET_COMM_COUNTER:
				len = 4;/* SLAVEADDR(1) + FUNCTION(1) + CRC(2) */
			break;
			
			case GET_COMM_EVENT_LOG:
				len = 4;/* SLAVEADDR(1) + FUNCTION(1) + CRC(2) */
			break; 
			
			case WRITE_MULTIPLE_COILS:
				if( msg->length > 5 ){
					len = 8 /* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + COUNT(2) + CRC(2) */ + msg->buffer[5]; /* byte count */
					}
			break;
			
			case WRITE_MULTIPLE_REGISTERS:
				if( msg->length > 5 ){
					len = 8 /* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + COUNT(2) + CRC(2) */ + 2*msg->buffer[5]; /* byte count */
					}
			break;
			
			case REPORT_SERVER_ID:
				len = 4;/* SLAVEADDR(1) + FUNCTION(1) + CRC(2) */
			break;

			case WRITE_MASK_REGISTER:
				len = 10; /* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + AND_MASK(2) + OR_MASK(2) + CRC(2) */
			break;

			default:
				len = 0;
			break;
			}
		}
	
	msg->target_length = len;
	
	return len;
	}

uint8_t modbus_inspect( modbus_message_t *msg ){
	uint16_t crc;
	modbus_get_target_length( msg );
	if( (msg->length > 0) && (msg->target_length > 0) ){
		if( msg->length == msg->target_length ){
			crc = (msg->buffer[msg->length-2] << 8) | msg->buffer[msg->length - 1];
			if( crc != modbus_compute_crc( msg ) ){
				return MODBUS_INVALID;
				}
			return modbus_validate_function( msg );
			}
		}
	if( msg->length < 4 ){
		return MODBUS_INVALID;
		}
	if( (msg->length > 8) && (msg->target_length < 1) ){
		return MODBUS_INVALID;
		}
	
	return MODBUS_INVALID;
	}


void modbus_create_exception_response( modbus_message_t *msg, uint8_t func, uint8_t exception ){
	modbus_reset( msg );
	modbus_append_byte( msg, msg->slave_address );
	modbus_append_byte( msg, func | 0x80 );
	modbus_append_byte( msg, exception );
	modbus_append_crc( msg );
	}


uint8_t modbus_handle_read_coils( modbus_message_t *msg, uint16_t address, uint16_t count ){
	uint8_t coils[32], i, bit, length;
	
	modbus_reset( msg );
	modbus_append_byte( msg, msg->slave_address );
	modbus_append_byte( msg, READ_COILS );
	
	if( address + count > 14 ){
		return ILLEGAL_DATA_ADDRESS;
		}
	
	for( i = 0 ; i < 32 ; ++i ){
		coils[i] = 0;
		}
	
	
	
	for( i = 0 ; i < count ; ++i ){
		bit = i & 0x7;
		/*coils[ i >> 3 ] |= (digitalRead( address + count ) & 0x01) << bit;*/
		coils[i >> 3] = 1 << bit;
		}
	
	length = count >> 3;
	if( (count & 7) > 0 ){ length += 1 ; }
	
	modbus_append_byte( msg, length );
	
	for( i = 0 ; i < length ; ++i ){
		modbus_append_byte(msg, coils[i] );
		}
	
	
	modbus_append_crc( msg );
	
	++modbus_global_event_counter;
	
	return MODBUS_OK;
	}

uint8_t modbus_handle_read_discrete_inputs( modbus_message_t *msg, uint16_t address, uint16_t count ){
	uint8_t coils[32], i, bit, length;
	
	modbus_reset( msg );
	modbus_append_byte( msg, msg->slave_address );
	modbus_append_byte( msg, READ_DISCRETE_INPUTS );
	
	if( address + count > 14 ){
		return ILLEGAL_DATA_ADDRESS;
		}
	
	for( i = 0 ; i < 32 ; ++i ){
		coils[i] = 0;
		}
	
	
	
	for( i = 0 ; i < count ; ++i ){
		bit = i & 0x7;
		/*coils[ i >> 3 ] |= (digitalRead( address + count ) & 0x01) << bit;*/
		coils[ i >> 3 ] = 1 << bit;
		}
	
	length = count >> 3;
	if( (count & 7) > 0 ){ length += 1 ; }
	
	modbus_append_byte( msg, length );
	
	for( i = 0 ; i < length ; ++i ){
		modbus_append_byte(msg, coils[i] );
		}
	
	
	modbus_append_crc( msg );
	
	
	++modbus_global_event_counter;
	
	return MODBUS_OK;
	}

uint8_t modbus_handle_read_holding_registers( modbus_message_t *msg, uint16_t address, uint16_t count ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_read_input_registers( modbus_message_t *msg, uint16_t address, uint16_t count ){
	uint16_t inputs[10], i;
	
	modbus_reset( msg );
	modbus_append_byte( msg, msg->slave_address );
	modbus_append_byte( msg, READ_INPUT_REGISTERS );
	
	if( address + count > 6 ){
		return ILLEGAL_DATA_ADDRESS;
		}
	
	for( i = 0 ; i < count ; ++i ){
		/*printf( "debug: analogRead(%i + %i) = %i\n", address, i, analogRead(address+i) );*/
		/*inputs[i] = analogRead( address + i );*/
		inputs[i] = address + i;
		}
	
	modbus_append_byte( msg, count * 2 );
	
	for( i = 0 ; i < count ; ++i ){
		modbus_append_word( msg, inputs[i] );
		}
	
	
	modbus_append_crc( msg );
	
	++modbus_global_event_counter;
	
	return MODBUS_OK;
	
	}

uint8_t modbus_handle_write_single_coil( modbus_message_t *msg, uint16_t address, uint16_t value ){
	
	if( address > 7 ){
		return ILLEGAL_DATA_ADDRESS;
		}
	
	/*
	if( value > 0 ){
		digitalWrite( address, HIGH );
		}
	else{
		digitalWrite( address, LOW );
		}
	*/
	
	if( value ){
		PORTA |= 1 << address;
		}
	else {
		PORTA &= ~(1 << address);
		}
		
	modbus_reset( msg );
	modbus_append_byte( msg, msg->slave_address );
	modbus_append_byte( msg, WRITE_SINGLE_COIL );
	modbus_append_word( msg, value );
	modbus_append_crc( msg );
	
	++modbus_global_event_counter;
	
	return MODBUS_OK;
	}

uint8_t modbus_handle_write_single_register( modbus_message_t *msg, uint16_t address, uint16_t value ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_read_exception_status( modbus_message_t *msg ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_diagnostics( modbus_message_t *msg, uint16_t subfunction ){
	
	if( subfunction == 0x00 ){ return MODBUS_OK; }
	
	
	
	
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_get_comm_event_log( modbus_message_t *msg ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_get_comm_event_counter( modbus_message_t *msg ){
	
	modbus_reset( msg );
	modbus_append_byte( msg, msg->slave_address );
	modbus_append_byte( msg, GET_COMM_COUNTER );
	modbus_append_word( msg, 0x0000 );
	modbus_append_word( msg, modbus_global_event_counter );
	modbus_append_crc( msg );
	
	
	return MODBUS_OK;
	}

uint8_t modbus_handle_write_multiple_coils( modbus_message_t *msg, uint16_t address, uint16_t count, uint8_t *values ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_write_multiple_registers( modbus_message_t *msg, uint16_t address, uint16_t count, uint16_t *values ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_report_server_id( modbus_message_t *msg ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_write_mask_register( modbus_message_t *msg, uint16_t address, uint16_t and_mask, uint16_t or_mask ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_message( modbus_message_t *msg, uint8_t own_address ){
	uint8_t func = msg->buffer[1];
	uint8_t response, message_address;
	
	response = MODBUS_INVALID;
	
	message_address = msg->buffer[0];
	if(  (message_address != own_address) && ( message_address != 0 ) ){ return MODBUS_INVALID; }
	
	msg->slave_address = own_address;
	
	if( modbus_inspect( msg ) != MODBUS_OK ){
		modbus_create_exception_response( msg, msg->buffer[1], ILLEGAL_FUNCTION );
		return MODBUS_OK;
		}
	
	switch( func ){
		case READ_COILS:
			{
			uint16_t address = modbus_extract_word( msg, ADDRESS_OFFSET );
			uint16_t count = modbus_extract_word( msg, COUNT_OFFSET );
			response = modbus_handle_read_coils( msg, address, count );
			}
		break;
		case READ_HOLDING_REGISTERS:
			{
			uint16_t address = modbus_extract_word( msg, ADDRESS_OFFSET );
			uint16_t count = modbus_extract_word( msg, COUNT_OFFSET );
			response = modbus_handle_read_holding_registers( msg, address, count );
			}
		break;
		case READ_INPUT_REGISTERS:
			{
			uint16_t address = modbus_extract_word( msg, ADDRESS_OFFSET );
			uint16_t count = modbus_extract_word( msg, COUNT_OFFSET );
			response = modbus_handle_read_input_registers( msg, address, count );
			}

		break;
		case WRITE_SINGLE_COIL:
			{
			uint16_t address = modbus_extract_word( msg, ADDRESS_OFFSET );
			uint16_t value = modbus_extract_word( msg, VALUE_OFFSET );
			response = modbus_handle_write_single_coil( msg, address, value );
			}
		break;
		case WRITE_SINGLE_REGISTER:
			{
			uint16_t address = modbus_extract_word( msg, ADDRESS_OFFSET );
			uint16_t value = modbus_extract_word( msg, VALUE_OFFSET );
			response = modbus_handle_write_single_register( msg, address, value );
			}
		break;
		case READ_EXCEPTION_STATUS:
			response = modbus_handle_read_exception_status( msg );
		break;
		case DIAGNOSTICS:
			{
			uint16_t subfunc = modbus_extract_word( msg, SUBFUNC_OFFSET );
			response = modbus_handle_diagnostics( msg, subfunc );
			}
		break;
		case GET_COMM_COUNTER:
			response = modbus_handle_get_comm_event_counter( msg );
		break;
		case GET_COMM_EVENT_LOG:
			response = modbus_handle_get_comm_event_log( msg );
		break;
		case WRITE_MULTIPLE_COILS:
			{
			uint16_t address = modbus_extract_word( msg, ADDRESS_OFFSET );
			uint16_t count = modbus_extract_word( msg, COUNT_OFFSET );
			uint8_t coils[32], i;
			for( i = 0 ; i < count ; ++i ){
				coils[i] = msg->buffer[VALUE_OFFSET + i];
				}
			response = modbus_handle_write_multiple_coils( msg, address, count, coils );
			}
		break;
		case WRITE_MULTIPLE_REGISTERS:
			{
			uint16_t address = modbus_extract_word( msg, ADDRESS_OFFSET );
			uint16_t count = modbus_extract_word( msg, COUNT_OFFSET );
			uint16_t registers[32];
			uint8_t i = 0;
			for( i = 0 ; i < count ; ++i ){
				registers[i] = modbus_extract_word( msg, VALUE_OFFSET + i*2 );
				}
			response = modbus_handle_write_multiple_registers( msg, address, count, registers );
			}
		break;
		case REPORT_SERVER_ID:
			response = modbus_handle_get_comm_event_log( msg );
		break;
		case WRITE_MASK_REGISTER:
			{
			uint16_t address = modbus_extract_word( msg, ADDRESS_OFFSET );
			uint16_t and_mask = modbus_extract_word( msg, AND_MASK_OFFSET );
			uint16_t or_mask = modbus_extract_word( msg, OR_MASK_OFFSET );
			response = modbus_handle_write_mask_register( msg, address, and_mask, or_mask );
			}
		break;
		default:
			modbus_create_exception_response( msg, msg->buffer[1], ILLEGAL_FUNCTION );
		break;
		
		}
	
	if( response != MODBUS_OK ){
		modbus_create_exception_response( msg, func, response );
		}
	
	if( message_address == 0 ){ return MODBUS_INVALID; }
	
	return MODBUS_OK;
	}



#ifdef DEBUG
void debug_print_msg( modbus_message_t *msg ){
	uint8_t i;
	for( i = 0 ; i < msg->length ; ++i ){
		printf( "%02x ", msg->buffer[i] );
		}
	printf( "\n" );
	}

#endif

/*
int main( int argc, char **argv ){
	
	modbus_message_t msg;
	
	modbus_reset( &msg );
	
	modbus_append_byte( &msg, 0x17 );
	modbus_append_byte( &msg, READ_INPUT_REGISTERS );
	modbus_append_word( &msg, 3 );
	modbus_append_word( &msg, 2 );
	modbus_append_crc( &msg );
	
	debug_print_msg( &msg );
	
	printf( "modbus_inspect: %i\n", modbus_inspect( &msg ) );
	
	modbus_handle_message( &msg, 0x17 );
	
	debug_print_msg( &msg );
	
	
	return 0;
	}
*/
