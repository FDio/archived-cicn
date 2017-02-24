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
 */
#include <config.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include <parc/algol/parc_Object.h>
#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_NameLabel.h>

struct ccnx_name_label {
    CCNxNameLabelType type;
    PARCBuffer *parameter;
};

static struct CCNxNameLabelMnemonic {
    const char *mnemonic;
    const CCNxNameLabelType type;
} CCNxNameLabelMnemonic[] = {
    { CCNxNameLabel_Name,              CCNxNameLabelType_NAME      },
    { CCNxNameLabel_InterestPayloadId, CCNxNameLabelType_PAYLOADID },
    { CCNxNameLabel_Serial,            CCNxNameLabelType_SERIAL    },
    { CCNxNameLabel_Chunk,             CCNxNameLabelType_CHUNK     },
    { CCNxNameLabel_ChunkMeta,         CCNxNameLabelType_CHUNKMETA },
    { CCNxNameLabel_App,               CCNxNameLabelType_APP0      },
    { CCNxNameLabel_Time,              CCNxNameLabelType_TIME      },
    { NULL,                            CCNxNameLabelType_Unknown   },
};

static bool
_ccnxNameLabel_Destructor(CCNxNameLabel **labelP)
{
    assertNotNull(labelP, "Parameter must be a non-null pointer to a CCNxNameLabels pointer.");

    CCNxNameLabel *label = *labelP;
    if (label->parameter != NULL) {
        parcBuffer_Release(&label->parameter);
    }
    return true;
}

parcObject_Override(CCNxNameLabel, PARCObject,
                    .destructor = (PARCObjectDestructor *) _ccnxNameLabel_Destructor);

parcObject_ImplementAcquire(ccnxNameLabel, CCNxNameLabel);

parcObject_ImplementRelease(ccnxNameLabel, CCNxNameLabel);

CCNxNameLabel *
ccnxNameLabel_Create(CCNxNameLabelType type, const PARCBuffer *parameter)
{
    CCNxNameLabel *result = NULL;

    if (type != CCNxNameLabelType_BADNAME && type != CCNxNameLabelType_Unknown) {
        result = parcObject_CreateInstance(CCNxNameLabel);
        if (result != NULL) {
            result->type = type;
            result->parameter = parameter == NULL ? NULL : parcBuffer_Acquire(parameter);
        }
    }

    return result;
}

CCNxNameLabelType
ccnxNameLabel_GetType(const CCNxNameLabel *label)
{
    ccnxNameLabel_OptionalAssertValid(label);

    return label->type;
}

PARCBuffer *
ccnxNameLabel_GetParameter(const CCNxNameLabel *label)
{
    ccnxNameLabel_OptionalAssertValid(label);
    return label->parameter;
}

static const char *
_ccnxNameLabelType_ToMnemonic(CCNxNameLabelType type)
{
    const char *result = NULL;

    for (struct CCNxNameLabelMnemonic *p = &CCNxNameLabelMnemonic[0]; p->mnemonic != NULL; p++) {
        if (p->type == type) {
            result = p->mnemonic;
            break;
        }
    }

    return result;
}

static CCNxNameLabelType
_ccnxNameLabelType_ResolveNumeric(PARCBuffer *label)
{
    CCNxNameLabelType result = (CCNxNameLabelType) parcBuffer_ParseNumeric(label);

    return result;
}

static CCNxNameLabelType
_ccnxNameLabelType_ResolveMnemonic(const PARCBuffer *label)
{
    CCNxNameLabelType result = CCNxNameLabelType_Unknown;

    size_t labelLength = parcBuffer_Remaining(label);
    char *labelAsBytes = parcBuffer_Overlay((PARCBuffer *) label, 0);

    for (struct CCNxNameLabelMnemonic *p = &CCNxNameLabelMnemonic[0]; p->mnemonic != NULL; p++) {
        if (strncasecmp(p->mnemonic, labelAsBytes, labelLength) == 0) {
            result = p->type;
            break;
        }
    }

    return result;
}

static CCNxNameLabelType
_ccnxNameLabelType_Resolve(PARCBuffer *label)
{
    CCNxNameLabelType result = CCNxNameLabelType_NAME;

    if (label != NULL) {
        if (parcBuffer_Remaining(label) > 0) {
            if (isdigit(parcBuffer_PeekByte(label))) {
                result = _ccnxNameLabelType_ResolveNumeric(label);
            } else {
                result = _ccnxNameLabelType_ResolveMnemonic(label);
            }
        } else {
            result = CCNxNameLabelType_NAME;
        }
    }

    return result;
}

/*
 * Parse the 'label [":" param] ' portion of an lpv-segment
 *
 *  lpv-segment      = label [":" param] "=" s-value
 *  v-segment        = s-value
 *  label            = alpha-t / num-t
 *  param            = alpha-t / num-t
 *  s-value          = *(s-pchar)
 *
 * number =
 * number
 */
CCNxNameLabel *
ccnxNameLabel_Parse(PARCBuffer *buffer)
{
    PARCBuffer *label = NULL;
    PARCBuffer *parameter = NULL;

    // When complete, the buffer's position will be set to the first byte of the value portion.
    if (parcBuffer_SkipTo(buffer, 1, (uint8_t *) "=")) {
        label = parcBuffer_Flip(parcBuffer_Duplicate(buffer));
        if (parcBuffer_Remaining(label) > 0) {
            parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) + 1);

            if (parcBuffer_SkipTo(label, 1, (uint8_t *) ":")) {
                size_t colon = parcBuffer_Position(label);
                parcBuffer_SetPosition(label, colon + 1);
                parameter = parcBuffer_Slice(label);

                parcBuffer_SetPosition(label, colon);
            }

            label = parcBuffer_Flip(label);
        } else {
            parcBuffer_Release(&label);
            return NULL;
        }
    } else {
        parcBuffer_Rewind(buffer);
    }

    CCNxNameLabelType type = CCNxNameLabelType_NAME;

    if (label != NULL) {
        type = _ccnxNameLabelType_Resolve(label);
        if (type == CCNxNameLabelType_App(0)) {
            if (parameter != NULL) {
                type = CCNxNameLabelType_App(parcBuffer_ParseNumeric(parameter));
                parcBuffer_Release(&parameter);
            }
        }
        parcBuffer_Release(&label);
    }

    CCNxNameLabel *result = ccnxNameLabel_Create(type, parameter);

    if (parameter != NULL) {
        parcBuffer_Release(&parameter);
    }

    return result;
}

bool
ccnxNameLabel_Equals(const CCNxNameLabel *x, const CCNxNameLabel *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (x->type == y->type) {
            if (parcBuffer_Equals(x->parameter, y->parameter)) {
                result = true;
            }
        }
    }

    return result;
}

CCNxNameLabel *
ccnxNameLabel_Copy(const CCNxNameLabel *label)
{
    ccnxNameLabel_OptionalAssertValid(label);

    PARCBuffer *parameter = (label->parameter == NULL) ? NULL : parcBuffer_Copy(label->parameter);
    CCNxNameLabel *result = ccnxNameLabel_Create(label->type, parameter);
    if (parameter != NULL) {
        parcBuffer_Release(&parameter);
    }

    return result;
}

bool
ccnxNameLabel_IsValid(const CCNxNameLabel *label)
{
    bool result = false;

    if (label != NULL) {
        if (label->type != CCNxNameLabelType_Unknown && label->type != CCNxNameLabelType_BADNAME) {
            if (label->parameter != NULL) {
                if (parcBuffer_IsValid(label->parameter)) {
                    result = true;
                }
            } else {
                result = true;
            }
        }
    }
    return result;
}

void
ccnxNameLabel_AssertValid(const CCNxNameLabel *label)
{
    trapIllegalValueIf(ccnxNameLabel_IsValid(label) == false, "Encountered an invalid CCNxNameLabel instance.");
}

PARCBufferComposer *
ccnxNameLabel_BuildString(const CCNxNameLabel *label, PARCBufferComposer *composer)
{
    ccnxNameLabel_OptionalAssertValid(label);

    if (label->type >= CCNxNameLabelType_App(0) && label->type <= CCNxNameLabelType_App(4096)) {
        parcBufferComposer_Format(composer, "%s:%u=", CCNxNameLabel_App, label->type - CCNxNameLabelType_App(0));
    } else {
        const char *mnemonic = _ccnxNameLabelType_ToMnemonic(label->type);
        if (mnemonic == NULL) {
            parcBufferComposer_Format(composer, "%u", label->type);
        } else {
            parcBufferComposer_PutString(composer, mnemonic);
        }

        if (label->parameter != NULL) {
            parcBufferComposer_PutString(composer, ":");
            parcBufferComposer_PutBuffer(composer, label->parameter);
        }
        parcBufferComposer_PutString(composer, "=");
    }

    return composer;
}

char *
ccnxNameLabel_ToString(const CCNxNameLabel *label)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    ccnxNameLabel_BuildString(label, composer);

    char *result = parcBufferComposer_ToString(composer);

    parcBufferComposer_Release(&composer);

    return result;
}
