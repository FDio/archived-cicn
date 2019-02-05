
/*
 * Copyright (c) 2019 Cisco and/or its affiliates.
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

#include <parc/windows/parc_Utils.h>
#include <Windows.h>
#include <stdint.h>
#include <stdio.h>
#ifndef _vscprintf
/* For some reason, MSVC fails to honour this #ifndef. */
/* Hence function renamed to _vscprintf_so(). */
int _vscprintf_so(const char *format, va_list pargs)
{
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#endif // _vscprintf

#ifndef vasprintf
int vasprintf(char **strp, const char *fmt, va_list ap)
{
    int len = _vscprintf_so(fmt, ap);
    if (len == -1)
        return -1;
    char *str = malloc((size_t)len + 1);
    if (!str)
        return -1;
    int r = vsnprintf(str, len + 1, fmt, ap); /* "secure" version of vsprintf */
    if (r == -1)
        return free(str), -1;
    *strp = str;
    return r;
}
#endif // vasprintf

#ifndef dprintf
int dprintf(int fd, char *fmt, ...) {
    va_list ap;
    FILE *f = _fdopen( fd, "rw" );
    int rc;

    va_start(ap, &fmt);
    rc = vfprintf(f, fmt, ap);
    fclose(f);
    va_end(ap);
    return rc;
}
#endif

#ifndef asprintf
int asprintf(char *strp[], const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vasprintf(strp, fmt, ap);
    va_end(ap);
    return r;
}
#endif // asprintf


int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}

#ifndef strndup
char *strndup(const char *str, size_t chars)
{
    char *buffer;
    int n;

    buffer = (char *) malloc(chars +1);
    if (buffer)
    {
        for (n = 0; ((n < chars) && (str[n] != 0)) ; n++) buffer[n] = str[n];
        buffer[n] = 0;
    }

    return buffer;
}
#endif


void usleep(__int64 usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}