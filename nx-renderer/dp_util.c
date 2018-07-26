
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

#define	DP_BUFMGR_NAME "DP"

const char *dp_forcc_name(uint32_t format)
{
    static char buf[32];
#if 0
    snprintf(buf, sizeof(buf),
         "%c%c%c%c %s-endian (0x%08x)",
         (format & 0xff),
         ((format >> 8) & 0xff),
         ((format >> 16) & 0xff),
         ((format >> 24) & 0x7f),
         format & DRM_FORMAT_BIG_ENDIAN ? "big" : "little",
         format);
#else
    snprintf(buf, sizeof(buf),
         "%c%c%c%c",
         (format & 0xff),
         ((format >> 8) & 0xff),
         ((format >> 16) & 0xff),
         ((format >> 24) & 0x7f));
#endif
    return buf;
}

bool dp_dbg_on = false;

void dp_debug_on(bool on)
{
	dp_dbg_on = on;
}

static unsigned int dbg_delay = 0;

void dp_debug_wait(int ms)
{
	dbg_delay = ms;
}

void dp_bufmgr_debug(const char *function,
			     int line, const char *msg, ...)
{
	va_list args;

    fprintf(stderr, "[%s", DP_BUFMGR_NAME);
    if (function)
        fprintf(stderr, ":%s", function);

	if (line > 0)
		fprintf(stderr, ":%5d] ", line);
	else
		fprintf(stderr, "] ");

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);

	if (dbg_delay) {
		usleep(dbg_delay*1000);
	}
}


