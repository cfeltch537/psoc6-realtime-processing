/**
 * Copyright (c) 2019 Tanzen Medical, Inc.
 *
 * @file encode.h
 * @author Nicholas Haltmeyer <hanicho1@umbc.edu>
 * @date 13 Jan 2019
 * @brief Routines for encoding RestEaZe frames.
 */

#ifndef _ENCODE_H
#define _ENCODE_H

#include "frame.h"

/**
 * @brief Encodes a Calendar struct as a Unix timestamp.
 * @param cal_0 Calendar time.
 * @return Seconds since 1 Jan 1970.
 */
//uint32_t encode_cal(const Calendar *cal_0);

/**
 * @brief Encodes a frame encoded with consistent overhead byte stuffing.
 * @param dec Source decoded frame.
 * @param enc Destination encoded frame.
 * @param len Length of the source frame in bytes.
 * @return The length of the encoded frame in bytes.
 * @see https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */
size_t encode_cobs(const uint8_t *dec, uint8_t *enc, size_t len);

/**
 * @brief Encodes a RestEaZe data frame.
 * @param data Source data frame.
 * @param enc Encoded data frame.
 * @return Length of the encoded frame in bytes.
 * @warning enc must be at least FRAME_DATA_MTU in size.
 */
size_t encode_data_frame(const data_frame_t *data, uint8_t *enc);

/**
 * @brief Encodes a delta-encoded RestEaZe data frame.
 * @param data_0 Data frame from the prior time step.
 * @param data_1 Data frame from the current time step.
 * @param dec Delta-encoded data frame.
 * @return Length of the delta-encoded frame in bytes.
 * @warning enc must be at least FRAME_DATA_MTU in size.
 */
size_t encode_delta_frame(const data_frame_t *data_0,
		const data_frame_t *data_1, uint8_t *enc);

/**
 * @brief Encodes a RestEaZe command frame.
 * @param cmd Source command frame.
 * @param enc Encoded command frame.
 * @return Length of the encoded frame in bytes.
 * @warning enc must be at least FRAME_CMD_MTU in size.
 */
size_t encode_command_frame(const command_frame_t *cmd, uint8_t *enc);

/**
 * @brief Encodes a RestEaZe response frame.
 * @param cmd Source response frame.
 * @param enc Encoded response frame.
 * @return Length of the encoded frame in bytes.
 * @warning enc must be at least FRAME_CMD_MTU in size.
 */
size_t encode_response_frame(const response_frame_t *resp, uint8_t *enc);

#endif /* _ENCODE_H */
