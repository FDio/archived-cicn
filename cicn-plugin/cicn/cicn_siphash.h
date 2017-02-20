/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
   SipHash reference C implementation

   Copyright (c) 2012-2014 Jean-Philippe Aumasson
   <jeanphilippe.aumasson@gmail.com>
   Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>

   To the extent possible under law, the author(s) have dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along
   with
   this software. If not, see
   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#ifndef _CICN_SIPHASH_H_
#define _CICN_SIPHASH_H_ 1

#if !CICN_VPP_PLUGIN
#error "cicn-internal file included externally"
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* default: SipHash-2-4 */
#define cROUNDS 2
#define dROUNDS 4

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)                                                        \
  (p)[0] = (uint8_t)((v));                                                     \
  (p)[1] = (uint8_t)((v) >> 8);                                                \
  (p)[2] = (uint8_t)((v) >> 16);                                               \
  (p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)                                                        \
  U32TO8_LE((p), (uint32_t)((v)));                                             \
  U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

#define U8TO64_LE(p)                                                           \
  (((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) |                          \
   ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) |                   \
   ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) |                   \
   ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56))

#define SIPROUND                                                               \
  do {                                                                         \
    v0 += v1;                                                                  \
    v1 = ROTL(v1, 13);                                                         \
    v1 ^= v0;                                                                  \
    v0 = ROTL(v0, 32);                                                         \
    v2 += v3;                                                                  \
    v3 = ROTL(v3, 16);                                                         \
    v3 ^= v2;                                                                  \
    v0 += v3;                                                                  \
    v3 = ROTL(v3, 21);                                                         \
    v3 ^= v0;                                                                  \
    v2 += v1;                                                                  \
    v1 = ROTL(v1, 17);                                                         \
    v1 ^= v2;                                                                  \
    v2 = ROTL(v2, 32);                                                         \
  } while (0)

#ifdef CICN_SIPHASH_DEBUG
#define SIPTRACE                                                               \
  do {                                                                         \
    printf("(%3d) v0 %08x %08x\n", (int)inlen, (uint32_t)(v0 >> 32),           \
           (uint32_t)v0);                                                      \
    printf("(%3d) v1 %08x %08x\n", (int)inlen, (uint32_t)(v1 >> 32),           \
           (uint32_t)v1);                                                      \
    printf("(%3d) v2 %08x %08x\n", (int)inlen, (uint32_t)(v2 >> 32),           \
           (uint32_t)v2);                                                      \
    printf("(%3d) v3 %08x %08x\n", (int)inlen, (uint32_t)(v3 >> 32),           \
           (uint32_t)v3);                                                      \
  } while (0)
#else
#define SIPTRACE
#endif

#ifdef CICN_SIPHASH_128		// exp. 128-bit code below, ifdef'd out, for reference
#error "cicn_siphash doesn't support 128-bit yet!"
#endif

/* Cool - need an extern declaration in order to keep llvm happy... */
extern inline uint64_t
cicn_siphash (const uint8_t * in, uint64_t inlen, const uint8_t * k);

/*
 *
 */
inline uint64_t
cicn_siphash (const uint8_t * in, uint64_t inlen, const uint8_t * k)
{
  /* "somepseudorandomlygeneratedbytes" */
  uint64_t v0 = 0x736f6d6570736575ULL;
  uint64_t v1 = 0x646f72616e646f6dULL;
  uint64_t v2 = 0x6c7967656e657261ULL;
  uint64_t v3 = 0x7465646279746573ULL;
  uint64_t b;
  uint64_t k0 = U8TO64_LE (k);
  uint64_t k1 = U8TO64_LE (k + 8);
  uint64_t m;
  int i;
  const uint8_t *end = in + inlen - (inlen % sizeof (uint64_t));
  const int left = inlen & 7;
  b = ((uint64_t) inlen) << 56;
  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;

#ifdef CICN_SIPHASH_128
  v1 ^= 0xee;
#endif /* CICN_SIPHASH_128 */

  for (; in != end; in += 8)
    {
      m = U8TO64_LE (in);
      v3 ^= m;

      SIPTRACE;
      for (i = 0; i < cROUNDS; ++i)
	SIPROUND;

      v0 ^= m;
    }

  switch (left)
    {
    case 7:
      b |= ((uint64_t) in[6]) << 48;
    case 6:
      b |= ((uint64_t) in[5]) << 40;
    case 5:
      b |= ((uint64_t) in[4]) << 32;
    case 4:
      b |= ((uint64_t) in[3]) << 24;
    case 3:
      b |= ((uint64_t) in[2]) << 16;
    case 2:
      b |= ((uint64_t) in[1]) << 8;
    case 1:
      b |= ((uint64_t) in[0]);
      break;
    case 0:
      break;
    }

  v3 ^= b;

  SIPTRACE;
  for (i = 0; i < cROUNDS; ++i)
    SIPROUND;

  v0 ^= b;

#ifndef CICN_SIPHASH_128
  v2 ^= 0xff;
#else
  v2 ^= 0xee;
#endif /* CICN_SIPHASH_128 */

  SIPTRACE;
  for (i = 0; i < dROUNDS; ++i)
    SIPROUND;

  return (v0 ^ v1 ^ v2 ^ v3);

/*  U64TO8_LE(out, b); TODO -- ref version mails back result and returns zero */

#ifdef CICN_SIPHASH_128
  v1 ^= 0xdd;

  SIPTRACE;
  for (i = 0; i < dROUNDS; ++i)
    SIPROUND;

  b = v0 ^ v1 ^ v2 ^ v3;
  U64TO8_LE (out + 8, b);
#endif /* CICN_SIPHASH_128 */

/*  return 0;  TODO -- ref version mails back result and returns zero... */
}

/*
 * Running state of hash, for taking advantage of incremental hashing
 */
typedef struct cicn_siphash_hi_s
{
  uint64_t sip_v_whole[4];
} cicn_siphash_hi_t;

/*
 * cicn_siphash DOCUMENTATION (algorithm details)
 *
 * Sources:
 * - Analysis: http://eprint.iacr.org/2012/351.pdf
 * - Code:     https://github.com/floodyberry/siphash
 *
 * siphash has an initialization phase, a compression phase, and a
 * finalization phase.
 * - The running state of siphash is stored in a "vector": 32 bytes,
 *   managed as a 4 element array of uint64_t.
 * - The initialization phase initializes the vector ("V") for the
 *   hash calculation, based on the key and some constants
 * - The compression phase processes the string to be hashed,
 *   processing an 8 byte (64 bit) block per iteration. Each
 *   interation includes
 *   - Convert the 8 bytes into a 64-bit number (using a little-endian
 *     conversion)
 *   - XOR the new 8 bytes into V[3]
 *   - Perform multiple (2) "rounds" of compression on V, using the logic
 *     in SipRound
 *   - XOR the new 8 bytes into V[0]
 *   - The last block is special. It is created as if extra bytes were
 *     available off the end of the string.  The last block includes
 *     - leftover bytes at the tail of the string (e.g. 3 leftover bytes if
 *       the string were 11 bytes long)
 *     - nulls to fill out the tail of the string to 7 bytes (e.g. 4 nulls
 *      if the string were 11 bytes long)
 *     - The number of actual leftover bytes in the 8th byte (e.g. 3,
 *       if the string were 11 bytes long).
 *     - For another example, if the string were 8 bytes long, the last
 *       (2nd) block would be all null.
 * - The finalization phase:
 *   - XOR 0xff info V[2]
 *   - Perform multiple (4) rounds of compression on V, using the
 *     logic in SipRound (i.e. compression and finalization use the same
 *     core compression logic)
 *   - XOR the 4 elements of V together to produce the 8 byte (64 bit)
 *     hash result.
 */

const unsigned char cicn_siphash_seed[16] = {
  0x12, 0x34, 0x56, 0x78, 0x98, 0x76, 0x54, 0x32,
  0x12, 0x34, 0x56, 0x78, 0x98, 0x76, 0x54, 0x32,
};

/*
 * Copy one siphash vector to another, e.g. to as part of saving a
 * hash's intermediate result for later re-use.
 * When processing a CICN name, calculating the siphashes of
 * each component prefix plus the siphash of the whole name, this
 * is used to keep track of partial results rather than doing
 * each siphash from scratch (restarting at the beginning of the whole name).
 * (See summary at "cicn_siphash DOCUMENTATION".)
 *   Returns:
 *     No return value
 *   Vout:
 *     Output vector, target of copy
 *   Vin:
 *     Input vector, source of copy
 */
#define cicn_siphash_vec_copy(Vout, Vin) do { \
	Vout[0] = Vin[0]; Vout[1] = Vin[1]; Vout[2] = Vin[2]; Vout[3] = Vin[3];\
    } while(0);

static inline void
cicn_siphash_hi_initialize (cicn_siphash_hi_t * arg,
			    const unsigned char *seed)
{
  const unsigned char *key = seed;
  uint64_t *V = arg->sip_v_whole;
  uint64_t K[2];

  K[0] = U8TO64_LE (&key[0]);
  K[1] = U8TO64_LE (&key[8]);

  /* "somepseu""dorandom""lygenera""tedbytes" */
  V[0] = K[0] ^ 0x736f6d6570736575ull;
  V[1] = K[1] ^ 0x646f72616e646f6dull;
#ifdef CICN_SIPHASH_128
  V[1] ^= 0xee;
#endif /* CICN_SIPHASH_128 */
  V[2] = K[0] ^ 0x6c7967656e657261ull;
  V[3] = K[1] ^ 0x7465646279746573ull;
}

/*
 * The core logic of one round of siphash compression/finalization
 * (See summary at "cicn_siphash DOCUMENTATION".)
 *   V:
 *     Vector holding the current state of the hash, to be put through
 *     (the core logic of) one round of compression/finalization.
 */
#define ROTL64(x,b) ROTL(x,b)
#define cicn_siphash_Round(V) {                        \
        V[0] += V[1]; V[2] += V[3];                     \
        V[1] = ROTL64(V[1],13);	V[3] = ROTL64(V[3],16); \
        V[1] ^= V[0]; V[3] ^= V[2];                     \
        V[0] = ROTL64(V[0],32);                         \
        V[2] += V[1]; V[0] += V[3];                     \
        V[1] = ROTL64(V[1],17); V[3] = ROTL64(V[3],21); \
        V[1] ^= V[2]; V[3] ^= V[0];                     \
        V[2] = ROTL64(V[2],32); }

/*
 * The full logic of one round of siphash compression (not finalization)
 * (See summary at "cicn_siphash DOCUMENTATION".)
 */
static inline void
cicn_siphash_compress (uint64_t V[4], uint64_t block_le_val)
{
  V[3] ^= block_le_val;
  cicn_siphash_Round (V);
  cicn_siphash_Round (V);
  V[0] ^= block_le_val;
}

/*
 * At the end of a prefix/name/bytestring to be siphashed, 0-7 bytes will
 * be left that do not make up a full 8-byte block. This routine
 * convolves those 0-7 bytes with 1 byte derived from prefix overall length
 * (not the count of trailing bytes) to get a last 64-bit quantity to be
 * used in siphash finalization.
 *
 * @param[in] base is the base of the entire bytestring
 * @param[in] len is the length of the entire bytestring
 * @param[in] pblk_offset is the byte offset of the partial block (last
 *            0-7 bytes)
 *
 * This routine, similar to the original code downloaded to siphash.c,
 * is careful to not read any bytes past the end of the block
 * (at the cost of doing multiple 1-byte reads rather than a single
 * 8-byte read and mask).
 * (See summary at "cicn_siphash DOCUMENTATION".)
 */
static inline uint64_t
cicn_siphash_partial_blk_val (const unsigned char *base, int len,
			      int pblk_offset)
{
  uint64_t pblk_val_64_LE;
  int partial_bytes = (len & 0x7);

  pblk_val_64_LE = (uint64_t) (len & 0xff) << 56;
  switch (partial_bytes)
    {
    case 7:
      pblk_val_64_LE |= (uint64_t) base[pblk_offset + 6] << 48;
    case 6:
      pblk_val_64_LE |= (uint64_t) base[pblk_offset + 5] << 40;
    case 5:
      pblk_val_64_LE |= (uint64_t) base[pblk_offset + 4] << 32;
    case 4:
      pblk_val_64_LE |= (uint64_t) base[pblk_offset + 3] << 24;
    case 3:
      pblk_val_64_LE |= (uint64_t) base[pblk_offset + 2] << 16;
    case 2:
      pblk_val_64_LE |= (uint64_t) base[pblk_offset + 1] << 8;
    case 1:
      pblk_val_64_LE |= (uint64_t) base[pblk_offset + 0];
    case 0:
    default:;
    }
  return (pblk_val_64_LE);
}

/*
 * The logic for convolving the final/partial 8-byte/64-bit block into
 * the running 32-byte vector, which is then xor'd ito the 64-bit hash value.
 * (See summary at "cicn_siphash DOCUMENTATION".)
 */
static inline uint64_t
cicn_siphash_finalize (uint64_t V[4])
{
  uint64_t hash;

#ifndef CICN_SIPHASH_128
  V[2] ^= 0xff;
#else
  V[2] ^= 0xee;
#endif /* CICN_SIPHASH_128 */

  cicn_siphash_Round (V);
  cicn_siphash_Round (V);
  cicn_siphash_Round (V);
  cicn_siphash_Round (V);
  hash = V[0] ^ V[1] ^ V[2] ^ V[3];
  return (hash);

#ifdef CICN_SIPHASH_128
  V[1] ^= 0xdd;

  cicn_hfn_sip_Round (V);
  cicn_hfn_sip_Round (V);
  cicn_hfn_sip_Round (V);
  cicn_hfn_sip_Round (V);

  hash = V[0] ^ V[1] ^ V[2] ^ V[3];
  U64TO8_LE (out + 8, hash);
#endif /* CICN_SIPHASH_128 */
}

/*
 * Calculate/return 64-bit siphash of bytestring (name prefix) beginning at
 * nrec_val with length pfx_len, for which intermediate siphash
 * information through crec_offset is already stored in V_running.
 * (In other words, this optimized siphash calculation need only
 * convolve the last pfx_len-crec_offset bytes of
 * prefix into the calculation.)
 *
 * As an important side effect, V_running is updated with siphash
 * information through the final full 8-byte block in the prefix, for
 * use in calculating the siphash of the following prefix.
 *
 * (See summary at "cicn_siphash DOCUMENTATION".)
 */
static inline uint64_t
cicn_siphash_hi_calculate (cicn_siphash_hi_t * arg,
			   const unsigned char *nrec_val, int pfx_len,
			   int crec_offset)
{
  uint64_t *V_running = arg->sip_v_whole;

  uint64_t hash;
  size_t cur_crec_base_blk, next_crec_base_blk, blk;
  uint64_t V_finalize[4];

  //printf("cur_crec_base_blk: %d ", cur_crec_base_blk);

  /* blks (8 bytes) are byte offsets: they count 0,8,16... not 0,1,2... */
  cur_crec_base_blk = (crec_offset & ~7);
  next_crec_base_blk = (pfx_len & ~7);
  for (blk = cur_crec_base_blk; blk < next_crec_base_blk; blk += 8)
    {
      cicn_siphash_compress (V_running, U8TO64_LE (&nrec_val[blk]));
    }

  /* copy V to v to finalize hash calculation for this prefix */
  cicn_siphash_vec_copy (V_finalize, V_running);

  cicn_siphash_compress (V_finalize,
			 cicn_siphash_partial_blk_val (nrec_val, pfx_len,
						       blk));
  hash = cicn_siphash_finalize (V_finalize);

  return (hash);
}

#endif /* _CICN_SIPHASH_H_ */
