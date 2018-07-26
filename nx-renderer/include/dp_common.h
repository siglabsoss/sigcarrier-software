#ifndef _DP_COMMON_H_
#define _DP_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

const char *dp_forcc_name(uint32_t format);

extern bool dp_dbg_on;
void dp_bufmgr_debug(const char *function,
			     int line, const char *msg, ...);

void dp_debug_on(bool on);
void dp_debug_wait(int ms);

#define DP_LOG(args...) \
		dp_bufmgr_debug(NULL, -1, ##args)

#define DP_ERR(args...) \
		dp_bufmgr_debug(__func__, __LINE__, ##args)

#define DP_DBG(args...) \
	do {		\
		if (dp_dbg_on)	\
			dp_bufmgr_debug(__func__, __LINE__, ##args);	\
	} while (0)

#ifdef __cplusplus
}
#endif

#endif
