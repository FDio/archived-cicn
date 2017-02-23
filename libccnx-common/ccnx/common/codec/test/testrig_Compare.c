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
 * Utilities used by the Schema unit tests to compare buffers
 *
 */

/**
 * Compares an encoding buffer to linear memory
 *
 * Will assert if the memory does not compare.  The encoding buffer will be finalized.
 * Will assert if the encoder has an error.
 *
 * @param [in] encoder The encoding buffer to compare
 * @param [in] length The length of linear memory
 * @param [in] memory The "truth" memory to compare against
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
testCompareEncoderToLinearMemory(CCNxCodecTlvEncoder *encoder, size_t length, uint8_t memory[length])
{
    assertFalse(ccnxCodecTlvEncoder_HasError(encoder), "Encoder has error")
    {
        printf("ERROR: %s\n", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));
    }

    PARCBuffer *truth = parcBuffer_Wrap(memory, length, 0, length);
    ccnxCodecTlvEncoder_Finalize(encoder);
    PARCBuffer *buffer = ccnxCodecTlvEncoder_CreateBuffer(encoder);

    assertTrue(parcBuffer_Equals(buffer, truth), "buffers not equal")
    {
        printf("Expected\n");
        parcBuffer_Display(truth, 3);

        printf("Got this\n");
        parcBuffer_Display(buffer, 3);
    }

    parcBuffer_Release(&truth);
    parcBuffer_Release(&buffer);
}

/**
 * Compares an encoding buffer to a PARCBuffer
 *
 * Will assert if the memory does not compare.  The encoding buffer will be finalized.
 * Will assert if the encoder has an error.
 *
 * @param [in] encoder The encoding buffer to compare
 * @param [in] buffer The buffer to compare to, must be setup to be read (i.e. flipped)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
testCompareEncoderToBuffer(CCNxCodecTlvEncoder *encoder, PARCBuffer *buffer)
{
    assertFalse(ccnxCodecTlvEncoder_HasError(encoder), "Encoder has error")
    {
        printf("ERROR: %s\n", ccnxCodecError_ToString(ccnxCodecTlvEncoder_GetError(encoder)));
    }


    uint8_t *linearMemory = parcByteArray_Array(parcBuffer_Array(buffer));
    size_t offset = parcBuffer_ArrayOffset(buffer) + parcBuffer_Position(buffer);
    size_t length = parcBuffer_Remaining(buffer);

    testCompareEncoderToLinearMemory(encoder, length, linearMemory + offset);
}

