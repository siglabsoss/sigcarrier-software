/*
 * Copyright © 2013 ARM Limited.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "../drmmode_driver.h"
#include <stddef.h>
#include <xf86drm.h>

/* Cursor dimensions
 * Technically we probably don't have any size limit.. since we
 * are just using an overlay... but xserver will always create
 * cursor images in the max size, so don't use width/height values
 * that are too big
 */
#define CURSORW   (64)
#define CURSORH   (64)
/* Padding added down each side of cursor image */
#define CURSORPAD (0)

#define ALIGN(val, align)	(((val) + (align) - 1) & ~((align) - 1))

/*
 * nexell gem memory type
 */
enum nx_gem_type {
	/*
	 * DMA continuous memory
	 * user   : non-cacheable
	 * kernel : non-cacheable
	 */
	NEXELL_BO_DMA,

	/*
	 * DMA continuous memory, allocate from DMA,
	 * user   : cacheable
	 * kernel : non-cacheable
	 */
	NEXELL_BO_DMA_CACHEABLE,

	/*
	 * System continuous memory, allocate from system
	 * user   : non-cacheable
	 * kernel : non-cacheable
	 */
	NEXELL_BO_SYSTEM,

	/*
	 * System continuous memory, allocate from system
	 * user   : cacheable
	 * kernel : cacheable
	 */
	NEXELL_BO_SYSTEM_CACHEABLE,

	/*
	 * System non-continuous memory, allocate from system
	 * user   : non-cacheable
	 * kernel : non-cacheable
	 */
	NEXELL_BO_SYSTEM_NONCONTIG,

	/*
	 * System non-continuous memory, allocate from system
	 * user   : cacheable
	 * kernel : cacheable
	 */
	NEXELL_BO_SYSTEM_NONCONTIG_CACHEABLE,

	NEXELL_BO_MAX,
};

struct nx_gem_create {
	uint64_t size;
	unsigned int flags;
	unsigned int handle;
	void *priv_data;
};

#define DRM_NX_GEM_CREATE		0x00
#define DRM_IOCTL_NX_GEM_CREATE	DRM_IOWR(DRM_COMMAND_BASE + \
			DRM_NX_GEM_CREATE, struct nx_gem_create)
/***************************************************************************/

static int create_custom_gem(int fd, struct armsoc_create_gem *create_gem)
{
	struct nx_gem_create create_nexell;
	int ret;
	unsigned int pitch;

	/* make pitch a multiple of 64 bytes for best performance */
	pitch = ALIGN(create_gem->width * ((create_gem->bpp + 7) / 8), 64);
	memset(&create_nexell, 0, sizeof(create_nexell));
	create_nexell.size = create_gem->height * pitch;

	assert((create_gem->buf_type == ARMSOC_BO_SCANOUT) ||
			(create_gem->buf_type == ARMSOC_BO_NON_SCANOUT));

	ret = drmIoctl(fd, DRM_IOCTL_NX_GEM_CREATE, &create_nexell);
	if (ret)
		return ret;

	/* Convert custom create_nexell to generic create_gem */
	create_gem->handle = create_nexell.handle;
	create_gem->pitch = pitch;
	create_gem->size = create_nexell.size;

	return 0;
}

struct drmmode_interface nexell_interface = {
	"nexell"	      /* name of drm driver */,
	1                     /* use_page_flip_events */,
	1                     /* use_early_display */,
	CURSORW               /* cursor width */,
	CURSORH               /* cursor_height */,
	CURSORPAD             /* cursor padding */,
	HWCURSOR_API_NONE     /* cursor_api */,
	NULL                  /* init_plane_for_cursor */,
	0                     /* vblank_query_supported */,
	create_custom_gem     /* create_custom_gem */,
};
