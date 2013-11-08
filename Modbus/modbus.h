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

#ifndef _MODBUS_H_
#define _MODBUS_H_

#define F_CPU 8000000UL		/* Clock Frequency = 8Mhz */

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>



#define MAX_LENGTH 256


#define READ_COILS					0x01
#define READ_DISCRETE_INPUTS		0x02
#define READ_HOLDING_REGISTERS		0x03
#define READ_INPUT_REGISTERS		0x04
#define WRITE_SINGLE_COIL			0x05
#define WRITE_SINGLE_REGISTER		0x06
#define READ_EXCEPTION_STATUS		0x07
#define DIAGNOSTICS					0x08
#define GET_COMM_COUNTER			0x0B
#define GET_COMM_EVENT_LOG			0x0C
#define WRITE_MULTIPLE_COILS		0x0F
#define WRITE_MULTIPLE_REGISTERS 	0x10
#define REPORT_SERVER_ID			0x11
#define READ_FILE_RECORD			0x14
#define WRITE_FILE_RECORD			0x15
#define WRITE_MASK_REGISTER			0x16
#define RW_MULTIPLE_REGISTERS		0x17
#define READ_FIFO_QUEUE				0x18
#define READ_DEVICE_IDENTIFICATION	0x2B

#define MODBUS_OK					0x00
#define MODBUS_INVALID				0x01

#define ILLEGAL_FUNCTION			0x01
#define ILLEGAL_DATA_ADDRESS		0x02
#define ILLEGAL_DATA_VALUE			0x03
#define SERVER_DEVICE_FAILURE		0x04

/* SLAVEADDR(1) + FUNCTION(1) + ADDR(2) + COUNT(2) + CRC(2) */
#define ADDRESS_OFFSET				0x02
#define SUBFUNC_OFFSET				0x02
#define COUNT_OFFSET				0x04
#define VALUE_OFFSET				0x04
#define AND_MASK_OFFSET				0x04
#define OR_MASK_OFFSET				0x06


typedef struct {
	uint8_t buffer[MAX_LENGTH];
	uint8_t length, target_length, slave_address;
	} modbus_message_t;

extern uint16_t modbus_global_event_counter;

uint16_t modbus_compute_crc( modbus_message_t *msg );
void modbus_reset( modbus_message_t *msg );
void modbus_append_byte( modbus_message_t *msg, uint8_t byte );
void modbus_append_word( modbus_message_t *msg, uint16_t word );
void modbus_append_crc( modbus_message_t *msg );
uint16_t modbus_extract_word( modbus_message_t *msg, uint8_t pos );
uint8_t modbus_validate_function( modbus_message_t *msg );
uint8_t modbus_get_target_length( modbus_message_t *msg );
uint8_t modbus_inspect( modbus_message_t *msg );
void modbus_create_exception_response( modbus_message_t *msg, uint8_t func, uint8_t exception );

extern uint8_t modbus_handle_read_coils( modbus_message_t *msg, uint16_t address, uint16_t count );
extern uint8_t modbus_handle_read_discrete_inputs( modbus_message_t *msg, uint16_t address, uint16_t count );
extern uint8_t modbus_handle_read_discrete_inputs( modbus_message_t *msg, uint16_t address, uint16_t count );
extern uint8_t modbus_handle_read_input_registers( modbus_message_t *msg, uint16_t address, uint16_t count );
extern uint8_t modbus_handle_read_holding_registers( modbus_message_t *msg, uint16_t address, uint16_t count );
extern uint8_t modbus_handle_write_single_coil( modbus_message_t *msg, uint16_t address, uint16_t value );
extern uint8_t modbus_handle_write_single_register( modbus_message_t *msg, uint16_t address, uint16_t value );
extern uint8_t modbus_handle_read_exception_status( modbus_message_t *msg );
extern uint8_t modbus_handle_diagnostics( modbus_message_t *msg, uint16_t subfunction );
extern uint8_t modbus_handle_get_comm_event_log( modbus_message_t *msg );
extern uint8_t modbus_handle_get_comm_event_counter( modbus_message_t *msg );
extern uint8_t modbus_handle_write_multiple_coils( modbus_message_t *msg, uint16_t address, uint16_t count, uint8_t *values );
extern uint8_t modbus_handle_write_multiple_registers( modbus_message_t *msg, uint16_t address, uint16_t count, uint16_t *values );
extern uint8_t modbus_handle_report_server_id( modbus_message_t *msg );
extern uint8_t modbus_handle_write_mask_register( modbus_message_t *msg, uint16_t address, uint16_t and_mask, uint16_t or_mask );

uint8_t modbus_handle_message( modbus_message_t *msg, uint8_t own_address );





#endif
