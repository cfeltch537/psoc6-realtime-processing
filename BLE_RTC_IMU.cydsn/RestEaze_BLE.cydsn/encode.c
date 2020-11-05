/**
 * Copyright (c) 2019 Tanzen Medical, Inc.
 *
 * @file encode.c
 * @author Nicholas Haltmeyer <hanicho1@umbc.edu>
 * @date 13 Jan 2019
 * @brief Routines for encoding RestEaZe frames.
 */

#include "encode.h"

#include <assert.h>
#include <time.h>

/**
 * @brief Encodes a Calendar struct as a Unix timestamp.
 * @param cal_0 Calendar time.
 * @return Seconds since 1 Jan 1970.
 */
//uint32_t encode_cal(const Calendar *cal_0) {
//	struct tm cal_1 = { 0 };
//
//	cal_1.tm_sec = cal_0->Seconds;
//	cal_1.tm_min = cal_0->Minutes;
//	cal_1.tm_hour = cal_0->Hours;
//	cal_1.tm_mday = cal_0->DayOfMonth;
//	cal_1.tm_mon = cal_0->Month;
//	cal_1.tm_year = cal_0->Year - 1970;
//	cal_1.tm_wday = cal_0->DayOfWeek;
//
//	/* NOTE setting tm_isdst to -1 will force mktime to check whether DST is in
//	 * effect. */
//	cal_1.tm_isdst = -1;
//
//	return (uint32_t) mktime(&cal_1);
//}

/**
 * @brief Encodes a frame encoded with consistent overhead byte stuffing.
 * @param dec Source decoded frame.
 * @param enc Destination encoded frame.
 * @param len Length of the source frame in bytes.
 * @return The length of the encoded frame in bytes.
 * @see https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */
size_t encode_cobs(const uint8_t *dec, uint8_t *enc, size_t len) {
	const uint8_t *beg = enc;
	const uint8_t *end = dec + len;

	uint8_t *code;
	uint8_t curs;

	code = enc++;
	curs = 1;

	while (dec < end) {
		if (curs != 0xFF) {
			uint8_t tmp = *dec++;

			if (tmp != 0) {
				*enc++ = tmp;
				curs++;
				continue;
			}
		}

		*code = curs;
		code = enc++;
		curs = 1;
	}

	*code = curs;
	*enc = 0;
	return enc - beg;
}

/**
 * @brief Computes the number of most-significant padding bits for 8-bit fields.
 * @param data 8-bit field.
 * @return Number of most-significant padding bits.
 * @see https://graphics.stanford.edu/~seander/bithacks.html
 */
uint8_t msb_offset8(uint8_t data) {
	uint8_t n = 0;

	if (data == 0)
		return 8;
	if (data <= 0x0F)
		n += 4, data <<= 4u;
	if (data <= 0x3F)
		n += 2, data <<= 2u;
	if (data <= 0x7F)
		n++;

	return n;
}

/**
 * @brief Computes the number of most-significant padding bits for 16-bit
 * fields.
 * @param data 16-bit field.
 * @return Number of most-significant padding bits.
 * @see https://graphics.stanford.edu/~seander/bithacks.html
 */
uint8_t msb_offset16(uint16_t data) {
	uint8_t n = 0;

	if (data == 0)
		return 16;
	if (data <= 0x00FF)
		n += 8, data <<= 8u;
	if (data <= 0x0FFF)
		n += 4, data <<= 4u;
	if (data <= 0x3FFF)
		n += 2, data <<= 2u;
	if (data <= 0x7FFF)
		n++;

	return n;
}

/**
 * @brief Computes the number of most-significant padding bits for 32-bit
 * fields.
 * @param data 32-bit field.
 * @return Number of most-significant padding bits.
 * @see https://graphics.stanford.edu/~seander/bithacks.html
 */
uint8_t msb_offset32(uint32_t data) {
	uint8_t n = 0;

	if (data <= 0x0000FFFF)
		n += 16, data <<= 16u;
	if (data <= 0x00FFFFFF)
		n += 8, data <<= 8u;
	if (data <= 0x0FFFFFFF)
		n += 4, data <<= 4u;
	if (data <= 0x3FFFFFFF)
		n += 2, data <<= 2u;
	if (data <= 0x7FFFFFFF)
		n++;

	return n;
}

/**
 * @brief Computes the number of least-significant padding bits for 8-bit
 * fields.
 * @param data 8-bit field.
 * @return Number of least-significant padding bits.
 * @see https://graphics.stanford.edu/~seander/bithacks.html
 */
uint8_t lsb_offset8(uint8_t data) {
	uint8_t n = 1;

	if (data & 0x1u)
		return 0;
	if (data == 0)
		return 8;
	if ((data & 0x0Fu) == 0)
		n += 4, data >>= 4u;
	if ((data & 0x03u) == 0)
		n += 2, data >>= 2u;

	n -= data & 0x1u;

	return n;
}

/**
 * @brief Computes the number of least-significant padding bits for 16-bit
 * fields.
 * @param data 16-bit field.
 * @return Number of least-significant padding bits.
 * @see https://graphics.stanford.edu/~seander/bithacks.html
 */
uint8_t lsb_offset16(uint16_t data) {
	uint8_t n = 1;

	if (data & 0x1u)
		return 0;
	if (data == 0)
		return 16;
	if ((data & 0x00FFu) == 0)
		n += 8, data >>= 8u;
	if ((data & 0x000Fu) == 0)
		n += 4, data >>= 4u;
	if ((data & 0x0003u) == 0)
		n += 2, data >>= 2u;

	n -= data & 0x1u;

	return n;
}

/**
 * @brief Computes the number of least-significant padding bits for 32-bit
 * fields.
 * @param data 32-bit field.
 * @return Number of least-significant padding bits.
 * @see https://graphics.stanford.edu/~seander/bithacks.html
 */
uint8_t lsb_offset32(uint32_t data) {
	uint8_t n = 1;

	if (data & 0x1u)
		return 0;
	if (data == 0)
		return 32;
	if ((data & 0xFFFFu) == 0)
		n += 16, data >>= 16u;
	if ((data & 0x00FFu) == 0)
		n += 8, data >>= 8u;
	if ((data & 0x000Fu) == 0)
		n += 4, data >>= 4u;
	if ((data & 0x0003u) == 0)
		n += 2, data >>= 2u;

	n -= data & 0x1u;

	return n;
}

/**
 * @brief Encodes the prefix bits for 8-bit fields.
 * @param rem Pointer to the current byte cursor.
 * @param enc Encoded byte array.
 * @param i Current byte index.
 * @param j Current bit index.
 * @return Number of bits in the encoded field.
 */
uint8_t encode_prefix_uint8(uint8_t *rem, uint8_t *enc, size_t *i, size_t *j) {
	assert(*j < 8);

	uint8_t n_bits = 8;
	uint8_t msb_offset = msb_offset8(*rem);
	uint8_t lsb_offset = lsb_offset8(*rem);

	/* Account for empty fields (all zeros). */
	msb_offset = msb_offset == 8 ? (uint8_t) 7 : msb_offset;
	lsb_offset = lsb_offset == 8 ? (uint8_t) 7 : lsb_offset;

	if (msb_offset >= lsb_offset) {
		n_bits -= msb_offset;
		(*rem) <<= msb_offset;
	} else {
		n_bits -= lsb_offset;
	}

	uint8_t curs = 0;
	uint8_t n_rem = n_bits;

	/* First bit indicates MSB or LSB shift. */
	if (msb_offset < lsb_offset) {
		curs = 0b10000000;
	}

	/* Next 3 bits indicate length (in bits). */
	curs |= (n_bits - 1) << 4;
	curs |= (uint8_t) ((*rem >> 4) & 0b1111);
	n_rem = (uint8_t) (n_rem < 4 ? 0 : n_rem - 4);
	(*rem) <<= 4;
	enc[*i] |= curs >> *j;

	/* Only move over to the next byte if we have remaining values in the cursor.
	 */
	if (4 + n_bits >= 8 - *j) {
		*i += 1;
		enc[*i] |= curs << (8 - *j);
	}

	*j = (*j + 4 + n_bits - n_rem) % 8;

	return n_rem;
}

/**
 * @brief Encodes the prefix bits for 16-bit fields.
 * @param rem Pointer to the current byte cursor.
 * @param enc Encoded byte array.
 * @param i Current byte index.
 * @param j Current bit index.
 * @return Number of bits in the encoded field.
 */
uint8_t encode_prefix_uint16(uint16_t *rem, uint8_t *enc, size_t *i,
		size_t *j) {
	assert(*j < 8);

	uint8_t n_bits = 16;
	uint8_t msb_offset = msb_offset16(*rem);
	uint8_t lsb_offset = lsb_offset16(*rem);

	/* Account for empty fields (all zeros). */
	msb_offset = msb_offset == 16 ? (uint8_t) 15 : msb_offset;
	lsb_offset = lsb_offset == 16 ? (uint8_t) 15 : lsb_offset;

	if (msb_offset >= lsb_offset) {
		n_bits -= msb_offset;
		(*rem) <<= msb_offset;
	} else {
		n_bits -= lsb_offset;
	}

	uint8_t n_rem = n_bits;
	uint8_t curs = 0;

	/* First bit indicates MSB or LSB shift. */
	if (msb_offset < lsb_offset) {
		curs = 0b10000000;
	}

	/* Next 4 bits indicate length (in bits) */
	curs |= (n_bits - 1) << 3;
	curs |= (uint8_t) ((*rem >> 13) & 0b111);
	n_rem = (uint8_t) (n_rem < 3 ? 0 : n_rem - 3);
	(*rem) <<= 3;
	enc[*i] |= curs >> *j;

	/* Only move over to the next byte if we have remaining values in the cursor.
	 */
	if (5 + n_bits >= 8 - *j) {
		*i += 1;
		enc[*i] |= curs << (8 - *j);
	}

	*j = (*j + 5 + n_bits - n_rem) % 8;

	return n_rem;
}

/**
 * @brief Encodes the prefix bits for 32-bit fields.
 * @param rem Pointer to the current byte cursor.
 * @param enc Encoded byte array.
 * @param i Current byte index.
 * @param j Current bit index.
 * @return Number of bits in the encoded field.
 */
uint8_t encode_prefix_uint32(uint32_t *rem, uint8_t *enc, size_t *i,
		size_t *j) {
	assert(*j < 8);

	uint8_t n_bits = 32;
	uint8_t msb_offset = msb_offset32(*rem);
	uint8_t lsb_offset = lsb_offset32(*rem);

	/* Account for empty fields (all zeros). */
	msb_offset = msb_offset == 32 ? (uint8_t) 31 : msb_offset;
	lsb_offset = lsb_offset == 32 ? (uint8_t) 31 : lsb_offset;

	if (msb_offset >= lsb_offset) {
		n_bits -= msb_offset;
		(*rem) <<= msb_offset;
	} else {
		n_bits -= lsb_offset;
	}

	uint8_t n_rem = n_bits;
	uint8_t curs = 0;

	/* First bit indicates MSB or LSB shift. */
	if (msb_offset < lsb_offset) {
		curs = 0b10000000;
	}

	/* Next 5 bits indicate length (in bits) */
	curs |= (n_bits - 1) << 2;
	curs |= (uint8_t) ((*rem >> 30) & 0b11);
	n_rem = (uint8_t) (n_rem < 2 ? 0 : n_rem - 2);
	(*rem) <<= 2;
	enc[*i] |= curs >> *j;

	/* Only move over to the next byte if we have remaining values in the cursor.
	 */
	if (6 + n_bits >= 8 - *j) {
		*i += 1;
		enc[*i] |= curs << (8 - *j);
	}

	*j = (*j + 6 + n_bits - n_rem) % 8;

	return n_rem;
}

/**
 * @brief Encodes an 8-bit field.
 * @param data 8-bit field.
 * @param enc Encoded byte array.
 * @param i Pointer to byte index.
 * @param j Pointer to bit index.
 */
void encode_uint8(uint8_t data, uint8_t *enc, size_t *i, size_t *j) {
	assert(*j < 8);

	uint8_t rem = data;
	uint8_t n_rem = encode_prefix_uint8(&rem, enc, i, j);

	while (n_rem > 0) {
		uint8_t curs = rem;
		enc[*i] |= curs >> *j;

		if (n_rem + *j < 8) {
			*j += n_rem;
			n_rem = 0;
		} else {
			(*i)++;
			enc[*i] |= curs << (8 - *j);
			n_rem -= (8 - *j);
			rem <<= (8 - *j);
			*j = 0;
		}
	}
}

/**
 * @brief Encodes a 16-bit field.
 * @param data 16-bit field.
 * @param enc Encoded byte array.
 * @param i Pointer to byte index.
 * @param j Pointer to bit index.
 */
void encode_uint16(uint16_t data, uint8_t *enc, size_t *i, size_t *j) {
	assert(*j < 8);

	uint16_t rem = data;
	uint8_t n_rem = encode_prefix_uint16(&rem, enc, i, j);

	while (n_rem > 0) {
		uint8_t curs = (uint8_t) ((rem >> 8) & 0xFF);
		enc[*i] |= curs >> *j;

		if (n_rem + *j < 8) {
			*j += n_rem;
			n_rem = 0;
		} else {
			(*i)++;
			enc[*i] |= curs << (8 - *j);
			n_rem -= (8 - *j);
			rem <<= (8 - *j);
			*j = 0;
		}
	}
}

/**
 * @brief Encodes a 32-bit field.
 * @param data 32-bit field.
 * @param enc Encoded byte array.
 * @param i Pointer to byte index.
 * @param j Pointer to bit index.
 */
void encode_uint32(uint32_t data, uint8_t *enc, size_t *i, size_t *j) {
	assert(*j < 8);

	uint32_t rem = data;
	uint8_t n_rem = encode_prefix_uint32(&rem, enc, i, j);

	while (n_rem > 0) {
		uint8_t curs = (uint8_t) ((rem >> 24) & 0xFF);
		enc[*i] |= curs >> *j;

		if (n_rem + *j < 8) {
			*j += n_rem;
			n_rem = 0;
		} else {
			(*i)++;
			enc[*i] |= curs << (8 - *j);
			n_rem -= (8 - *j);
			rem <<= (8 - *j);
			*j = 0;
		}
	}
}

/**
 * @brief Encodes a RestEaZe data frame.
 * @param data Source data frame.
 * @param enc Encoded data frame.
 * @return Length of the encoded frame in bytes.
 * @warning enc must be at least FRAME_MTU in length.
 */
size_t encode_data_frame(const data_frame_t *data, uint8_t *enc) {
	size_t i = 0; /* Byte index */
	size_t j = 0; /* Bit index */

	/* Frame header */
	enc[i] = FRAME_TYPE_DATA;
	enc[i] |= data->sensor_status & FRAME_CTRL_DATA_STAT_SENSOR;
	enc[i] |= data->battery_status & FRAME_CTRL_DATA_STAT_BATTERY;
	i++;

	/* Field header */
	enc[i] = data->field_hdr;
	i++;

	encode_uint32(data->seq, enc, &i, &j);

	encode_uint8(data->session, enc, &i, &j);

	if (data->field_hdr & FRAME_FIELD_TIME) {
		encode_uint32(data->time_sec, enc, &i, &j);
		encode_uint32(data->time_msec, enc, &i, &j);
	}

	if (data->field_hdr & FRAME_FIELD_TICK) {
		encode_uint16(data->last_tick_usec, enc, &i, &j);
	}

	if (data->field_hdr & FRAME_FIELD_FUEL) {
		encode_uint16(data->fuel, enc, &i, &j);
	}

	if (data->field_hdr & FRAME_FIELD_CAP) {
		encode_uint16(data->cap[0], enc, &i, &j);
		encode_uint16(data->cap[1], enc, &i, &j);
		encode_uint16(data->cap[2], enc, &i, &j);
	}

	if (data->field_hdr & FRAME_FIELD_ACCEL) {
		encode_uint16((uint16_t) data->accel[0], enc, &i, &j);
		encode_uint16((uint16_t) data->accel[1], enc, &i, &j);
		encode_uint16((uint16_t) data->accel[2], enc, &i, &j);
	}

	if (data->field_hdr & FRAME_FIELD_GYRO) {
		encode_uint16((uint16_t) data->gyro[0], enc, &i, &j);
		encode_uint16((uint16_t) data->gyro[1], enc, &i, &j);
		encode_uint16((uint16_t) data->gyro[2], enc, &i, &j);
	}

	if (data->field_hdr & FRAME_FIELD_SENTINEL) {
		/* Sentinel Fields
		1.  uint32_t for the IR Led data
		2.  uint32_t for the Red Led data
		3.  uint32_t for the Green Led data
		4.  uint16_t for the heart rate data
		5.  uint16_t for the blood oxygen level
		5.  uint8_t for the confidence level
		6.  uint8_t for the finger detected status
		 */
		encode_uint32(data->led_ir, enc, &i, &j);
		encode_uint32(data->led_red, enc, &i, &j);
		encode_uint32(data->led_green, enc, &i, &j);
		encode_uint16(data->heart_rate, enc, &i, &j);
		encode_uint16(data->blood_oxygen_level, enc, &i, &j);
		encode_uint8(data->confidence_level, enc, &i, &j);
		encode_uint8(data->finger_detected_status, enc, &i, &j);
		encode_uint32(data->temperature, enc, &i, &j);

	}

	if (data->field_hdr & FRAME_FIELD_SYNC) {
		encode_uint8(data->sync, enc, &i, &j);
	}

	if (bytes_parity(enc, i + 1)) {
		enc[0] |= FRAME_CTRL_PARITY;
	}

	return i + 1;
}

/**
 * @brief Decodes a delta-encoded RestEaZe data frame.
 * @param data_0 Data frame from the prior time step.
 * @param data_1 Data frame from the current time step.
 * @param dec Delta-encoded data frame.
 * @return Length of the delta-encoded frame in bytes.
 * @warning enc must be at least FRAME_MTU in length.
 */
size_t encode_delta_frame(const data_frame_t *data_checkpoint,
		const data_frame_t *data, uint8_t *enc) {
	/* The sequence number of data_0 must be immediately prior (without overflow).
	 */
	assert(data_checkpoint->seq == data->seq - 1 || data_checkpoint->seq == UINT32_MAX && data->seq == 0);

#ifndef NDEBUG
	/* The fields present in the new frame must be a subset of the fields in the
	 * prior. */
	size_t i;

	for (i = 0; i < 8; i++) {
		if (((data->field_hdr >> (7 - i)) & 0x01) &&
				!((data_checkpoint->field_hdr >> (7 - i)) & 0x01)) {
			assert(false);
		}
	}
#endif

	/*
	 * Hack to handle negative delta encoding of time_msec
	 */

	uint32_t delta_sec = data->time_sec - data_checkpoint->time_sec;
	int32_t delta_msec = data->time_msec - data_checkpoint->time_msec;
	while(delta_msec < 0){
		delta_msec += 1000;
		delta_sec -= 1;
	}

	data_frame_t delta = {
			.sensor_status = data->sensor_status,
			.battery_status = data->battery_status,
			.field_hdr = data->field_hdr,
			.seq = data->seq,
			.session = data->session,
//			.time_sec = data->time_sec - data_checkpoint->time_sec,
//			.time_msec = data->time_msec - data_checkpoint->time_msec,
			.time_sec = delta_sec,
			.time_msec = (uint32_t)delta_msec,
			.last_tick_usec = data->last_tick_usec - data_checkpoint->last_tick_usec,
			.fuel = data->fuel - data_checkpoint->fuel,
			.cap = {
					data->cap[0] - data_checkpoint->cap[0],
					data->cap[1] - data_checkpoint->cap[1],
					data->cap[2] - data_checkpoint->cap[2] },
			.accel = {
					data->accel[0] - data_checkpoint->accel[0],
					data->accel[1] - data_checkpoint->accel[1],
					data->accel[2] - data_checkpoint->accel[2] },
			.gyro = {
					data->gyro[0] - data_checkpoint->gyro[0],
					data->gyro[1] - data_checkpoint->gyro[1],
					data->gyro[2] - data_checkpoint->gyro[2] },
//			.mag = {
//					data->mag[0] - data_checkpoint->mag[0],
//					data->mag[1] - data_checkpoint->mag[1],
//					data->mag[2] - data_checkpoint->mag[2] },

			/* Sentinel Fields */
			.led_ir = data->led_ir - data_checkpoint->led_ir,
			.led_red = data->led_red - data_checkpoint->led_red,
			.led_green = data->led_green - data_checkpoint->led_green,
			.heart_rate = data->heart_rate - data_checkpoint->heart_rate,
			.blood_oxygen_level = data->blood_oxygen_level - data_checkpoint->blood_oxygen_level,
			.confidence_level = data->confidence_level - data_checkpoint->confidence_level,
			.finger_detected_status = data->finger_detected_status - data_checkpoint->finger_detected_status,
			.temperature = data->temperature - data_checkpoint->temperature,
			.sync = data->sync - data_checkpoint->sync, };

	size_t len = encode_data_frame(&delta, enc);

	/* Set the delta bitfield */
	enc[0] |= FRAME_CTRL_DATA_DELTA;

	/* Flip the parity bit (to account for the delta bitfield being set) */
	enc[0] ^= FRAME_CTRL_PARITY;

	return len;
}

/**
 * @brief Encodes a RestEaZe command frame.
 * @param cmd Source command frame.
 * @param enc Encoded command frame.
 * @return Length of the encoded frame in bytes.
 * @warning enc must be at least FRAME_CMD_MTU in size.
 */
size_t encode_command_frame(const command_frame_t *cmd, uint8_t *enc) {
	size_t i = 0; /* Byte index */
	size_t j = 0; /* Bit index */

	/* Frame header */
	enc[i] = FRAME_TYPE_CMD;
	enc[i] |= cmd->cmd & FRAME_CTRL_CMD;
	i++;

	enc[i] = cmd->var;
	i++;

	encode_uint32(cmd->seq, enc, &i, &j);
	encode_uint32(cmd->val, enc, &i, &j);

	if (bytes_parity(enc, i + 1)) {
		enc[0] |= FRAME_CTRL_PARITY;
	}

	return i + 1;
}

/**
 * @brief Encodes a RestEaZe response frame.
 * @param cmd Source response frame.
 * @param enc Encoded response frame.
 * @return Length of the encoded frame in bytes.
 * @warning enc must be at least FRAME_CMD_MTU in size.
 */
size_t encode_response_frame(const response_frame_t *resp, uint8_t *enc) {
	size_t i = 0; /* Byte index */
	size_t j = 0; /* Bit index */

	/* Frame header */
	enc[i] = FRAME_TYPE_RESP;
	enc[i] |= resp->ret & FRAME_CTRL_RESP;
	i++;

	enc[i] = resp->var;
	i++;

	encode_uint32(resp->seq, enc, &i, &j);
	encode_uint32(resp->val, enc, &i, &j);

	if (bytes_parity(enc, i + 1)) {
		enc[0] |= FRAME_CTRL_PARITY;
	}

	return i + 1;
}
