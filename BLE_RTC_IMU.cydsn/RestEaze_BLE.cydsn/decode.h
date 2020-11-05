/**
 * Copyright (c) 2019 Tanzen Medical, Inc.
 *
 * @file decode.h
 * @author Nicholas Haltmeyer <hanicho1@umbc.edu>
 * @date 13 Jan 2019
 * @brief Routines for decoding RestEaZe frames.
 */

#ifndef _DECODE_H
#define _DECODE_H

#include "frame.h"

/**
 * @brief Decodes a Unix timestamp as a Calendar struct.
 * @param ts_0 Unix timestamp.
 * @return Recovered Calendar struct.
 */
//Calendar decode_cal(const uint32_t *ts_0);

/**
 * @brief Decodes a frame encoded with consistent overhead byte stuffing.
 * @param enc Source encoded frame.
 * @param dec Destination decoded frame.
 * @param len Length of the source frame in bytes.
 * @return The length of the decoded frame in bytes.
 * @see https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */
size_t decode_cobs(const uint8_t *enc, uint8_t *dec, size_t len);

/**
 * @brief Decodes a RestEaZe data frame.
 * @param data Source data frame.
 * @param dec Decoded data frame.
 * @return EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise.
 */
int decode_data_frame(const uint8_t *data, data_frame_t *dec);

/**
 * @brief Decodes a delta-encoded RestEaZe data frame.
 * @param data_0 Data frame from the prior time step.
 * @param data_1 Encoded data frame from the current time step.
 * @param dec Decoded delta-encoded data frame.
 * @return EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise.
 */
int decode_delta_frame(const data_frame_t *data_0, const uint8_t *data_1,
		data_frame_t *dec);

/**
 * @brief Decodes a RestEaZe command frame.
 * @param cmd Source command frame.
 * @param dec Decoded command frame.
 * @return EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise.
 */
int decode_command_frame(const uint8_t *cmd, command_frame_t *dec);

/**
 * @brief Decodes a RestEaZe response frame.
 * @param cmd Source response frame.
 * @param dec Decoded response frame.
 * @return EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise.
 */
int decode_response_frame(const uint8_t *resp, response_frame_t *dec);

#endif /* _DECODE_H */
