#include "mh-z19c.h"

//
// Definitions
//

#define START_BYTE 0xff
#define RESERVED_BYTE 0x01


enum mh_z19c_commands {
	COMMAND_READ_CO2_CONCENTRATION = 0x86,
	COMMAND_SELF_CALIBRATION_FUNCTION_FOR_ZERO_POINT = 0x79,
};

static char *error_strings[] = {
	[MH_Z19C_ERROR_NONE] = "No error",
	[MH_Z19C_ERROR_WRITE_SERIAL] = "Error writing serial data",
	[MH_Z19C_ERROR_INVALID_START_BYTE] = "Invalid start character",
	[MH_Z19C_ERROR_BAD_CHECKSUM] = "Bad checksum",
	[MH_Z19C_ERROR_READ_SERIAL] = "Error reading serial data",
	[MH_Z19C_ERROR_INVALID_RESPONSE_COMMAND] = "Invalid response command",
};

//
// Structs
//

__attribute__((packed)) struct mh_z19c_command {
	uint8_t start_byte;
	uint8_t reserved;
	uint8_t command;
	uint8_t data[5];
	uint8_t checksum;
};

__attribute__((packed)) struct mh_z19c_return_value {
	uint8_t start_byte;
	uint8_t command;
	uint8_t data[6];
	uint8_t checksum;
};

//
// Helper Functions
//

uint8_t checksum(uint8_t *packet) {
	uint8_t value;

	value = 0;
	packet++;

	for(int i=0 ; i < 7 ; i++, packet++)
		value += *packet;

	value = 0xff - value;
	value++;

	return value;
}

static enum mh_z19c_error send_command(
	enum mh_z19c_commands command,
	uint8_t data[5],
	int (*write_serial)(uint8_t)
) {
	struct mh_z19c_command packet;
	uint8_t *ptr;
	int i;

	packet.start_byte = START_BYTE;
	packet.reserved = RESERVED_BYTE;
	packet.command = command;
	for(int i=0 ; i < sizeof(packet.data) ; i++)
		packet.data[i] = data[i];
	packet.checksum = checksum((uint8_t *)&packet);

	for(i=0, ptr=(uint8_t *)&packet ; i < sizeof(struct mh_z19c_command) ; i++, ptr++)
		if(write_serial(*ptr))
			return MH_Z19C_ERROR_WRITE_SERIAL;

	return MH_Z19C_ERROR_NONE;
}

static enum mh_z19c_error read_return_value(
	struct mh_z19c_return_value *return_value,
	int (*read_serial)(uint8_t *)
) {
	uint8_t c;

	// Start byte
	if(read_serial(&c))
		return MH_Z19C_ERROR_READ_SERIAL;
	if(c != START_BYTE)
		return MH_Z19C_ERROR_INVALID_START_BYTE;
	return_value->start_byte = c;

	// Command
	if(read_serial(&c))
		return MH_Z19C_ERROR_READ_SERIAL;
	return_value->command = c;

	// Data
	for(int i=0; i < sizeof(return_value->data) ; i++)
		if(read_serial(&c))
			return MH_Z19C_ERROR_READ_SERIAL;
		else
			return_value->data[i] = c;

	// Checksum
	if(read_serial(&c))
		return MH_Z19C_ERROR_READ_SERIAL;
	return_value->checksum = c;

	if(return_value->checksum != checksum((uint8_t *)return_value))
		return MH_Z19C_ERROR_BAD_CHECKSUM;

	return MH_Z19C_ERROR_NONE;
}

//
// Public API
//

enum mh_z19c_error mh_z19c_read(
	uint16_t *co2_concentration,
	int (*write_serial)(uint8_t),
	int (*read_serial)(uint8_t *)
) {
	enum mh_z19c_error ret;
	uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
	struct mh_z19c_return_value return_value;

	if((ret=send_command(COMMAND_READ_CO2_CONCENTRATION, data, write_serial)))
		return ret;

	if((ret=read_return_value(&return_value, read_serial)))
		return ret;

	if(return_value.command != COMMAND_READ_CO2_CONCENTRATION)
		return MH_Z19C_ERROR_INVALID_RESPONSE_COMMAND;

	*co2_concentration = (uint16_t)(return_value.data[0]) * 256 + return_value.data[1];

	return MH_Z19C_ERROR_NONE;
}

enum mh_z19c_error mh_z19c_self_calibration_for_zero_point(
	bool enabled,
	int (*write_serial)(uint8_t)
) {
	uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	if(enabled)
		data[0] = 0xa0;

	return send_command(COMMAND_SELF_CALIBRATION_FUNCTION_FOR_ZERO_POINT, data, write_serial);
}

char *mh_z19c_strerror(enum mh_z19c_error mh_z19c_errno) {
	return error_strings[mh_z19c_errno];
}