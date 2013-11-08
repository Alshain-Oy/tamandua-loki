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


uint16_t modbus_global_event_counter = 0;


uint8_t modbus_handle_read_coils( 	modbus_message_t *msg, 
									uint16_t address, 
									uint16_t count ){
										
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
		coils[ (uint8_t)(i >> 3) ] = (uint8_t)( 1 << bit);
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

uint8_t modbus_handle_read_discrete_inputs( modbus_message_t *msg, 
											uint16_t address, 
											uint16_t count ){
												
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
		coils[ (uint8_t)(i >> 3) ] = (uint8_t)(1 << bit);
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

uint8_t modbus_handle_read_holding_registers( /*@unused@*/ modbus_message_t *msg, 
											  /*@unused@*/ uint16_t address, 
											  /*@unused@*/ uint16_t count ){
												  
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_read_input_registers( modbus_message_t *msg, 
											uint16_t address, 
											uint16_t count ){
												
	uint16_t inputs[10], i;
	
	modbus_reset( msg );
	modbus_append_byte( msg, msg->slave_address );
	modbus_append_byte( msg, READ_INPUT_REGISTERS );
	
	if( address + count > 6 ){
		return ILLEGAL_DATA_ADDRESS;
		}
	
	for( i = 0 ; i < 10 ; ++i ){
		inputs[i] = 0;
		}
	
	for( i = 0 ; i < count ; ++i ){
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

uint8_t modbus_handle_write_single_coil( /*@unused@*/ modbus_message_t *msg, 
										 /*@unused@*/ uint16_t address, 
										 /*@unused@*/ uint16_t value ){
											 
	
	if( address > 7 ){
		return ILLEGAL_DATA_ADDRESS;
		}
	
	if( value > 0 ){
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

uint8_t modbus_handle_write_single_register( /*@unused@*/ modbus_message_t *msg, 
											 /*@unused@*/ uint16_t address, 
											 /*@unused@*/ uint16_t value ){
												 
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_read_exception_status( /*@unused@*/ modbus_message_t *msg ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_diagnostics( /*@unused@*/ modbus_message_t *msg, 
												uint16_t subfunction ){
	
	if( subfunction == 0x00 ){ return MODBUS_OK; }
	
	
	
	
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_get_comm_event_log( /*@unused@*/ modbus_message_t *msg ){
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

uint8_t modbus_handle_write_multiple_coils( /*@unused@*/ modbus_message_t *msg, 
											/*@unused@*/ uint16_t address, 
											/*@unused@*/ uint16_t count, 
											/*@unused@*/ uint8_t *values ){
												
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_write_multiple_registers( /*@unused@*/ modbus_message_t *msg, 
												/*@unused@*/ uint16_t address, 
												/*@unused@*/ uint16_t count, 
												/*@unused@*/ uint16_t *values ){
	return ILLEGAL_FUNCTION;
	
	}

uint8_t modbus_handle_report_server_id( /*@unused@*/ modbus_message_t *msg ){
	return ILLEGAL_FUNCTION;
	}

uint8_t modbus_handle_write_mask_register( /*@unused@*/ modbus_message_t *msg, 
										   /*@unused@*/ uint16_t address, 
										   /*@unused@*/ uint16_t and_mask, 
										   /*@unused@*/ uint16_t or_mask ){
	return ILLEGAL_FUNCTION;
	}
