/**
 * Copyright (c) 2019 Tanzen Medical, Inc.
 *
 * @file frame.c
 * @author Nicholas Haltmeyer <hanicho1@umbc.edu>
 * @date 13 Jan 2019
 * @brief Common definitions for RestEaZe frames.
 */

#include "frame.h"

/**
 * @brief Computes the parity bit for a byte array.
 * @param enc Encoded frame.
 * @param len Number of bytes in the frame.
 * @return Parity
 * @warning When computing the parity of an encoded frame, the Parity bitfield
 * *will* change the result if set.
 * @see http://www.graphics.stanford.edu/~seander/bithacks.html
 */
bool bytes_parity(const uint8_t *enc, size_t len) {
#define P2(n) (n), (n) ^ 1, (n) ^ 1, (n)
#define P4(n) P2((n)), P2((n) ^ 1), P2((n) ^ 1), P2((n))
#define P6(n) P4((n)), P4((n) ^ 1), P4((n) ^ 1), P4((n))

	const bool ParityTable256[256] = { P6(0), P6(1), P6(1), P6(0) };

#undef P2
#undef P4
#undef P6

	size_t i;
	uint8_t xor = enc[0];

	for (i = 1; i < len; i++) {
	xor ^= enc[i];
}

return ParityTable256[xor];
}
