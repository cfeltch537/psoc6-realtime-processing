/**
 * Copyright (c) 2019 Tanzen Medical, Inc.
 *
 * @file frame.h
 * @author Nicholas Haltmeyer <hanicho1@umbc.edu>
 * @date 13 Jan 2019
 * @brief Common definitions for RestEaZe frames.
 */

#ifndef _FRAME_H
#define _FRAME_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Maximum data frame length if all fields are UINTN_MAX. */
#define FRAME_DATA_MTU 55u
#define FRAME_CMD_MTU 32u  /* TODO */
#define FRAME_RESP_MTU 32u /* TODO */

#define FRAME_TYPE 0b01100000u
#define FRAME_TYPE_DATA 0b00000000u
#define FRAME_TYPE_CMD 0b00100000u
#define FRAME_TYPE_RESP 0b01000000u

#define FRAME_CTRL_PARITY 0b10000000u
#define FRAME_CTRL_TYPE 0b01100000u

#define FRAME_CTRL_DATA_DELTA 0b00010000u
#define FRAME_CTRL_DATA_STAT_SENSOR 0b00001100u
#define FRAME_CTRL_DATA_STAT_BATTERY 0b00000011u

#define FRAME_CTRL_CMD 0b00011111u
#define FRAME_CTRL_RESP 0b00011111u

#define FRAME_FIELD_TIME 0b10000000u
#define FRAME_FIELD_TICK 0b01000000u
#define FRAME_FIELD_FUEL 0b00100000u
#define FRAME_FIELD_CAP 0b00010000u
#define FRAME_FIELD_ACCEL 0b00001000u
#define FRAME_FIELD_GYRO 0b00000100u
// TODO: Replacing Magnetometer data with Sentinel Data
//#define FRAME_FIELD_MAG 0b00000010u
#define FRAME_FIELD_SENTINEL 0b00000010u
#define FRAME_FIELD_SYNC 0b00000001u

#define FRAME_CMD_GET 0b00000000u
#define FRAME_CMD_SET 0b00000001u
#define FRAME_CMD_TEST 0b00000010u /* TODO Rename */
#define FRAME_CMD_DUMP 0b00000011u

#define FRAME_VARS 29u

/* SET vars */
#define FRAME_VAR_RUNNING 0x00u
#define FRAME_VAR_DUMPING 0x01u
#define FRAME_VAR_DEST 0x02u
#define FRAME_VAR_DATETIME 0x03u
#define FRAME_VAR_ACCEL_FS 0x04u
#define FRAME_VAR_ACCEL_OFFSET_X 0x05u
#define FRAME_VAR_ACCEL_OFFSET_Y 0x06u
#define FRAME_VAR_ACCEL_OFFSET_Z 0x07u
#define FRAME_VAR_GYRO_FS 0x08u
#define FRAME_VAR_GYRO_OFFSET_X 0x09u
#define FRAME_VAR_GYRO_OFFSET_Y 0x0Au
#define FRAME_VAR_GYRO_OFFSET_Z 0x0Bu
#define FRAME_VAR_MAG_ASA_X 0x0Cu
#define FRAME_VAR_MAG_ASA_Y 0x0Du
#define FRAME_VAR_MAG_ASA_Z 0x0Eu
#define FRAME_VAR_FUEL_MAX 0x0Fu
#define FRAME_VAR_MEM_RESET 0x1Cu
#define FRAME_VAR_MEM_HARD_RESET 0x1Du

/* GET vars*/
#define FRAME_VAR_FUEL 0x10u
#define FRAME_VAR_FUEL_STATUS 0x1Bu
#define FRAME_VAR_VERSION 0x11u
#define FRAME_VAR_FLASH_CHUNKS 0x12u
#define FRAME_VAR_FLASH_HEAD 0x13u
#define FRAME_VAR_FLASH_TAIL 0x14u

/* TEST vars */
#define FRAME_VAR_IMU 0x15u
#define FRAME_VAR_CDC 0x16u

/* DUMP vars */
#define FRAME_VAR_START 0x17u
#define FRAME_VAR_DONE 0x18u
#define FRAME_VAR_ACK 0x19u
#define FRAME_VAR_NACK 0x1Au

typedef struct _data_frame_t {
	/* Control fields */
	uint8_t sensor_status;
	uint8_t battery_status;
	uint8_t field_hdr;
	uint32_t seq;
	uint8_t session;

	/* Data fields */
	uint32_t time_sec;
	uint32_t time_msec;
	uint16_t last_tick_usec;
	uint16_t fuel;
	uint16_t cap[3];
	int16_t accel[3];
	int16_t gyro[3];
	// int16_t mag[3];

	/* Sentinel Fields
	1.  uint32_t for the IR Led data
	2.  uint32_t for the Red Led data
	3.  uint32_t for the Green Led data
	4.  uint16_t for the heart rate data
	5.  uint16_t for the blood oxygen level
	5.  uint8_t for the confidence level
	6.  uint8_t for the finger detected status
	 */
	uint32_t led_ir;
	uint32_t led_red;
	uint32_t led_green;
	uint16_t heart_rate;
	uint16_t blood_oxygen_level;
	uint8_t confidence_level;
	uint8_t finger_detected_status;
	uint32_t temperature;

	uint8_t sync;
} data_frame_t;

typedef struct _command_frame_t {
	uint8_t cmd;
	uint8_t var;
	uint32_t seq;
	uint32_t val;
} command_frame_t;

typedef struct _response_frame_t {
	uint8_t ret;
	uint8_t var;
	uint32_t seq;
	uint32_t val;
} response_frame_t;

/**
 * @brief Computes the parity bit for a byte array.
 * @param enc Encoded frame.
 * @param len Number of bytes in the frame.
 * @return Parity
 * @warning When computing the parity of an encoded frame, the Parity bitfield
 * *will* change the result if set.
 * @see http://www.graphics.stanford.edu/~seander/bithacks.html#ParityParallel
 */
bool bytes_parity(const uint8_t *enc, size_t len);

#endif /* _FRAME_H */
