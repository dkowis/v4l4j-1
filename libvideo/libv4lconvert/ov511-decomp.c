/* We would like to embed this inside libv4l, but we cannot as I've failed
   to contact Mark W. McClelland to get permission to relicense this,
   so this lives in an external (GPL licensed) helper */

/* OV511 Decompression Support Module
 *
 * Copyright (c) 1999-2003 Mark W. McClelland. All rights reserved.
 * http://alpha.dyndns.org/ov511/
 *
 * Original decompression code Copyright 1998-2000 OmniVision Technologies
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 */

#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "helper-funcs.h"

/******************************************************************************
 * Decompression Functions
 ******************************************************************************/

static void DecompressYHI(u8 *pIn,
		u8 *pOut,
		int           *iIn,	/* in/out */
		int           *iOut,	/* in/out */
		const int      w,
		const int      YUVFlag) {
	short ZigZag[64];
	int temp[64];
	int Zcnt_Flag = 0;
	int Num8_Flag = 0;
	int in_pos = *iIn;
	int out_pos = *iOut;
	int tmp, tmp1, tmp2, tmp3;
	u8 header, ZTable[64];
	short tmpl, tmph, half_byte, idx, count;
	unsigned long ZigZag_length = 0, ZT_length, i, j;
	short DeZigZag[64];

	const short a = 11584;
	const short b = 16068;
	const short c = 15136;
	const short d = 13624;
	const short e =  9104;
	const short f =  6270;
	const short g =  3196;

	int out_idx;

	/* Take off every 'Zig' */
	for (i = 0; i < 64; i++)
		ZigZag[i] = 0;

	/*****************************
	 * Read in the Y header byte *
	 *****************************/

	header = pIn[in_pos];
	in_pos++;

	ZigZag_length = header & 0x3f;
	ZigZag_length = ZigZag_length + 1;

	Num8_Flag = header & 0x40;
	Zcnt_Flag = header & 0x80;

	/*************************
	 * Read in the Y content *
	 *************************/

	if (Zcnt_Flag == 0) {    /* Without Zero Table read contents directly */
		/* Read in ZigZag[0] */
		ZigZag[0] = pIn[in_pos++];
		tmpl = pIn[in_pos++];
		tmph = tmpl << 8;
		ZigZag[0] = ZigZag[0] | tmph;
		ZigZag[0] = ZigZag[0] << 4;
		ZigZag[0] = ZigZag[0] >> 4;

		if (Num8_Flag) { /* 8 Bits */
			for (i = 1; i < ZigZag_length; i++) {
				ZigZag[i] = pIn[in_pos++];
				ZigZag[i] = ZigZag[i] << 8;
				ZigZag[i] = ZigZag[i] >> 8;
			}
		} else {   /* 12 bits and has no Zero Table */
			idx = 1;
			half_byte = 0;
			for (i = 1; i < ZigZag_length; i++) {
				if (half_byte == 0) {
					ZigZag[i] = pIn[in_pos++];
					tmpl = pIn[in_pos++];
					tmph = tmpl << 8;
					tmph = tmph & 0x0f00;
					ZigZag[i] = ZigZag[i] | tmph;
					ZigZag[i] = ZigZag[i] << 4;
					ZigZag[i] = ZigZag[i] >> 4;
					half_byte = 1;
				} else {
					ZigZag[i] = pIn[in_pos++];
					ZigZag[i] = ZigZag[i] << 8;
					tmpl = tmpl & 0x00f0;
					ZigZag[i] = ZigZag[i] | tmpl;
					ZigZag[i] = ZigZag[i] >> 4;
					half_byte = 0;
				}
			}
		}
	} else {  /* Has Zero Table */
		/* Calculate Z-Table length */
		ZT_length = ZigZag_length / 8;
		tmp = ZigZag_length % 8;

		if (tmp > 0)
			ZT_length = ZT_length + 1;

		/* Read in Zero Table */
		for (j = 0; j < ZT_length; j++)
			ZTable[j] = pIn[in_pos++];

		/* Read in ZigZag[0] */
		ZigZag[0] = pIn[in_pos++];
		tmpl = pIn[in_pos++];
		tmph = tmpl << 8;
		ZigZag[0] = ZigZag[0] | tmph;
		ZigZag[0] = ZigZag[0] << 4;
		ZigZag[0] = ZigZag[0] >> 4;

		/* Decode ZigZag */
		idx = 0;
		ZTable[idx] = ZTable[idx] << 1;
		count = 7;

		if (Num8_Flag) {	/* 8 Bits and has zero table */
			for (i = 1; i < ZigZag_length; i++) {
				if ((ZTable[idx] & 0x80)) {
					ZigZag[i] = pIn[in_pos++];
					ZigZag[i] = ZigZag[i] << 8;
					ZigZag[i] = ZigZag[i] >> 8;
				}

				ZTable[idx] = ZTable[idx]<<1;
				count--;
				if (count == 0)	{
					count = 8;
					idx++;
				}
			}
		} else {	/* 12 bits and has Zero Table */
			half_byte = 0;
			for (i = 1; i < ZigZag_length; i++) {
				if (ZTable[idx] & 0x80) {
					if (half_byte == 0) {
						ZigZag[i] = pIn[in_pos++];
						tmpl = pIn[in_pos++];
						tmph = tmpl << 8;
						tmph = tmph & 0x0f00;
						ZigZag[i] = ZigZag[i] | tmph;
						ZigZag[i] = ZigZag[i] << 4;
						ZigZag[i] = ZigZag[i] >> 4;
						half_byte = 1;
					} else {
						ZigZag[i] = pIn[in_pos++];
						ZigZag[i] = ZigZag[i] << 8;
						tmpl = tmpl & 0x00f0;
						ZigZag[i] = ZigZag[i] | tmpl;
						ZigZag[i] = ZigZag[i] >> 4;
						half_byte = 0;
					}
				}

				ZTable[idx] = ZTable[idx] << 1;
				count--;
				if (count == 0)	{
					count = 8;
					idx++;
				}
			}
		}
	}

	/*************
	 * De-ZigZag *
	 *************/

	for (j = 0; j < 64; j++)
		DeZigZag[j] = 0;

	if (YUVFlag == 1) {
		DeZigZag[0] = ZigZag[0];
		DeZigZag[1] = ZigZag[1] << 1;
		DeZigZag[2] = ZigZag[5] << 1;
		DeZigZag[3] = ZigZag[6] << 2;

		DeZigZag[8] = ZigZag[2] << 1;
		DeZigZag[9] = ZigZag[4] << 1;
		DeZigZag[10] = ZigZag[7] << 1;
		DeZigZag[11] = ZigZag[13] << 2;

		DeZigZag[16] = ZigZag[3] << 1;
		DeZigZag[17] = ZigZag[8] << 1;
		DeZigZag[18] = ZigZag[12] << 2;
		DeZigZag[19] = ZigZag[17] << 2;

		DeZigZag[24] = ZigZag[9] << 2;
		DeZigZag[25] = ZigZag[11] << 2;
		DeZigZag[26] = ZigZag[18] << 2;
		DeZigZag[27] = ZigZag[24] << 3;
	} else {
		DeZigZag[0] = ZigZag[0];
		DeZigZag[1] = ZigZag[1] << 2;
		DeZigZag[2] = ZigZag[5] << 2;
		DeZigZag[3] = ZigZag[6] << 3;

		DeZigZag[8] = ZigZag[2] << 2;
		DeZigZag[9] = ZigZag[4] << 2;
		DeZigZag[10] = ZigZag[7] << 2;
		DeZigZag[11] = ZigZag[13] << 4;

		DeZigZag[16] = ZigZag[3] << 2;
		DeZigZag[17] = ZigZag[8] << 2;
		DeZigZag[18] = ZigZag[12] << 3;
		DeZigZag[19] = ZigZag[17] << 4;

		DeZigZag[24] = ZigZag[9] << 3;
		DeZigZag[25] = ZigZag[11] << 4;
		DeZigZag[26] = ZigZag[18] << 4;
		DeZigZag[27] = ZigZag[24] << 4;
	}

	/*****************
	 **** IDCT 1D ****
	 *****************/

#define IDCT_1D(c0, c1, c2, c3, in)					    \
	do {								    \
		tmp1 = ((c0) * DeZigZag[in]) + ((c2) * DeZigZag[(in) + 2]); \
		tmp2 = (c1) * DeZigZag[(in) + 1];			    \
		tmp3 = (c3) * DeZigZag[(in) + 3];			    \
	} while (0)

#define COMPOSE_1(out1, out2)			\
	do {					\
		tmp = tmp1 + tmp2 + tmp3;	\
		temp[out1] = tmp >> 15;		\
		tmp = tmp1 - tmp2 - tmp3;	\
		temp[out2] = tmp >> 15;		\
	} while (0)

#define COMPOSE_2(out1, out2)			\
	do {					\
		tmp = tmp1 + tmp2 - tmp3;	\
		temp[out1] = tmp >> 15;		\
		tmp = tmp1 - tmp2 + tmp3;	\
		temp[out2] = tmp >> 15;		\
	} while (0)

	/* j = 0 */
	IDCT_1D(a, b,  c, d,  0); COMPOSE_1(0, 56);
	IDCT_1D(a, b,  c, d,  8); COMPOSE_1(1, 57);
	IDCT_1D(a, b,  c, d, 16); COMPOSE_1(2, 58);
	IDCT_1D(a, b,  c, d, 24); COMPOSE_1(3, 59);

	/* j = 1 */
	IDCT_1D(a, d,  f, g,  0); COMPOSE_2(8, 48);
	IDCT_1D(a, d,  f, g,  8); COMPOSE_2(9, 49);
	IDCT_1D(a, d,  f, g, 16); COMPOSE_2(10, 50);
	IDCT_1D(a, d,  f, g, 24); COMPOSE_2(11, 51);

	/* j = 2 */
	IDCT_1D(a, e, -f, b,  0); COMPOSE_2(16, 40);
	IDCT_1D(a, e, -f, b,  8); COMPOSE_2(17, 41);
	IDCT_1D(a, e, -f, b, 16); COMPOSE_2(18, 42);
	IDCT_1D(a, e, -f, b, 24); COMPOSE_2(19, 43);

	/* j = 3 */
	IDCT_1D(a, g, -c, e,  0); COMPOSE_2(24, 32);
	IDCT_1D(a, g, -c, e,  8); COMPOSE_2(25, 33);
	IDCT_1D(a, g, -c, e, 16); COMPOSE_2(26, 34);
	IDCT_1D(a, g, -c, e, 24); COMPOSE_2(27, 35);

#undef IDCT_1D
#undef COMPOSE_1
#undef COMPOSE_2

	/*****************
	 **** IDCT 2D ****
	 *****************/

#define IDCT_2D(c0, c1, c2, c3, in)					\
	do {								\
		tmp = temp[in] * (c0) + temp[(in) + 1] * (c1)		\
		+ temp[(in) + 2] * (c2) + temp[(in) + 3] * (c3);	\
	} while (0)

#define STORE(i)				\
	do {					\
		tmp = tmp >> 15;		\
		tmp = tmp + 128;		\
		if (tmp > 255)			\
			tmp = 255;		\
		if (tmp < 0)			\
			tmp = 0;		\
		pOut[i] = (u8)tmp;	\
	} while (0)

#define IDCT_2D_ROW(in)						\
	do {							\
		IDCT_2D(a,  b,  c,  d, in); STORE(0 + out_idx);	\
		IDCT_2D(a,  d,  f, -g, in); STORE(1 + out_idx);	\
		IDCT_2D(a,  e, -f, -b, in); STORE(2 + out_idx);	\
		IDCT_2D(a,  g, -c, -e, in); STORE(3 + out_idx);	\
		IDCT_2D(a, -g, -c,  e, in); STORE(4 + out_idx);	\
		IDCT_2D(a, -e, -f,  b, in); STORE(5 + out_idx);	\
		IDCT_2D(a, -d,  f,  g, in); STORE(6 + out_idx);	\
		IDCT_2D(a, -b,  c, -d, in); STORE(7 + out_idx);	\
	} while (0)


#define IDCT_2D_FAST(c0, c1, c2, c3, in)				\
	do {								\
		tmp1 = ((c0) * temp[in]) + ((c2) * temp[(in) + 2]);	\
		tmp2 = (c1) * temp[(in) + 1];				\
		tmp3 = (c3) * temp[(in) + 3];				\
	} while (0)

#define STORE_FAST_1(out1, out2)				\
	do {							\
		tmp = tmp1 + tmp2 + tmp3;			\
		STORE((out1) + out_idx);			\
		tmp = tmp1 - tmp2 - tmp3;			\
		STORE((out2) + out_idx);			\
	} while (0)

#define STORE_FAST_2(out1, out2)				\
	do {							\
		tmp = tmp1 + tmp2 - tmp3;			\
		STORE((out1) + out_idx);			\
		tmp = tmp1 - tmp2 + tmp3;			\
		STORE((out2) + out_idx);			\
	} while (0)

#define IDCT_2D_FAST_ROW(in)						\
	do {								\
		IDCT_2D_FAST(a, b,  c, d, in);	STORE_FAST_1(0, 7);	\
		IDCT_2D_FAST(a, d,  f, g, in);	STORE_FAST_2(1, 6);	\
		IDCT_2D_FAST(a, e, -f, b, in);	STORE_FAST_2(2, 5);	\
		IDCT_2D_FAST(a, g, -c, e, in);	STORE_FAST_2(3, 4);	\
	} while (0)

	out_idx = out_pos;

	IDCT_2D_ROW(0);		out_idx += w;
	IDCT_2D_ROW(8);		out_idx += w;
	IDCT_2D_ROW(16);	out_idx += w;
	IDCT_2D_ROW(24);	out_idx += w;
	IDCT_2D_ROW(32);	out_idx += w;
	IDCT_2D_ROW(40);	out_idx += w;
	IDCT_2D_FAST_ROW(48);	out_idx += w;
	IDCT_2D_FAST_ROW(56);

	*iIn = in_pos;
	*iOut = out_pos + 8;
}

#define DECOMP_Y() DecompressYHI(pIn, pY, &iIn, &iY, w, 1)
#define DECOMP_U() DecompressYHI(pIn, pU, &iIn, &iU, w / 2, 2)
#define DECOMP_V() DecompressYHI(pIn, pV, &iIn, &iV, w / 2, 2)

#if 0
static inline int Decompress400HiNoMMX(u8 *pIn, u8 *pOut, const unsigned int w, const unsigned int h, const int inSize) {
	u8 *pY = pOut;

	int iIn = 0;
	for (unsigned int y = 0; y < h; y += 8) {
		int iY = w * y;

		for (unsigned x = 0; x < w; x += 8)
			DECOMP_Y();
	}

	return 0;
}
#endif

static inline int Decompress420HiNoMMX(u8 *pIn, u8 *pOut, const unsigned int w, const unsigned int h) {
	u8 *pY = pOut;
	u8 *pU = pY + w * h;
	u8 *pV = pU + w * h / 4;
	int iIn = 0, iY = 0, iU = 0, iV = 0;
	unsigned int xY = 0, xUV = 0;
	const unsigned int nBlocks = (w * h) / (32 * 8);
	for (unsigned int count = 0; count < nBlocks; count++) {
		DECOMP_U();
		DECOMP_V();	xUV += 16;
		if (xUV >= w) {
			iU += (w * 7) / 2;
			iV += (w * 7) / 2;
			xUV = 0;
		}

		DECOMP_Y();	xY += 8;
		DECOMP_Y();	xY += 8;
		if (xY >= w) {
			iY += w * 7;
			xY = 0;
		}
		DECOMP_Y();	xY += 8;
		DECOMP_Y();	xY += 8;
		if (xY >= w) {
			iY += w * 7;
			xY = 0;
		}
	}

	return 0;
}

/* Copies a 64-byte segment at pIn to an 8x8 block at pOut. The width of the
 * image at pOut is specified by w.
 */
static inline void make_8x8(u8 *pIn, u8 *pOut, unsigned int w) {
	for (unsigned int y = 0; y < 8; y++) {
		u8* pOut1 = pOut;
		for (unsigned x = 0; x < 8; x++)
			*pOut1++ = *pIn++;
		pOut += w;
	}
}

#if 0
/*
 * For RAW BW (YUV 4:0:0) images, data show up in 256 byte segments.
 * The segments represent 4 squares of 8x8 pixels as follows:
 *
 *      0  1 ...  7    64  65 ...  71   ...  192 193 ... 199
 *      8  9 ... 15    72  73 ...  79        200 201 ... 207
 *           ...              ...                    ...
 *     56 57 ... 63   120 121 ... 127        248 249 ... 255
 *
 */
static void yuv400raw_to_yuv400p(struct ov511_frame *frame, u8 *pIn0, u8 *pOut0) {

	/* Copy Y */
	u8* pIn = pIn0;
	u8* pOutLine = pOut0;
	for (unsigned int y = 0; y + 1 < frame->rawheight; y += 8) {
		u8* pOut = pOutLine;
		for (unsigned int x = 0; x < frame->rawwidth - 1; x += 8) {
			make_8x8(pIn, pOut, frame->rawwidth);
			pIn += 64;
			pOut += 8;
		}
		pOutLine += 8 * frame->rawwidth;
	}
}
#endif

/*
 * For YUV 4:2:0 images, the data show up in 384 byte segments.
 * The first 64 bytes of each segment are U, the next 64 are V.  The U and
 * V are arranged as follows:
 *
 *      0  1 ...  7
 *      8  9 ... 15
 *           ...
 *     56 57 ... 63
 *
 * U and V are shipped at half resolution (1 U,V sample -> one 2x2 block).
 *
 * The next 256 bytes are full resolution Y data and represent 4 squares
 * of 8x8 pixels as follows:
 *
 *      0  1 ...  7    64  65 ...  71   ...  192 193 ... 199
 *      8  9 ... 15    72  73 ...  79        200 201 ... 207
 *           ...              ...                    ...
 *     56 57 ... 63   120 121 ... 127   ...  248 249 ... 255
 *
 * Note that the U and V data in one segment represent a 16 x 16 pixel
 * area, but the Y data represent a 32 x 8 pixel area. If the width is not an
 * even multiple of 32, the extra 8x8 blocks within a 32x8 block belong to the
 * next horizontal stripe.
 *
 * If dumppix module param is set, _parse_data just dumps the incoming segments,
 * verbatim, in order, into the frame. When used with vidcat -f ppm -s 640x480
 * this puts the data on the standard output and can be analyzed with the
 * parseppm.c utility I wrote.  That's a much faster way for figuring out how
 * these data are scrambled.
 */

/* Converts from raw, uncompressed segments at pIn0 to a YUV420P frame at pOut0.
 *
 * FIXME: Currently only handles width and height that are multiples of 16
 */
static void yuv420raw_to_yuv420p(u8 *pIn0, u8 *pOut0, u32 width, u32 height) {
	int k;
	u8 *pIn, *pOut, *pOutLine;
	const unsigned int a = width * height;
	const unsigned int w = width / 2;

	/* Copy U and V */
	pIn = pIn0;
	pOutLine = pOut0 + a;
	for (unsigned int y = 0; y + 1 < height; y += 16) {
		pOut = pOutLine;
		for (unsigned int x = 0; x + 1< width; x += 16) {
			make_8x8(pIn, pOut, w);
			make_8x8(pIn + 64, pOut + a / 4, w);
			pIn += 384;
			pOut += 8;
		}
		pOutLine += 8 * w;
	}

	/* Copy Y */
	pIn = pIn0 + 128;
	pOutLine = pOut0;
	k = 0;
	for (unsigned int y = 0; y + 1 < height; y += 8) {
		pOut = pOutLine;
		for (unsigned int x = 0; x + 1 < width; x += 8) {
			make_8x8(pIn, pOut, width);
			pIn += 64;
			pOut += 8;
			if ((++k) > 3) {
				k = 0;
				pIn += 128;
			}
		}
		pOutLine += 8 * width;
	}
}


/* Remove all 0 blocks from input */
static void remove0blocks(u8 *pIn, int *inSize) {
	long long *in = (long long *)pIn;
	long long *out = (long long *)pIn;
	
	for (int i = 0; i < *inSize; i += 32, in += 4) {
		bool all_zero = true;
		for (unsigned int j = 0; j < 4; j++)
			if (in[j]) {
				all_zero = false;
				break;
			}
		
		/* Skip 32 byte blocks of all 0 */
		if (all_zero)
			continue;
		
		for (unsigned int j = 0; j < 4; j++)
			*out++ = in[j];
	}
	
	*inSize -= (in - out) * 8;
}

static int v4lconvert_ov511_to_yuv420(u8 *src, u8 *dest, unsigned int w, unsigned int h, unsigned int src_size) {
	src_size -= 11; /* Remove footer */

	remove0blocks(src, &src_size);

	/* Compressed ? */
	if (src[8] & 0x40)
		return Decompress420HiNoMMX(src + 9, dest, w, h);
	return 0;
}

int main(int argc, char *argv[]) {
	u32 width, height, yvu, src_size, dest_size;
	u8 src_buf[500000];
	u8 dest_buf[500000];
	
	if (argc < 1) {
		fprintf(stderr, "1 argument required");
		return 2;
	}

	while (1) {
		if (v4lconvert_helper_read(STDIN_FILENO, &width, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (v4lconvert_helper_read(STDIN_FILENO, &height, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (v4lconvert_helper_read(STDIN_FILENO, &yvu, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (v4lconvert_helper_read(STDIN_FILENO, &src_size, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (src_size > sizeof(src_buf)) {
			fprintf(stderr, "%s: error: src_buf too small, need: %d\n", argv[0], src_size);
			return 2;
		}

		if (v4lconvert_helper_read(STDIN_FILENO, src_buf, src_size, argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */


		dest_size = width * height * 3 / 2;
		if (dest_size > sizeof(dest_buf)) {
			fprintf(stderr, "%s: error: dest_buf too small, need: %d\n", argv[0], dest_size);
			dest_size = (unsigned) -1;
		} else if (v4lconvert_ov511_to_yuv420(src_buf, dest_buf, width, height, src_size))
			dest_size = (unsigned) -1;

		if (v4lconvert_helper_write(STDOUT_FILENO, &dest_size, sizeof(int), argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */

		if (dest_size == (unsigned) -1)
			continue;

		if (v4lconvert_helper_write(STDOUT_FILENO, dest_buf, dest_size, argv[0]))
			return 1; /* Erm, no way to recover without loosing sync with libv4l */
	}
}
