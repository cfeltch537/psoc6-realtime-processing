/**
 * Copyright (c) 2019 Tanzen Medical, Inc.
 *
 * @file decode.c
 * @author Nicholas Haltmeyer <hanicho1@umbc.edu>
 * @date 13 Jan 2019
 * @brief Routines for decoding RestEaZe frames.
 */

#include <assert.h>
#include <time.h>

#include "decode.h"

/**
 * @brief Decodes a Unix timestamp as a Calendar struct.
 * @param ts_0 Unix timestamp.
 * @return Recovered Calendar struct.
 */
//Calendar decode_cal(const uint32_t *ts_0) {
//	// TODO: TI starts the epoch at 1900
//	struct tm *utc = gmtime((time_t *) ts_0);
//
//	Calendar cal = { .Seconds = utc->tm_sec, .Minutes = utc->tm_min, .Hours =
//			utc->tm_hour, .DayOfWeek = utc->tm_wday, .DayOfMonth = utc->tm_mday,
//			.Month = utc->tm_mon, .Year = utc->tm_year + 1970 };
//
//	return cal;
//}

/**
 * @brief Decodes a frame encoded with consistent overhead byte stuffing.
 * @param enc Source encoded frame.
 * @param dec Destination decoded frame.
 * @param len Length of the source frame in bytes.
 * @return The length of the decoded frame in bytes.
 * @see https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */
size_t decode_cobs(const uint8_t *enc, uint8_t *dec, size_t len) {
	const uint8_t *beg = dec;
	const uint8_t *end = enc + len;

	uint8_t curs = 0xFF;
	uint8_t copy = 0x00;

	while (enc < end) {
		if (copy != 0) {
			*dec++ = *enc++;
		} else {
			if (curs != 0xFF) {
				*dec++ = 0;
			}

			copy = curs = *enc++;

			if (curs == 0) {
				break;
			}
		}

		copy--;
	}

	return dec - beg;
}

/**
 * @brief Decodes the prefix bits for 8-bit fields.
 * @param enc Encoded byte array.
 * @param i Current byte index.
 * @param j Current bit index.
 * @param lsb Flag indicating whether least-significant bits are stripped.
 * @return The current byte cursor (bit-shifted).
 */
uint8_t decode_prefix_uint8(const uint8_t *enc, size_t *i, size_t *j,
		bool *lsb) {
	assert(*j < 8);

	*lsb = (bool) ((enc[*i] >> (7 - *j)) & 0x01);
	uint8_t curs = 0x00;

	if (*j == 7) {
		*i += 1;
		*j = 0;
	} else {
		*j += 1;
	}

	/* Need to read 3 bits for the length. */
	if (*j >= 5) {
		/* j == 5 --> j - 5 == 0 */
		/* j == 6 --> j - 5 == 1 */
		/* j == 7 --> j - 5 == 2 */
		uint8_t rem = (uint8_t) (*j - 5);
		curs |= ((uint8_t) (enc[*i] << *j) >> *j) << rem;

		*i += 1;
		*j = 0;

		if (rem > 0) {
			curs |= enc[*i] >> (8 - rem);
			*j = rem;
		}
	} else {
		/* j == 0 --> 0b11100000 >> 5 == 0b00000111 */
		/* j == 1 --> 0b01110000 >> 4 == 0b00000111 */
		/* j == 2 --> 0b00111000 >> 3 == 0b00000111 */
		/* j == 3 --> 0b00011100 >> 2 == 0b00000111 */
		/* j == 4 --> 0b00001110 >> 1 == 0b00000111 */
		curs |= (enc[*i] >> (5 - *j)) & 0b00000111;
		*j += 3;
	}

	return curs + (uint8_t) 1;
}

/**
 * @brief Decodes the prefix bits for 16-bit fields.
 * @param enc Encoded byte array.
 * @param i Current byte index.
 * @param j Current bit index.
 * @param lsb Flag indicating whether least-significant bits are stripped.
 * @return The current byte cursor (bit-shifted).
 */
uint8_t decode_prefix_uint16(const uint8_t *enc, size_t *i, size_t *j,
		bool *lsb) {
	assert(*j < 8);

	*lsb = (bool) ((enc[*i] >> (7 - *j)) & 0x01);
	uint8_t curs = 0x00;

	if (*j == 7) {
		*i += 1;
		*j = 0;
	} else {
		*j += 1;
	}

	/* Need to read 4 bits for the length. */
	if (*j >= 4) {
		/* j == 4 --> j - 4 == 0 */
		/* j == 5 --> j - 4 == 1 */
		/* j == 6 --> j - 4 == 2 */
		/* j == 7 --> j - 4 == 3 */
		uint8_t rem = (uint8_t) (*j - 4);
		curs |= ((uint8_t) (enc[*i] << *j) >> *j) << rem;

		*i += 1;
		*j = 0;

		if (rem > 0) {
			curs |= enc[*i] >> (8 - rem);
			*j = rem;
		}
	} else {
		/* j == 0 --> 0b11110000 >> 4 == 0b00001111 */
		/* j == 1 --> 0b01111000 >> 3 == 0b00001111 */
		/* j == 2 --> 0b00111100 >> 2 == 0b00001111 */
		/* j == 3 --> 0b00011110 >> 1 == 0b00001111 */
		curs |= (enc[*i] >> (4 - *j)) & 0b00001111;
		*j += 4;
	}

	return curs + (uint8_t) 1;
}

/**
 * @brief Decodes the prefix bits for 32-bit fields.
 * @param enc Encoded byte array.
 * @param i Current byte index.
 * @param j Current bit index.
 * @param lsb Flag indicating whether least-significant bits are stripped.
 * @return The current byte cursor (bit-shifted).
 */
uint8_t decode_prefix_uint32(const uint8_t *enc, size_t *i, size_t *j,
		bool *lsb) {
	assert(*j < 8);

	*lsb = (bool) ((enc[*i] >> (7 - *j)) & 0x01);
	uint8_t curs = 0x00;

	if (*j == 7) {
		*i += 1;
		*j = 0;
	} else {
		*j += 1;
	}

	/* Need to read 5 bits for the length. */
	if (*j >= 3) {
		/* j == 3 --> j - 3 == 0 */
		/* j == 4 --> j - 3 == 1 */
		/* j == 5 --> j - 3 == 2 */
		/* j == 6 --> j - 3 == 3 */
		/* j == 7 --> j - 3 == 4 */
		uint8_t rem = (uint8_t) (*j - 3);
		curs |= ((uint8_t) (enc[*i] << *j) >> *j) << rem;

		*i += 1;
		*j = 0;

		if (rem > 0) {
			curs |= enc[*i] >> (8 - rem);
			*j = rem;
		}
	} else {
		/* j == 0 --> 0b11111000 >> 3 == 0b00011111 */
		/* j == 1 --> 0b01111100 >> 2 == 0b00011111 */
		/* j == 2 --> 0b00111110 >> 1 == 0b00011111 */
		curs |= (enc[*i] >> (3 - *j) & 0b00011111);
		*j += 5;
	}

	return curs + (uint8_t) 1;
}

/**
 * @brief Decodes an 8-bit field.
 * @param enc Encoded byte array.
 * @param i Pointer to byte index.
 * @param j Pointer to bit index.
 * @return Decoded 8-bit field.
 */
uint8_t decode_uint8(const uint8_t *enc, size_t *i, size_t *j) {
	bool lsb = false;
	uint8_t curs = 0x00;
	size_t len = decode_prefix_uint8(enc, i, j, &lsb);
	size_t k;

	for (k = 0; k < len; k++) {
		curs <<= 1;
		curs |= (enc[*i] >> (7 - *j)) & 0x01;

		if (*j == 7) {
			*i += 1;
			*j = 0;
		} else {
			*j += 1;
		}
	}

	if (lsb) {
		curs <<= 8 - len;
	}

	return curs;
}

/**
 * @brief Decodes a 16-bit field.
 * @param enc Encoded byte array.
 * @param i Pointer to byte index.
 * @param j Pointer to bit index.
 * @return Decoded 16-bit field.
 */
uint16_t decode_uint16(const uint8_t *enc, size_t *i, size_t *j) {
	bool lsb = false;
	uint16_t curs = 0x0000;
	size_t len = decode_prefix_uint16(enc, i, j, &lsb);
	size_t k;

	for (k = 0; k < len; k++) {
		curs <<= 1;
		curs |= (enc[*i] >> (7 - *j)) & 0x01;

		if (*j == 7) {
			*i += 1;
			*j = 0;
		} else {
			*j += 1;
		}
	}

	if (lsb) {
		curs <<= 16 - len;
	}

	return curs;
}

/**
 * @brief Decodes a 32-bit field.
 * @param enc Encoded byte array.
 * @param i Pointer to byte index.
 * @param j Pointer to bit index.
 * @return Decoded 32-bit field.
 */
uint32_t decode_uint32(const uint8_t *enc, size_t *i, size_t *j) {
	bool lsb = false;
	uint32_t curs = 0x00000000;
	size_t len = decode_prefix_uint32(enc, i, j, &lsb);
	size_t k;

	for (k = 0; k < len; k++) {
		curs <<= 1;
		curs |= (enc[*i] >> (7 - *j)) & 0x01;

		if (*j == 7) {
			*i += 1;
			*j = 0;
		} else {
			*j += 1;
		}
	}

	if (lsb) {
		curs <<= 32 - len;
	}

	return curs;
}

/**
 * @brief Decodes a RestEaZe data frame.
 * @param data Source data frame.
 * @param dec Decoded data frame.
 * @return EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise.
 */
int decode_data_frame(const uint8_t *data, data_frame_t *dec) {
	size_t i = 0;
	size_t j = 0;

	if ((data[i] & FRAME_CTRL_TYPE) != FRAME_TYPE_DATA) {
		return EXIT_FAILURE;
	}

	dec->sensor_status = (uint8_t) (data[i] & FRAME_CTRL_DATA_STAT_SENSOR);
	dec->battery_status = (uint8_t) (data[i] & FRAME_CTRL_DATA_STAT_BATTERY);
	i++;

	dec->field_hdr = data[i];
	i++;

	dec->seq = decode_uint32(data, &i, &j);

	if (dec->field_hdr & FRAME_FIELD_TIME) {
		dec->time_sec = decode_uint32(data, &i, &j);
		dec->time_msec = decode_uint32(data, &i, &j);
	}

	if (dec->field_hdr & FRAME_FIELD_TICK) {
		dec->last_tick_usec = decode_uint16(data, &i, &j);
	}

	if (dec->field_hdr & FRAME_FIELD_FUEL) {
		dec->fuel = decode_uint16(data, &i, &j);
	}

	if (dec->field_hdr & FRAME_FIELD_CAP) {
		dec->cap[0] = decode_uint16(data, &i, &j);
		dec->cap[1] = decode_uint16(data, &i, &j);
		dec->cap[2] = decode_uint16(data, &i, &j);
	}

	if (dec->field_hdr & FRAME_FIELD_ACCEL) {
		dec->accel[0] = decode_uint16(data, &i, &j);
		dec->accel[1] = decode_uint16(data, &i, &j);
		dec->accel[2] = decode_uint16(data, &i, &j);
	}

	if (dec->field_hdr & FRAME_FIELD_GYRO) {
		dec->gyro[0] = decode_uint16(data, &i, &j);
		dec->gyro[1] = decode_uint16(data, &i, &j);
		dec->gyro[2] = decode_uint16(data, &i, &j);
	}

	if (dec->field_hdr & FRAME_FIELD_SENTINEL) {
		/* Sentinel Fields
		1.  uint32_t for the IR Led data
		2.  uint32_t for the Red Led data
		3.  uint32_t for the Green Led data
		4.  uint16_t for the heart rate data
		5.  uint16_t for the blood oxygen level
		5.  uint8_t for the confidence level
		6.  uint8_t for the finger detected status
		 */
		dec->led_ir = decode_uint32(data, &i, &j);
		dec->led_red = decode_uint32(data, &i, &j);
		dec->led_green = decode_uint32(data, &i, &j);
		dec->heart_rate = decode_uint16(data, &i, &j);
		dec->blood_oxygen_level = decode_uint16(data, &i, &j);
		dec->confidence_level = decode_uint8(data, &i, &j);
		dec->finger_detected_status = decode_uint8(data, &i, &j);
		dec->temperature = decode_uint32(data, &i, &j);
	}

	if (dec->field_hdr & FRAME_FIELD_SYNC) {
		dec->sync = decode_uint8(data, &i, &j);
	}

	/* parity | data[0] & FRAME_CTRL_PARITY |   *
	 *      0 |                           0 | 1 *
	 *      0 |                           1 | 1 *
	 *      1 |                           0 | 0 *
	 *      1 |                           1 | 0 */
	bool parity = bytes_parity(data, i + 1);

	if (parity) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * @brief Decodes a delta-encoded RestEaZe data frame.
 * @param data_0 Data frame from the prior time step.
 * @param data_1 Encoded data frame from the current time step.
 * @param dec Decoded delta-encoded data frame.
 * @return EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise.
 */
int decode_delta_frame(const data_frame_t *data_0, const uint8_t *data_1,
		data_frame_t *dec) {
	assert(data_1[0] & FRAME_CTRL_DATA_DELTA);

	int err = decode_data_frame(data_1, dec);

	if (err) {
		return err;
	}

	/* The sequence number of data_0 must be immediately prior (without overflow).
	 */
	assert(
			data_0->seq == dec->seq - 1 || data_0->seq == UINT32_MAX && dec->seq == 0);

#ifndef NDEBUG
	/* The fields present in the new frame must be a subset of the fields in the
	 * prior. */
	size_t i;

	for (i = 0; i < 8; i++) {
		if (((dec->field_hdr >> (7 - i)) & 0x01) &&
				!((data_0->field_hdr >> (7 - i)) & 0x01)) {
			assert(false);
		}
	}
#endif

	dec->time_sec += data_0->time_sec;
	dec->time_msec += data_0->time_msec;
	dec->last_tick_usec += data_0->last_tick_usec;
	dec->fuel += data_0->fuel;
	dec->cap[0] += data_0->cap[0];
	dec->cap[1] += data_0->cap[1];
	dec->cap[2] += data_0->cap[2];
	dec->accel[0] += data_0->accel[0];
	dec->accel[1] += data_0->accel[1];
	dec->accel[2] += data_0->accel[2];
	dec->gyro[0] += data_0->gyro[0];
	dec->gyro[1] += data_0->gyro[1];
	dec->gyro[2] += data_0->gyro[2];
	dec->led_ir += data_0->led_ir;
	dec->led_red += data_0->led_red;
	dec->led_green += data_0->led_green;
	dec->heart_rate += data_0->heart_rate;
	dec->blood_oxygen_level += data_0->blood_oxygen_level;
	dec->confidence_level += data_0->confidence_level;
	dec->finger_detected_status += data_0->finger_detected_status;
	dec->temperature += data_0->temperature;
	dec->sync += data_0->sync;

	return EXIT_SUCCESS;
}

/**
 * @brief Decodes a RestEaZe command frame.
 * @param cmd Source command frame.
 * @param dec Decoded command frame.
 * @return EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise.
 */
int decode_command_frame(const uint8_t *cmd, command_frame_t *dec) {
	size_t i = 0;
	size_t j = 0;

	if ((cmd[i] & FRAME_CTRL_TYPE) != FRAME_TYPE_CMD) {
		return EXIT_FAILURE;
	}

	dec->cmd = (uint8_t) (cmd[i] & FRAME_CTRL_CMD);
	i++;

	dec->var = cmd[i];
	i++;

	dec->seq = decode_uint32(cmd, &i, &j);
	dec->val = decode_uint32(cmd, &i, &j);

	/* parity | cmd[0] & FRAME_CTRL_PARITY |   *
	 *      0 |                          0 | 1 *
	 *      0 |                          1 | 1 *
	 *      1 |                          0 | 0 *
	 *      1 |                          1 | 0 */
	bool parity = bytes_parity(cmd, i + 1);

	if (parity) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * @brief Decodes a RestEaZe response frame.
 * @param cmd Source response frame.
 * @param dec Decoded response frame.
 * @return EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise.
 */
int decode_response_frame(const uint8_t *resp, response_frame_t *dec) {
	size_t i = 0;
	size_t j = 0;

	if ((resp[i] & FRAME_CTRL_TYPE) != FRAME_TYPE_RESP) {
		return EXIT_FAILURE;
	}

	dec->ret = (uint8_t) (resp[i] & FRAME_CTRL_RESP);
	i++;

	dec->var = resp[i];
	i++;

	dec->seq = decode_uint32(resp, &i, &j);
	dec->val = decode_uint32(resp, &i, &j);

	/* parity | resp[0] & FRAME_CTRL_PARITY |   *
	 *      0 |                           0 | 1 *
	 *      0 |                           1 | 1 *
	 *      1 |                           0 | 0 *
	 *      1 |                           1 | 0 */
	bool parity = bytes_parity(resp, i + 1);

	if (parity) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
