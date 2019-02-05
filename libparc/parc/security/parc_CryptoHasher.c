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

/**
 * Implements SHA256, and SHA512 with OpenSSL or Apple's CommonCrypto, depending on platform.
 * It all follows the OpenSSL calling conventions:
 *    CTX_*    is the digest's context
 *    INIT_*   initializes the digest
 *    UPDATE_* updates the digest given a buffer
 *    FINAL_*  finalizes the digest, returning the digest
 *    LENGTH_* is the byte length of the digest.
 *
 */

#include <config.h>
#include <stdio.h>

#include <parc/security/parc_CryptoHasher.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_Memory.h>
#include <parc/assert/parc_Assert.h>
#include <parc/algol/parc_Object.h>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>

#define CTX_SHA256    CC_SHA256_CTX
#define INIT_SHA256   CC_SHA256_Init
#define UPDATE_SHA256 CC_SHA256_Update
#define FINAL_SHA256  CC_SHA256_Final
#define LENGTH_SHA256 CC_SHA256_DIGEST_LENGTH

#define CTX_SHA512    CC_SHA512_CTX
#define INIT_SHA512   CC_SHA512_Init
#define UPDATE_SHA512 CC_SHA512_Update
#define FINAL_SHA512  CC_SHA512_Final
#define LENGTH_SHA512 CC_SHA512_DIGEST_LENGTH

#else
#include <openssl/sha.h>
#define CTX_SHA256    SHA256_CTX
#define INIT_SHA256   SHA256_Init
#define UPDATE_SHA256 SHA256_Update
#define FINAL_SHA256  SHA256_Final
#define LENGTH_SHA256 SHA256_DIGEST_LENGTH

#define CTX_SHA512    SHA512_CTX
#define INIT_SHA512   SHA512_Init
#define UPDATE_SHA512 SHA512_Update
#define FINAL_SHA512  SHA512_Final
#define LENGTH_SHA512 SHA512_DIGEST_LENGTH
#endif

// -------------------------------------------------------------
// function prototypes for use in the function structures
static void *_sha256_create(void *env);
static int _sha256_init(void *ctx);
static int _sha256_update(void *ctx, const void *buffer, size_t length);
static PARCBuffer *_sha256_finalize(void *ctx);
static void _sha256_destroy(void **ctxPtr);

static void *_sha512_create(void *env);
static int _sha512_init(void *ctx);
static int _sha512_update(void *ctx, const void *buffer, size_t length);
static PARCBuffer *_sha512_finalize(void *ctx);
static void _sha512_destroy(void **ctxPtr);

static void *_crc32_create(void *env);
static int _crc32_init(void *ctx);
static int _crc32_update(void *ctx, const void *buffer, size_t length);
static PARCBuffer *_crc32_finalize(void *ctx);
static void _crc32_destroy(void **ctxPtr);
// -------------------------------------------------------------

// These are templates for the environments.  SHA256 and SHA512 do not
// have a functor_env, all the state is carried in the setup context.

static PARCCryptoHasherInterface functor_sha256 = {
    .functor_env     = NULL,
    .hasher_setup    = _sha256_create,
    .hasher_init     = _sha256_init,
    .hasher_update   = _sha256_update,
    .hasher_finalize = _sha256_finalize,
    .hasher_destroy  = _sha256_destroy
};

static PARCCryptoHasherInterface functor_sha512 = {
    .functor_env     = NULL,
    .hasher_setup    = _sha512_create,
    .hasher_init     = _sha512_init,
    .hasher_update   = _sha512_update,
    .hasher_finalize = _sha512_finalize,
    .hasher_destroy  = _sha512_destroy
};

static PARCCryptoHasherInterface functor_crc32 = {
    .functor_env     = NULL,
    .hasher_setup    = _crc32_create,
    .hasher_init     = _crc32_init,
    .hasher_update   = _crc32_update,
    .hasher_finalize = _crc32_finalize,
    .hasher_destroy  = _crc32_destroy
};

struct parc_crypto_hasher {
    PARCCryptoHashType type;
    PARCCryptoHasherInterface functor;
    void *hasher_ctx;
};

static void
_parcCryptoHasher_FinalRelease(PARCCryptoHasher **hasherP)
{
    PARCCryptoHasher *hasher = (PARCCryptoHasher *) *hasherP;
    hasher->functor.hasher_destroy(&(hasher->hasher_ctx));
}

parcObject_ExtendPARCObject(PARCCryptoHasher, _parcCryptoHasher_FinalRelease, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(parcCryptoHasher, PARCCryptoHasher);

parcObject_ImplementRelease(parcCryptoHasher, PARCCryptoHasher);

PARCCryptoHasher *
parcCryptoHasher_Create(PARCCryptoHashType type)
{
    PARCCryptoHasher *hasher = parcObject_CreateInstance(PARCCryptoHasher);
    parcAssertNotNull(hasher, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCCryptoHasher));

    hasher->type = type;

    switch (type) {
        case PARCCryptoHashType_SHA256:
            hasher->functor = functor_sha256;
            break;

        case PARCCryptoHashType_SHA512:
            hasher->functor = functor_sha512;
            break;

        case PARCCryptoHashType_CRC32C:
            hasher->functor = functor_crc32;
            break;

        default:
            parcMemory_Deallocate((void **) &hasher);
            parcTrapIllegalValue(type, "Unknown hasher type: %d", type);
    }

    hasher->hasher_ctx = hasher->functor.hasher_setup(hasher->functor.functor_env);
    return hasher;
}

PARCCryptoHasher *
parcCryptoHasher_CustomHasher(PARCCryptoHashType type, PARCCryptoHasherInterface functor)
{
    PARCCryptoHasher *hasher = parcObject_CreateInstance(PARCCryptoHasher);
    parcAssertNotNull(hasher, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCCryptoHasher));
    hasher->type = type;
    hasher->functor = functor;
    hasher->hasher_ctx = hasher->functor.hasher_setup(hasher->functor.functor_env);
    return hasher;
}

/**
 * Reset the internal state of the digest to start a new session
 * Returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
parcCryptoHasher_Init(PARCCryptoHasher *digester)
{
    parcAssertNotNull(digester, "Parameter must be non-null");

    int success = digester->functor.hasher_init(digester->hasher_ctx);
    return (success == 1) ? 0 : -1;
}

int
parcCryptoHasher_UpdateBytes(PARCCryptoHasher *digester, const void *buffer, size_t length)
{
    parcAssertNotNull(digester, "Parameter must be non-null");
    int success = digester->functor.hasher_update(digester->hasher_ctx, buffer, length);
    return (success == 1) ? 0 : -1;
}

int
parcCryptoHasher_UpdateBuffer(PARCCryptoHasher *digester, const PARCBuffer *buffer)
{
    parcAssertNotNull(digester, "Parameter must be non-null");
    PARCBuffer *buf = parcBuffer_Slice(buffer);
    size_t length = parcBuffer_Limit(buf);
    void *byteArray = parcBuffer_Overlay(buf, length);
    int success = digester->functor.hasher_update(digester->hasher_ctx, byteArray, length);

    parcBuffer_Release(&buf);
    return (success == 1) ? 0 : -1;
}

PARCCryptoHash *
parcCryptoHasher_Finalize(PARCCryptoHasher *digester)
{
    parcAssertNotNull(digester, "Parameter must be non-null");
    PARCBuffer *digestBuffer = digester->functor.hasher_finalize(digester->hasher_ctx);

    if (parcBuffer_Position(digestBuffer) != 0) {
        parcBuffer_Flip(digestBuffer);
    }

    PARCCryptoHash *parcDigest = parcCryptoHash_Create(digester->type, digestBuffer);

    parcBuffer_Release(&digestBuffer);
    return parcDigest;
}

// ===============================================

static void *
_sha256_create(void *dummy)
{
    void *ctx = parcMemory_AllocateAndClear(sizeof(CTX_SHA256));
    return ctx;
}

static int
_sha256_init(void *ctx)
{
    return INIT_SHA256(ctx);
}

static int
_sha256_update(void *ctx, const void *buffer, size_t length)
{
    return UPDATE_SHA256(ctx, buffer, (unsigned) length);
}

static PARCBuffer *
_sha256_finalize(void *ctx)
{
    uint8_t buffer[LENGTH_SHA256];
    FINAL_SHA256(buffer, ctx);

    PARCBuffer *output = parcBuffer_Allocate(LENGTH_SHA256);
    parcBuffer_PutArray(output, LENGTH_SHA256, buffer);

    return output;
}

static void
_sha256_destroy(void **ctxPtr)
{
    parcMemory_Deallocate((void **) ctxPtr);
    *ctxPtr = NULL;
}

// ===============================================

static void *
_sha512_create(void *dummy)
{
    void *ctx = parcMemory_AllocateAndClear(sizeof(CTX_SHA512));
    return ctx;
}

static int
_sha512_init(void *ctx)
{
    return INIT_SHA512(ctx);
}

static int
_sha512_update(void *ctx, const void *buffer, size_t length)
{
    return UPDATE_SHA512(ctx, buffer, (unsigned) length);
}

static PARCBuffer *
_sha512_finalize(void *ctx)
{
    uint8_t buffer[LENGTH_SHA512];
    FINAL_SHA512(buffer, ctx);

    PARCBuffer *output = parcBuffer_Allocate(LENGTH_SHA512);
    parcBuffer_PutArray(output, LENGTH_SHA512, buffer);

    return output;
}

static void
_sha512_destroy(void **ctxPtr)
{
    parcMemory_Deallocate((void **) ctxPtr);
    *ctxPtr = NULL;
}

// ==================================================
// CRC32C Implementation PARCCryptoHasher

typedef struct crc32c_state {
    uint32_t crc32;
} _CRC32CState;

// =====================================
// Hardware calculation

#ifdef __SSE4_2__
#include <nmmintrin.h>

#ifdef __x86_64__
// The length rounded to 8-bytes
#define CRC_ROUNDING_MASK 0xFFFFFFFFFFFFFFF8ULL
#define LARGEST_CRC_INTRINSIC _mm_crc32_u64
#define CRC_CAST_TYPE uint64_t
#else
// The length rounded to 4-bytes
#define CRC_ROUNDING_MASK 0xFFFFFFFCUL
#define LARGEST_CRC_INTRINSIC _mm_crc32_u32
#define CRC_CAST_TYPE uint32_t
#endif //__x86_64__

static uint32_t
_crc32c_UpdateIntel(uint32_t crc, size_t len, uint8_t p[len])
{
    size_t blocks = len & CRC_ROUNDING_MASK;
    size_t offset = 0;

    while (offset < blocks) {
        crc = (uint32_t) LARGEST_CRC_INTRINSIC((CRC_CAST_TYPE) crc, *(CRC_CAST_TYPE *) &p[offset]);
        offset += sizeof(CRC_CAST_TYPE);
    }

    // now do the last bytes if it was not 8-byte aligned
    size_t position = blocks;
    while (position < len) {
        crc = _mm_crc32_u8((uint32_t) crc, p[position]);
        position++;
    }

    return crc;
}
#endif

// =====================================
// Software calculation

// Table generated from CRC Calculator http://sourceforge.net/projects/crccalculator/files/CRC/
// The table is for bit-reversed bytes

static const uint32_t _crc32c_table[] = {
    0x00000000, 0xF26B8303, 0xE13B70F7, 0x1350F3F4,
    0xC79A971F, 0x35F1141C, 0x26A1E7E8, 0xD4CA64EB,
    0x8AD958CF, 0x78B2DBCC, 0x6BE22838, 0x9989AB3B,
    0x4D43CFD0, 0xBF284CD3, 0xAC78BF27, 0x5E133C24,
    0x105EC76F, 0xE235446C, 0xF165B798, 0x030E349B,
    0xD7C45070, 0x25AFD373, 0x36FF2087, 0xC494A384,
    0x9A879FA0, 0x68EC1CA3, 0x7BBCEF57, 0x89D76C54,
    0x5D1D08BF, 0xAF768BBC, 0xBC267848, 0x4E4DFB4B,
    0x20BD8EDE, 0xD2D60DDD, 0xC186FE29, 0x33ED7D2A,
    0xE72719C1, 0x154C9AC2, 0x061C6936, 0xF477EA35,
    0xAA64D611, 0x580F5512, 0x4B5FA6E6, 0xB93425E5,
    0x6DFE410E, 0x9F95C20D, 0x8CC531F9, 0x7EAEB2FA,
    0x30E349B1, 0xC288CAB2, 0xD1D83946, 0x23B3BA45,
    0xF779DEAE, 0x05125DAD, 0x1642AE59, 0xE4292D5A,
    0xBA3A117E, 0x4851927D, 0x5B016189, 0xA96AE28A,
    0x7DA08661, 0x8FCB0562, 0x9C9BF696, 0x6EF07595,
    0x417B1DBC, 0xB3109EBF, 0xA0406D4B, 0x522BEE48,
    0x86E18AA3, 0x748A09A0, 0x67DAFA54, 0x95B17957,
    0xCBA24573, 0x39C9C670, 0x2A993584, 0xD8F2B687,
    0x0C38D26C, 0xFE53516F, 0xED03A29B, 0x1F682198,
    0x5125DAD3, 0xA34E59D0, 0xB01EAA24, 0x42752927,
    0x96BF4DCC, 0x64D4CECF, 0x77843D3B, 0x85EFBE38,
    0xDBFC821C, 0x2997011F, 0x3AC7F2EB, 0xC8AC71E8,
    0x1C661503, 0xEE0D9600, 0xFD5D65F4, 0x0F36E6F7,
    0x61C69362, 0x93AD1061, 0x80FDE395, 0x72966096,
    0xA65C047D, 0x5437877E, 0x4767748A, 0xB50CF789,
    0xEB1FCBAD, 0x197448AE, 0x0A24BB5A, 0xF84F3859,
    0x2C855CB2, 0xDEEEDFB1, 0xCDBE2C45, 0x3FD5AF46,
    0x7198540D, 0x83F3D70E, 0x90A324FA, 0x62C8A7F9,
    0xB602C312, 0x44694011, 0x5739B3E5, 0xA55230E6,
    0xFB410CC2, 0x092A8FC1, 0x1A7A7C35, 0xE811FF36,
    0x3CDB9BDD, 0xCEB018DE, 0xDDE0EB2A, 0x2F8B6829,
    0x82F63B78, 0x709DB87B, 0x63CD4B8F, 0x91A6C88C,
    0x456CAC67, 0xB7072F64, 0xA457DC90, 0x563C5F93,
    0x082F63B7, 0xFA44E0B4, 0xE9141340, 0x1B7F9043,
    0xCFB5F4A8, 0x3DDE77AB, 0x2E8E845F, 0xDCE5075C,
    0x92A8FC17, 0x60C37F14, 0x73938CE0, 0x81F80FE3,
    0x55326B08, 0xA759E80B, 0xB4091BFF, 0x466298FC,
    0x1871A4D8, 0xEA1A27DB, 0xF94AD42F, 0x0B21572C,
    0xDFEB33C7, 0x2D80B0C4, 0x3ED04330, 0xCCBBC033,
    0xA24BB5A6, 0x502036A5, 0x4370C551, 0xB11B4652,
    0x65D122B9, 0x97BAA1BA, 0x84EA524E, 0x7681D14D,
    0x2892ED69, 0xDAF96E6A, 0xC9A99D9E, 0x3BC21E9D,
    0xEF087A76, 0x1D63F975, 0x0E330A81, 0xFC588982,
    0xB21572C9, 0x407EF1CA, 0x532E023E, 0xA145813D,
    0x758FE5D6, 0x87E466D5, 0x94B49521, 0x66DF1622,
    0x38CC2A06, 0xCAA7A905, 0xD9F75AF1, 0x2B9CD9F2,
    0xFF56BD19, 0x0D3D3E1A, 0x1E6DCDEE, 0xEC064EED,
    0xC38D26C4, 0x31E6A5C7, 0x22B65633, 0xD0DDD530,
    0x0417B1DB, 0xF67C32D8, 0xE52CC12C, 0x1747422F,
    0x49547E0B, 0xBB3FFD08, 0xA86F0EFC, 0x5A048DFF,
    0x8ECEE914, 0x7CA56A17, 0x6FF599E3, 0x9D9E1AE0,
    0xD3D3E1AB, 0x21B862A8, 0x32E8915C, 0xC083125F,
    0x144976B4, 0xE622F5B7, 0xF5720643, 0x07198540,
    0x590AB964, 0xAB613A67, 0xB831C993, 0x4A5A4A90,
    0x9E902E7B, 0x6CFBAD78, 0x7FAB5E8C, 0x8DC0DD8F,
    0xE330A81A, 0x115B2B19, 0x020BD8ED, 0xF0605BEE,
    0x24AA3F05, 0xD6C1BC06, 0xC5914FF2, 0x37FACCF1,
    0x69E9F0D5, 0x9B8273D6, 0x88D28022, 0x7AB90321,
    0xAE7367CA, 0x5C18E4C9, 0x4F48173D, 0xBD23943E,
    0xF36E6F75, 0x0105EC76, 0x12551F82, 0xE03E9C81,
    0x34F4F86A, 0xC69F7B69, 0xD5CF889D, 0x27A40B9E,
    0x79B737BA, 0x8BDCB4B9, 0x988C474D, 0x6AE7C44E,
    0xBE2DA0A5, 0x4C4623A6, 0x5F16D052, 0xAD7D5351
};

/*
 * If we use the hardware implementation, this function may be unused, but
 * we keep it around for unit testing
 */
__attribute__((unused))
static uint32_t
_crc32c_UpdateSoftware(uint32_t crc, size_t len, uint8_t *p)
{
    for (int i = 0; i < len; i++) {
        crc = (crc >> 8) ^ _crc32c_table[((uint8_t) (crc & 0xFF)) ^ p[i]];
    }

    return crc;
}

/**
 * Initializes the CRC32C value (init to 0xFFFFFFFF)
 */
static uint32_t
_crc32c_Init(void)
{
    return ~0;
}


/**
 * Finalizes the CRC32 (xor with 0xFFFFFFFF)
 */
static uint32_t
_crc32c_Finalize(uint32_t crc)
{
    return crc ^ ~0;
}

/**
 * Updates the CRC32 value with a byte array.  Does
 * bit mirroring to match either the Intel instruction set or
 * the CRC table used by the software calculation.
 */
static uint32_t
_crc32c_Update(uint32_t crc, size_t len, uint8_t *p)
{
#ifdef __SSE4_2__
    crc = _crc32c_UpdateIntel(crc, len, p);
#else
    crc = _crc32c_UpdateSoftware(crc, len, p);
#endif
    return crc;
}

/*
 * Creates a new context variable for starting a hash
 */
static void *
_crc32_create(void *env __attribute__ ((unused)))
{
    _CRC32CState *ctx = parcMemory_AllocateAndClear(sizeof(_CRC32CState));
    parcAssertNotNull(ctx, "parcMemory_AllocateAndClear(%zu) returned NULL for _CRC32CState", sizeof(_CRC32CState));

    // Now initialize it with our digest and key, so in hmac_init we can avoid using those
    return ctx;
}

static int
_crc32_init(void *ctx)
{
    _CRC32CState *state = ctx;

    // initialize the CRC32C with all 1's
    state->crc32 = _crc32c_Init();

    return 0;
}

static int
_crc32_update(void *ctx, const void *buffer, size_t length)
{
    _CRC32CState *state = ctx;
    state->crc32 = _crc32c_Update(state->crc32, length, (uint8_t *) buffer);
    return 0;
}

static PARCBuffer *
_crc32_finalize(void *ctx)
{
    _CRC32CState *state = ctx;
    state->crc32 = _crc32c_Finalize(state->crc32);
    PARCBuffer *crcDigest = parcBuffer_Allocate(sizeof(uint32_t));
    parcBuffer_PutUint32(crcDigest, state->crc32);
    return parcBuffer_Flip(crcDigest);
}

static void
_crc32_destroy(void **ctxPtr)
{
    _CRC32CState *state = *ctxPtr;
    parcMemory_Deallocate((void **) &state);
    *ctxPtr = NULL;
}
