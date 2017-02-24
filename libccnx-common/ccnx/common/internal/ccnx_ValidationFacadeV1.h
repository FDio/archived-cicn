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
 * @file ccnx_ValidationFacadeV1.h
 * @brief Generic functions to fetch/set the KeyId, PublicKey, Certificate, or validation payload
 *
 * The Validation Facade may be used directly on CCNxInterest or CCNxContentObject structures, for example:
 *
 * @code
 * {
 *    CCNxName *name = ccnxName_CreateFromCString("lci:/foo");
 *    CCNxContentObject *object = ccnxContentObject_CreateWithNameAndPayload(name, NULL);
 *    ccnxName_Release(&name);
 *    // generate the KeyId
 *    ccnxValidationFacadeV1_SetKeyId(object, keyId);
 * }
 * @endcode
 *
 */

#ifndef libccnx_ccnx_ValidationFacadeV1_h
#define libccnx_ccnx_ValidationFacadeV1_h

#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/ccnx_KeyLocator.h>

#include <ccnx/common/ccnx_Link.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <parc/security/parc_CryptoSuite.h>


// ===========================================================
// Validation Algorithm

#include <ccnx/common/validation/ccnxValidation_CRC32C.h>
#include <ccnx/common/validation/ccnxValidation_EcSecp256K1.h>
#include <ccnx/common/validation/ccnxValidation_HmacSha256.h>
#include <ccnx/common/validation/ccnxValidation_RsaSha256.h>

// ===========================================================
// Getters

/**
 * If the Validation Algorithm has a KeyId field, return it if it exists
 *
 * Not all validation algorithms have a KeyId field.  Only true signature algoritms, such as
 * RSA or ECC should always have one.  HMAC or other MACs often use the KeyId to identify a
 * key agreed to via a key exchange protocol, so the meaning is only applicable to those parties.
 * Integrity checks, such as CRC-32C, do not have a KeyId.
 *
 * @param [in] message An allocated dictionary
 *
 * @retval non-null The KeyId field of the Validation Algorithm
 * @retval null KeyId is missing
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    PARCBuffer *keyid = ccnxValidationFacadeV1_GetKeyId(dictionary);
 * }
 * @endcode
 */
PARCBuffer *ccnxValidationFacadeV1_GetKeyId(const CCNxTlvDictionary *message);

/**
 * If the Validation Algorithm has a KeyName, return the embedded Link
 *
 * The returned CCNxLink is allocated on function call, so caller must
 * release it when done.
 *
 * @param [in] message An allocated dictionary
 *
 * @retval non-null The KeyName link of the Validation Algorithm (must be released)
 * @retval null KeyName is missing
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    CCNxLink *link = ccnxValidationFacadeV1_GetKeyName(dictionary);
 * }
 * @endcode
 */
CCNxLink *ccnxValidationFacadeV1_GetKeyName(const CCNxTlvDictionary *mesage);

/**
 * If the Validation Algorithm has a Public Key embedded, return it
 *
 * @param [in] message An allocated dictionary
 *
 * @retval non-null The KeyId field of the Validation Algorithm
 * @retval null KeyId is missing
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    PARCBuffer *publicKeyDEREncoding = ccnxValidationFacadeV1_GetPublicKey(dictionary);
 * }
 * @endcode
 */
PARCBuffer *ccnxValidationFacadeV1_GetPublicKey(const CCNxTlvDictionary *message);

/**
 * If the Validation Algorithm has a Certificate embedded, return it
 *
 * @param [in] message An allocated dictionary
 *
 * @retval non-null The Certificate field of the Validation Algorithm
 * @retval null KeyId is missing
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    PARCBuffer *x509certDEREncoding = ccnxValidationFacadeV1_GetCertificate(dictionary);
 * }
 * @endcode
 */
PARCBuffer *ccnxValidationFacadeV1_GetCertificate(const CCNxTlvDictionary *message);

/**
 * Returns the Validation Payload, if present.
 *
 * The validation payload is the actual bytes of the signature or authentication code or
 * integrity check.  it's format will be specific to the ValidationAlgorithm.
 *
 * @param [in] message An allocated dictionary
 *
 * @retval non-null The validation payload
 * @retval null Validation payload does not exist
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    PARCBuffer *validationPayload = ccnxValidationFacadeV1_GetPayload(dictionary);
 * }
 * @endcode
 */
PARCBuffer *ccnxValidationFacadeV1_GetPayload(const CCNxTlvDictionary *message);

/**
 * Determines if the packet specified a supported crypto suite
 *
 * @param [in] message An allocated dictionary
 *
 * @retval true There is a valid crypto suite specified
 * @retval false There is not a valid crypto suite specified
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    if (ccnxValidationFacadeV1_HasCryptoSuite(dictionary)) {
 *      PARCCryptoSuite suite = ccnxValidationFacadeV1_GetCryptoSuite(dictionary);
 *      // process the validation alg
 *    }
 * }
 * @endcode
 */
bool ccnxValidationFacadeV1_HasCryptoSuite(const CCNxTlvDictionary *message);

/**
 * Returns the Validation Algorithm specified in the packet
 *
 * If the packet specified a supported crypto suite, return the suite.  If
 * it is not a supported algorithm, function all assert an error.  You must use
 * ccnxValidationFacadeV1_HasCryptoSuite() to determine if there is a supported crypto suite.
 *
 * An unsupported crypto suite's entire TLV container is put in the CustomField section of
 * the dictionary and you can retrieve it from there if you know the type or iterate the list.
 *
 * @param [in] message An allocated dictionary
 *
 * @return value The crypto suite
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    if (ccnxValidationFacadeV1_HasCryptoSuite(dictionary)) {
 *      PARCCryptoSuite suite = ccnxValidationFacadeV1_GetCryptoSuite(dictionary);
 *      // process the validation alg
 *    }
 * }
 * @endcode
 */
PARCCryptoSuite ccnxValidationFacadeV1_GetCryptoSuite(const CCNxTlvDictionary *message);

/**
 * Determines if the packet specified a signing time
 *
 * @param [in] message An allocated dictionary
 *
 * @retval true There is a signing time in the validation algorithm
 * @retval false There is not a signing time
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    if (ccnxValidationFacadeV1_HasSigningTime(dictionary)) {
 *      uint64_t signingTime = ccnxValidationFacadeV1_GetSigningTime(dictionary);
 *      // process the signing time
 *    }
 * }
 * @endcode
 */
bool ccnxValidationFacadeV1_HasSigningTime(const CCNxTlvDictionary *message);

/**
 * Retruns the Validation Algorithm specified in the packet
 *
 * If the packet has a Signing Time in the Validation Algorithm, return that value.  If
 * it is not, function all assert an error.  You must use
 * ccnxValidationFacadeV1_HasSigningTime() to determine if there is a
 * signing time.
 *
 * The signing time is UTC milli-seconds since the epoch.
 *
 * @param [in] message An allocated dictionary
 *
 * @return value The signing time in UTC milli-seconds since the epoch.
 *
 * Example:
 * @code
 * {
 *    PARCBuffer *wireFormat = // packet received from the network
 *    CCNxTlvDictionary *dictionary = ccnxCodecTlvPacket_Decode(wireFormat);
 *    if (ccnxValidationFacadeV1_HasSigningTime(dictionary)) {
 *      uint64_t signingTime = ccnxValidationFacadeV1_GetSigningTime(dictionary);
 *      // process the signing time
 *    }
 * }
 * @endcode
 */
uint64_t ccnxValidationFacadeV1_GetSigningTime(const CCNxTlvDictionary *message);

// ===========================================================
// Setters

/**
 * Sets the KeyId attribute of the dictionary
 *
 * The KeyId is a mandatory field for validation algorithms that use a key, such
 * as HMAC or RSA or ECC.
 *
 * Normally, one uses a function in the ccnxValidation algoirthm to set this value
 * such as bool ccnxValidationHmacSha256_Set(CCNxTlvDictionary *message, const PARCBuffer *keyid).
 *
 * @param [in] message The dictionary to set the value in
 * @param [in] keyid The encoded value of the keyid
 *
 * @retval true Value set in the dictionary
 * @retval false Error, likely the value was already set
 *
 * Example:
 * @code
 * {
 *    CCNxName *name = ccnxName_CreateFromCString("lci:/foo");
 *    CCNxContentObject *object = ccnxContentObject_CreateWithNameAndPayload(name, NULL);
 *    ccnxName_Release(&name);
 *
 *    PARCBuffer *secretKey = parcBuffer_Wrap("password", 8, 0, 8);
 *    PARCSigner *signer = ccnxValidationHmacSha256_CreateSigner(secretKey);
 *
 *    PARCKeyId *keyid = parcSigner_CreateKeyId(signer);
 *    const PARCBuffer *keyIdBytes = parcKeyId_GetKeyId(keyid);
 *    ccnxValidationFacadeV1_SetKeyId(object, keyIdBytes);
 *    parcKeyId_Release(&keyid);
 *
 *    // continue with signer and dictionary
 * }
 * @endcode
 */
bool ccnxValidationFacadeV1_SetKeyId(CCNxTlvDictionary *message, const PARCBuffer *keyid);


/**
 * Stores the KeyLocator in the standard Dictionary places for use by the standard Getters
 *
 * If the Validator does not need any special handing of a KeyLocator, this function
 * will store the keylocator in the default dictionary entries for use by the standard getters.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxValidationFacadeV1_SetKeyLocator(CCNxTlvDictionary *message, CCNxKeyLocator *keyLocator);

/**
 * Stores the KeyName in the standard Dictionary places for use by the standard Getters
 *
 * Stores the fields from the Link in the dictionary.
 *
 * @param [in] message The message to update
 * @param [in] keyNameLink The link to put in the dictionary
 *
 * @retval true Values successfully added to dictionary
 * @retval false An error (likely a duplicate value)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxValidationFacadeV1_SetKeyName(CCNxTlvDictionary *message, const CCNxLink *keyNameLink);

/**
 * Embeds the DER encoded public key in the Validation Algorithm
 *
 * If this field is not set, the public key will not be placed in the
 * validation algorithm.
 *
 * @param [in] message The message to update
 * @param [in] derEncodedKey The DER encoded public key to put in the dictionary
 *
 * @retval true Value successfully added to dictionary
 * @retval false An error (likely a duplicate value)
 *
 * Example:
 * @code
 * {
 *    CCNxName *name = ccnxName_CreateFromCString("lci:/foo");
 *    CCNxContentObject *object = ccnxContentObject_CreateWithNameAndPayload(name, NULL);
 *    ccnxName_Release(&name);
 *
 *    PARCSigner *signer = parcSigner_Create(parcPublicKeySignerPkcs12Store_Open("keystore.p12", "password", PARCCryptoHashType_SHA256));
 *
 *    PARCBuffer *publicKey = parcSigner_GetDEREncodedPublicKey(signer);
 *
 *    ccnxValidationFacadeV1_SetPublicKey(object, publicKey);
 *
 *    // continue with signer and dictionary
 * } * @endcode
 */
bool ccnxValidationFacadeV1_SetPublicKey(CCNxTlvDictionary *message, const PARCBuffer *derEncodedKey);

/**
 * Embeds the DER encoded public key in the Validation Algorithm
 *
 * If this field is not set, the public key will not be placed in the
 * validation algorithm.
 *
 * @param [in] message The message to update
 * @param [in] derEncodedCertificate The DER encoded public key to put in the dictionary
 *
 * @retval true Value successfully added to dictionary
 * @retval false An error (likely a duplicate value)
 *
 * Example:
 * @code
 * {
 *    CCNxName *name = ccnxName_CreateFromCString("lci:/foo");
 *    CCNxContentObject *object = ccnxContentObject_CreateWithNameAndPayload(name, NULL);
 *    ccnxName_Release(&name);
 *
 *    PARCSigner *signer = parcSigner_Create(parcPublicKeySignerPkcs12Store_Open("keystore.p12", "password", PARCCryptoHashType_SHA256));
 *
 *    PARCBuffer *cert = parcSigner_GetDEREncodedCertificate(signer);
 *
 *    ccnxValidationFacadeV1_SetPublicKey(object, cert);
 *
 *    // continue with signer and dictionary
 * } * @endcode
 */
bool ccnxValidationFacadeV1_SetCertificate(CCNxTlvDictionary *message, const PARCBuffer *derEncodedCertificate);

/**
 * Sets the crypto suite in the dictionary
 *
 * It is not necessary to set the crypto suite if the packet is encoded with a Signer,
 * as the crypto suite will be derived from the signer.
 *
 * Normally, one uses a function in the ccnxValidation algoirthm to set this value
 * such as bool ccnxValidationHmacSha256_Set(CCNxTlvDictionary *message, const PARCBuffer *keyid).
 *
 * @param [in] message The message to update
 * @param [in] suite The cryptosuite value to set in the packet
 *
 * @retval true The crypto suite was set
 * @retval false The crypto suite was not set (not supported by V1 or other error)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxValidationFacadeV1_SetCryptoSuite(CCNxTlvDictionary *message, PARCCryptoSuite suite);

/**
 * Sets the signing time in the dictionary
 *
 * The signing time represents when the signature is created.  It is sometimes used for
 * replay attack prevention.  It is the UTC time in milli-seconds since the epoch (i.e. a posix time).
 *
 * If the signing time is not specified in the dictionary, it will be automatically created
 * based on the system clock when the signature is generated (this assumes the system clock
 * and timezone are correctly set).
 *
 * @param [in] message The message to update
 * @param [in] signingTime UTC time since the epoch in milli-seconds
 *
 * @retval true The value was set
 * @retval false The value was not set (likely a duplicate value)
 *
 * Example:
 * @code
 * @endcode
 */
bool ccnxValidationFacadeV1_SetSigningTime(CCNxTlvDictionary *message, uint64_t signingTime);

/**
 * Saves the validation payload in the dictionary
 *
 * The validation payload is the output of the validation algorithm, i.e.
 * the 32-bit CRC32C checksum or the RSA signature.  Usually the Codec is setting this
 * value after it has generated the wire format.  If this value is set prior to encoding,
 * this value will be used regardless if it is correct or not.
 *
 * You can only save the payload once per dictionary.
 *
 * @param [in] message The message to update
 * @param [in] validationPayload The payload to put in the packet
 *
 * @return true Payload saved
 * @return false Payload was already set or other error
 *
 * Example:
 * @code
 * {
 *    PARCSignature *signature = ccnxCodecTlvEncoder_ComputeSignature(encoder);
 *    PARCBuffer *sigbits = parcSignature_GetSignature(signature);
 *
 *    // this creates its own reference to sigbits
 *    ccnxValidationFacadeV1_SetPayload(packetDictionary, sigbits);
 *
 *    parcSignature_Release(&signature);
 *    parcBuffer_Release(&sigbits);
 * }
 * @endcode
 */
bool ccnxValidationFacadeV1_SetPayload(CCNxTlvDictionary *message, const PARCBuffer *validationPayload);

#endif // libccnx_ccnx_ValidationFacadeV1_h
