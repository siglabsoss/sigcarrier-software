#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/mman.h>

#include <drm_fourcc.h>

#include "xf86drm.h"

#include "dp_common.h"
#include "dp.h"

#ifndef CAL_ALIGN
#define CAL_ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif

static int dp_framebuffer_dump_format_sizes(uint32_t format, int width, int height,
			int *virtual_width, int *virtual_height,
			int w[3], int h[3],
			int *bpp, int *num_buffers)
{
	int ret = 0;

	DP_DBG("request %s (%d x %d)\n",
		dp_forcc_name(format), width, height);

	switch (format) {
	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
		*bpp = 16;
		*virtual_width = width;
		*virtual_height = height;

		*num_buffers = 1;
		w[0] = width;
		h[0] = height;
		w[1] = 0;
		h[1] = 0;
		w[2] = 0;
		h[2] = 0;
		break;

	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
		*bpp = 24;
		*virtual_width = width;
		*virtual_height = height;

		*num_buffers = 1;
		w[0] = width;
		h[0] = height;
		w[1] = 0;
		h[1] = 0;
		w[2] = 0;
		h[2] = 0;
		break;

	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
		*bpp = 32;
		*virtual_width = width;
		*virtual_height = height;

		*num_buffers = 1;
		w[0] = width;
		h[0] = height;
		w[1] = 0;
		h[1] = 0;
		w[2] = 0;
		h[2] = 0;
		break;

	/* 1 buffer */
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
		*bpp = 8;
		*virtual_width = width * 2;
		*virtual_height = height;

		*num_buffers = 1;
		w[0] = width;
		h[0] = height;
		w[1] = 0;
		h[1] = 0;
		w[2] = 0;
		h[2] = 0;
		break;

	/* 2 buffer */
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
		*bpp = 8;
		*virtual_width = width;
		*virtual_height = height * 3 / 2;

		*num_buffers = 2;
		w[0] = width;
		h[0] = height;
		w[1] = width;
		h[1] = height/2;
		w[2] = 0;
		h[2] = 0;
		break;

	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
		*bpp = 8;
		*virtual_width = width;
		*virtual_height = height * 2;

		*num_buffers = 2;
		w[0] = width;
		h[0] = height;
		w[1] = width;
		h[1] = height;
		w[2] = 0;
		h[2] = 0;
		break;

	/* 3 buffer */
#if 0
	case DRM_FORMAT_YUV410:
	case DRM_FORMAT_YVU410:
	case DRM_FORMAT_YUV411:
	case DRM_FORMAT_YVU411:
		*bpp = 8;
		*num_buffers = 3;
		????
		break;
#endif

	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		*bpp = 8;
		*virtual_width = width;
		*virtual_height = height * 3 / 2;

		*num_buffers = 3;
		w[0] = width;
		h[0] = height;
		w[1] = width/2;
		h[1] = height/2;
		w[2] = w[1];
		h[2] = h[1];
		break;

	case DRM_FORMAT_YUV422:
	case DRM_FORMAT_YVU422:
		*bpp = 8;
		*virtual_width = width;
		*virtual_height = height * 2;

		*num_buffers = 3;
		w[0] = width;
		h[0] = height;
		w[1] = width/2;
		h[1] = height;
		w[2] = w[1];
		h[2] = h[1];
		break;

	case DRM_FORMAT_YUV444:
	case DRM_FORMAT_YVU444:
		*bpp = 8;
		*virtual_width = width;
		*virtual_height = height * 3;

		*num_buffers = 3;
		w[0] = width;
		h[0] = height;
		w[1] = width;
		h[1] = height;
		w[2] = w[1];
		h[2] = h[1];
		break;

	default:
		ret = -EINVAL;
		break;
	}

	DP_DBG("%s : planes %d (%dx%d, %dx%d, %dx%d, %d bpp)\n",
		ret ? "fail" : "done",
		*num_buffers, w[0], h[0], w[1], h[1], w[2], h[2], *bpp);

	return ret;
}

static int dp_framebuffer_dumb_format_planes(unsigned int format,
			unsigned int width, unsigned int height,
			int pitch, uint32_t handle,
			uint32_t handles[4], unsigned int pitches[4],
			unsigned int offsets[4])
{
	int num_planes = 0;
	int i = 0, ret = 0;

	DP_DBG("request %s (%d x %d, %d) handle: 0x%x\n",
		dp_forcc_name(format), width, height, pitch, handle);

	switch (format) {
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		num_planes = 1;
		break;

	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = handle;
		num_planes = 2;
		break;

	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		pitches[1] = pitches[0] / 2;
		offsets[1] = pitches[0] * height;
		handles[1] = handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height / 2;
		handles[2] = handle;
		num_planes = 3;
		break;

	case DRM_FORMAT_YUV422:
	case DRM_FORMAT_YVU422:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		pitches[1] = pitches[0] / 2;
		offsets[1] = pitches[0] * height;
		handles[1] = handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height;
		handles[2] = handle;
		num_planes = 3;
		break;

	case DRM_FORMAT_YUV444:
	case DRM_FORMAT_YVU444:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height;
		handles[2] = handle;
		num_planes = 3;
		break;

	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		num_planes = 1;
		break;

	default:
		ret = -EINVAL;
	}

	if (0 > ret)
		DP_ERR("%s : for %s\n", dp_forcc_name(format));

	for (i = 0; num_planes > i; i++)
		DP_DBG("[%d] hnd:0x%x, offs:%d, pitch:%d\n",
			i, handles[i], offsets[i], pitches[i]);

	return ret;
}

static int dp_framebuffer_format_planes(unsigned int format,
			unsigned int width, unsigned int height,
			int pitch, uint32_t handle,
			uint32_t handles[4], unsigned int pitches[4],
			unsigned int offsets[4])
{
	int num_planes = 0;
	int i = 0, ret = 0;

	DP_DBG("request %s (%d x %d, %d) handle: 0x%x\n",
		dp_forcc_name(format), width, height, pitch, handle);

	switch (format) {
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		num_planes = 1;
		break;

	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = handle;
		num_planes = 2;
		break;

	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		pitches[1] = CAL_ALIGN(pitches[0] >> 1, 16);
		offsets[1] = pitches[0] * CAL_ALIGN(height, 16);
		handles[1] = handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * CAL_ALIGN(height / 2, 16);
		handles[2] = handle;
		num_planes = 3;
		break;

	case DRM_FORMAT_YUV422:
	case DRM_FORMAT_YVU422:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		pitches[1] = pitches[0] / 2;
		offsets[1] = pitches[0] * height;
		handles[1] = handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height;
		handles[2] = handle;
		num_planes = 3;
		break;

	case DRM_FORMAT_YUV444:
	case DRM_FORMAT_YVU444:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height;
		handles[2] = handle;
		num_planes = 3;
		break;

	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
		offsets[0] = 0;
		handles[0] = handle;
		pitches[0] = pitch;
		num_planes = 1;
		break;

	default:
		ret = -EINVAL;
	}

	if (0 > ret)
		DP_ERR("%s : for %s\n", dp_forcc_name(format));

	for (i = 0; num_planes > i; i++)
		DP_DBG("[%d] hnd:0x%x, offs:%d, pitch:%d\n",
			i, handles[i], offsets[i], pitches[i]);

	return ret;
}

static int dp_framebuffer_dumb_format_maps(unsigned int format,
			void *virtual, unsigned int offsets[4], void *planes[4])
{
	int num_planes = 0;
	int i = 0, ret = 0;

	DP_DBG("request %s virtual: 0x%x\n",
		dp_forcc_name(format), virtual);

	switch (format) {
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		planes[0] = virtual;
		num_planes = 1;
		break;

	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		num_planes = 2;
		break;

	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
	case DRM_FORMAT_YUV422:
	case DRM_FORMAT_YVU422:
	case DRM_FORMAT_YUV444:
	case DRM_FORMAT_YVU444:
		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		planes[2] = virtual + offsets[2];
		num_planes = 3;
		break;

	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
		planes[0] = virtual;
		num_planes = 1;
		break;

	default:
		ret = -EINVAL;
	}

	if (0 > ret)
		DP_ERR("%s : for %s\n", dp_forcc_name(format));

	for (i = 0; num_planes > i; i++)
		DP_DBG("[%d] planes:0x%x\n", i, planes[i]);

	return ret;
}

struct dp_framebuffer *dp_framebuffer_create(struct dp_device *device,
			uint32_t format, int width, int height, bool seperate)
{
	struct dp_framebuffer *fb;
	unsigned int handles[3] = {0, };
	int virtual_width = 0;
	int virtual_height = 0;
	int buffers =  1, count = 0;
	int w[3] = {0, };
	int h[3] = {0, };
	int bpp = 0, err;
	int i;

	fb = calloc(1, sizeof(*fb));
	if (!fb)
		return NULL;

	fb->device = device;
	fb->width = width;
	fb->height = height;
	fb->format = format;

	/*
 	 * format planes size
 	 */
	err = dp_framebuffer_dump_format_sizes(format, width, height,
			&virtual_width, &virtual_height, w, h, &bpp, &buffers);

	if (err) {
		free(fb);
		return NULL;
	}

	if (1 == buffers)
		seperate = 0;

	count = seperate ? buffers : 1;

	DP_DBG("dumb : (%d) %s buffer for %d buffers %d bpp\n",
		count, seperate ? "seperated" : "one", buffers, bpp);
	/*
	 * allocate dumb buffers
	 */
	for (i = 0; count > i; i++) {
		unsigned int handle;
		int pitch, size;
		int fd = device->fd;
		int aw = seperate ? w[i] : virtual_width;
		int ah = seperate ? h[i] : virtual_height;

		err = dp_buffer_alloc(fd, aw, ah, bpp, &handle, &pitch, &size);
		if (0 > err) {
			DP_ERR("fail : CREATE_DUMB for [%d] %dx%d, %d : %m\n",
				i, aw, ah, bpp);
			goto err_create;
		}

		fb->handles[i] = handle;
		fb->sizes[i] = size;
		fb->pitches[i] = pitch;
		handles[i] = handle;

		DP_DBG("dumb : %s [%d] %dx%d hnd 0x%x, pitch %d, size %d, %d bpp\n",
			dp_forcc_name(format), i, aw, ah,
			fb->handles[i], fb->pitches[i], fb->sizes[i], bpp);
	}

	fb->planes = count;
	fb->seperated = seperate;

	if (!seperate) {
		uint32_t handle = fb->handles[0];
		int pitch = fb->pitches[0];

		/*
	 	 * format planes info
	 	 */
		err = dp_framebuffer_dumb_format_planes(format,
					width, height, pitch, handle,
					fb->handles, (unsigned int *)fb->pitches, fb->offsets);
		if (0 > err)
			goto err_create;

		DP_DBG("dumb : %s hnd[0x%x,0x%x,0x%x], pitch[%d,%d,%d], offs[%d,%d,%d] \n",
			dp_forcc_name(format),
			fb->handles[0], fb->handles[1], fb->handles[2],
			fb->pitches[0], fb->pitches[1], fb->pitches[2],
			fb->offsets[0], fb->offsets[1], fb->offsets[2]);
	}

	return fb;

err_create:
	for ( ; i > 0; i--) {
		if (handles[i])
			dp_buffer_free(device->fd, handles[i]);
	}

	free(fb);
	return NULL;
}

struct dp_framebuffer *dp_framebuffer_config(struct dp_device *device,
			uint32_t format, int width, int height, bool seperate, int gem_fd, int size)
{
	struct dp_framebuffer *fb;
	int virtual_width = 0;
	int virtual_height = 0;
	int buffers =  1, count = 0;
	int w[3] = {0, };
	int h[3] = {0, };
	int bpp = 0, err;
	int i;

	if(!gem_fd)
	{
		DP_ERR("[%s] : invalid gem_fd : %d \n",__func__, gem_fd);
		return NULL;
	}

	fb = calloc(1, sizeof(*fb));
	if (!fb)
		return NULL;

	fb->device = device;
	fb->width = width;
	fb->height = height;
	fb->format = format;

	/*
 	 * format planes size
 	 */
	err = dp_framebuffer_dump_format_sizes(format, width, height,
			&virtual_width, &virtual_height, w, h, &bpp, &buffers);

	if (err) {
		free(fb);
		return NULL;
	}

	if (1 == buffers)
		seperate = 0;

	count = seperate ? buffers : 1;

	DP_DBG("dumb : (%d) %s buffer for %d buffers %d bpp\n",
		count, seperate ? "seperated" : "one", buffers, bpp);
	/*
	 * allocate dumb buffers
	 */
	for (i = 0; count > i; i++) {
		int aw = seperate ? w[i] : virtual_width;
		int ah = seperate ? h[i] : virtual_height;

		fb->handles[i] = gem_fd;
		fb->sizes[i] = size;
		fb->pitches[i] = CAL_ALIGN(aw, 32);
		DP_DBG("dumb : %s [%d] %dx%d hnd 0x%x, pitch %d, size %d, %d bpp\n",
			dp_forcc_name(format), i, aw, ah,
			fb->handles[i], fb->pitches[i], fb->sizes[i], bpp);
	}

	fb->planes = count;
	fb->seperated = seperate;

	if (!seperate) {
		uint32_t handle = fb->handles[0];
		int pitch = fb->pitches[0];

		/*
	 	 * format planes info
	 	 */
		err = dp_framebuffer_format_planes(format,
					width, height, pitch, handle,
					fb->handles, (unsigned int*)fb->pitches, fb->offsets);
		if (0 > err)
			goto err_create;

		DP_DBG("dumb : %s hnd[0x%x,0x%x,0x%x], pitch[%d,%d,%d], offs[%d,%d,%d] \n",
			dp_forcc_name(format),
			fb->handles[0], fb->handles[1], fb->handles[2],
			fb->pitches[0], fb->pitches[1], fb->pitches[2],
			fb->offsets[0], fb->offsets[1], fb->offsets[2]);
	}

	return fb;

err_create:
	free(fb);
	return NULL;
}

void dp_framebuffer_free(struct dp_framebuffer *fb)
{
	struct dp_device *device = fb->device;
	int count, i;

	count = fb->seperated ? fb->planes : 1;

	for (i = 0; i < count; i++) {

		if (!fb->handles[i])
			break;
		dp_buffer_free(device->fd, fb->handles[i]);
	}

	free(fb);
}

int dp_framebuffer_addfb2(struct dp_framebuffer *fb)
{
	struct dp_device *device = fb->device;
	int width = fb->width;
	int height = fb->height;
	uint32_t format = fb->format;
	int err;

	err = (int)drmModeAddFB2(device->fd, width, height, format,
				fb->handles, (unsigned int*)fb->pitches, fb->offsets, &fb->id, 0);
	if (0 > err)
		return -EINVAL;

	return 0;
}

void dp_framebuffer_delfb2(struct dp_framebuffer *fb)
{
	struct dp_device *device = fb->device;

	if (fb->id)
		drmModeRmFB(device->fd, fb->id);
}

int dp_framebuffer_map(struct dp_framebuffer *fb)
{
	struct dp_device *device = fb->device;
	struct drm_mode_map_dumb args;
	void *ptr;
	int count;
	int i, err;

	count = fb->seperated ? fb->planes : 1;

	/*
 	 * memory map plane buffers
 	 */
	for (i = 0; count > i; i++) {

		if (!fb->handles[i])
			continue;

		memset(&args, 0, sizeof(args));
		args.handle = fb->handles[i];

		err = drmIoctl(device->fd, DRM_IOCTL_MODE_MAP_DUMB, &args);
		if (0 > err)
			return -errno;

		DP_DBG("map  : %s [%d] hnd:0x%x, offs:0x%x\n",
			dp_forcc_name(fb->format), i, args.handle, args.offset);

		ptr = mmap(0, fb->sizes[i], PROT_READ | PROT_WRITE, MAP_SHARED,
			   device->fd, args.offset);

		if (ptr == MAP_FAILED)
			return -errno;

		fb->ptrs[i] = ptr;

		DP_DBG("done : %s [%d] ptr:0x%x, hnd:0x%x, offs:%d, pitch:%d\n",
			dp_forcc_name(fb->format), i, fb->ptrs[i],
			fb->handles[i], fb->offsets[i], fb->pitches[i]);
	}

	/*
 	 * formate map planes
 	 */
 	if (!fb->seperated)
		dp_framebuffer_dumb_format_maps(fb->format,
				fb->ptrs[0], fb->offsets, fb->ptrs);

	return 0;
}

void dp_framebuffer_unmap(struct dp_framebuffer *fb)
{
	int count, i;

	count = fb->seperated ? fb->planes : 1;

	for (i = 0; count > i; i++) {

		if (!fb->ptrs[i])
			continue;

		munmap(fb->ptrs[i], fb->sizes[i]);
		fb->ptrs[i] = NULL;
	}
}
