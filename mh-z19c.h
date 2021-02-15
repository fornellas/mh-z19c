#ifndef MH_Z19C_H
#define MH_Z19C_H

#include <inttypes.h>
#include <stdbool.h>

//
// Defines
//


//
// Enums & Structs
//

enum mh_z19c_error {
	MH_Z19C_ERROR_NONE,
	MH_Z19C_ERROR_WRITE_SERIAL,
	MH_Z19C_ERROR_INVALID_START_BYTE,
	MH_Z19C_ERROR_BAD_CHECKSUM,
	MH_Z19C_ERROR_READ_SERIAL,
	MH_Z19C_ERROR_INVALID_RESPONSE_COMMAND,
};

//
// Functions
//

// enum mh_z19c_error mh_z19c_read_pwm(
// 	uint16_t *co2_concentration,
// 	int (*read_pwm_pin)(bool *),
// 	uint32_t (*get_ms)(void)
// );

enum mh_z19c_error mh_z19c_read(
	uint16_t *co2_concentration,
	int (*write_serial)(uint8_t),
	int (*read_serial)(uint8_t *)
);

enum mh_z19c_error mh_z19c_self_calibration_for_zero_point(
	bool enabled,
	int (*write_serial)(uint8_t)
);

char *mh_z19c_strerror(enum mh_z19c_error mh_z19c_errno);

#endif