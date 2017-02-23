//
//  parc_Status.c
//  Libparc
//
//
//
#include <config.h>
#include <stdio.h>

#include "parc_Execution.h"

/*
 * A PARCExecution value is a unique thing which can have a string assigned to it.
 *
 * I want the function to be:
 *
 * A thing that returns an Execution
 *
 * Execution function() { }
 *
 * A thing that an Execution value can be
 *
 * Execution value = function;
 */

struct PARCExecution {
    struct PARCExecution (*type)(char *format, ...);
    char *message;
};

PARCExecution *PARCExecution_OK = &(PARCExecution) {
    .message = "OK"
};

PARCExecution *PARCExecution_Timeout = &(PARCExecution) {
    .message = "Timeout"
};

PARCExecution *PARCExecution_Interrupted = &(PARCExecution) {
    .message = "Interrupted"
};

PARCExecution *PARCExecution_IOError = &(PARCExecution) {
    .message = "I/O Error"
};

PARCExecution *
parcExecution_OK(const char *format, ...)
{
    return PARCExecution_OK;
}

PARCExecution *
parcExecution_Interrupted(const char *format, ...)
{
    return PARCExecution_Interrupted;
}

PARCExecution *
parcExecution_IOError(const char *format, ...)
{
    return PARCExecution_IOError;
}

bool
parcExecution_Is(const PARCExecution *exec, const PARCExecution *other)
{
    return (exec == other);
}

char *
parcExecution_GetMessage(const PARCExecution *exec)
{
    return exec->message;
}

PARCExecution *
bar()
{
    return PARCExecution_OK;
}

PARCExecution *
baz()
{
    return parcExecution_OK("Nothing to say");
}

void
foo()
{
    PARCExecution *x = bar();
    PARCExecution *y = baz();

    printf("%s\n", parcExecution_GetMessage(x));
    printf("%s\n", parcExecution_GetMessage(y));
}
