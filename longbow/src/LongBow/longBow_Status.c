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

#include <LongBow/longBow_Status.h>
#include <LongBow/unit-test.h>

bool
longBowStatus_IsSuccessful(LongBowStatus status)
{
    if (status == LONGBOW_STATUS_SUCCEEDED || longBowStatus_IsWarning(status) || longBowStatus_IsIncomplete(status)) {
        return true;
    }
    return false;
}

bool
longBowStatus_IsFailed(LongBowStatus status)
{
    switch (status) {
        case LONGBOW_STATUS_FAILED:
        case LONGBOW_STATUS_MEMORYLEAK:
        case LongBowStatus_STOPPED:
        case LONGBOW_STATUS_TEARDOWN_FAILED:
        case LONGBOW_STATUS_SETUP_FAILED:
            return true;

        default:
            if (status >= LongBowStatus_SIGNALLED) {
                return true;
            }
            return false;
    }
}

bool
longBowStatus_IsWarning(LongBowStatus status)
{
    switch (status) {
        case LongBowStatus_WARNED:
        case LongBowStatus_TEARDOWN_WARNED:
            return true;

        default:
            return false;
    }
}

bool
longBowStatus_IsIncomplete(LongBowStatus status)
{
    switch (status) {
        case LONGBOW_STATUS_SKIPPED:
        case LongBowStatus_UNIMPLEMENTED:
        case LongBowStatus_IMPOTENT:
            return true;

        default:
            return false;
    }
}

bool
longBowStatus_IsSignalled(LongBowStatus status)
{
    return status >= LongBowStatus_SIGNALLED;
}

static struct toString {
    LongBowStatus status;
    char *string;
} toString[] = {
    { LONGBOW_STATUS_SUCCEEDED,       "Succeeded"         },
    { LongBowStatus_WARNED,           "Warning"           },
    { LongBowStatus_TEARDOWN_WARNED,  "Tear Down Warning" },
    { LONGBOW_STATUS_SKIPPED,         "Skipped"           },
    { LongBowStatus_UNIMPLEMENTED,    "Unimplemented"     },
    { LongBowStatus_IMPOTENT,         "Impotent"          },
    { LONGBOW_STATUS_FAILED,          "Failed"            },
    { LongBowStatus_STOPPED,          "Stopped"           },
    { LONGBOW_STATUS_TEARDOWN_FAILED, "Tear Down Failed"  },
    { LONGBOW_STATUS_SETUP_FAILED,    "Setup Failed"      },
    { LONGBOW_STATUS_MEMORYLEAK,      "Memory Leak"       },
    { 0,                              NULL                },
};

static const char *
_longBowStatus_StatusToString(const LongBowStatus status)
{
    const char *result = NULL;
    for (const struct toString *element = &toString[0]; element->string != NULL; element++) {
        if (element->status == status) {
            result = element->string;
            break;
        }
    }
    return result;
}

char *
longBowStatus_ToString(const LongBowStatus status)
{
    char *result = (char *) _longBowStatus_StatusToString(status);

    if (result == NULL) {
        if (status >= LongBowStatus_SIGNALLED) {
            int signalNumber = status - LongBowStatus_SIGNALLED;
            char *signalName = strsignal(signalNumber);
            int check;
            if (signalName == NULL) {
                check = asprintf(&result, "Signaled %d.", signalNumber);
            } else {
                check = asprintf(&result, "Signaled %s.", signalName);
            }
            if (check == -1) {
                return NULL;
            }
        } else {
            int check = asprintf(&result, "Unknown status: %d.  This is a bug.", status);
            if (check == -1) {
                return NULL;
            }
        }
    } else {
        result = strdup(result);
    }

    return result;
}
