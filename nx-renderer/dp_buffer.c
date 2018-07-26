#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/mman.h>

#include "drm_fourcc.h"

#include "xf86drm.h"

#include "dp_common.h"
#include "dp.h"

unsigned int dp_buffer_alloc(int fd, int width, int height, int bpp,
				unsigned int *handle, int *pitch, int *size)
{

	struct drm_mode_create_dumb args;
	int err;

	memset(&args, 0, sizeof(args));

	args.width  = width;
	args.height = height;
	args.bpp = bpp ? bpp : 8;

	err = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &args);
	if (0 > err) {
		DP_ERR("fail : CREATE_DUMB for %dx%d, %d : %m\n",
			width, height, bpp);
		return err;
	}

	if (handle)
		*handle = args.handle;

	if (pitch)
		*pitch = args.pitch;

	if (size)
		*size = args.size;

	DP_DBG("dumb : %dx%d,%d - hnd 0x%x, pitch %d, size %d\n",
		width, height, bpp, args.handle, args.pitch, args.size);

	return 0;
}

void dp_buffer_free(int fd, unsigned int handle)
{
	struct drm_mode_destroy_dumb args;

	memset(&args, 0, sizeof(args));
	args.handle = handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &args);
}

