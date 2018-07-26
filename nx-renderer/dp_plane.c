#include <errno.h>
#include <string.h>

#include "dp_common.h"
#include "dp.h"

static int dp_plane_probe(struct dp_plane *plane)
{
	struct dp_device *device = plane->device;
	drmModeObjectPropertiesPtr props;
	drmModePlane *p;
	unsigned int i;

	p = drmModeGetPlane(device->fd, plane->id);
	if (!p)
		return -ENODEV;

	if (p->crtc_id == 0) {
		for (i = 0; i < device->num_crtcs; i++) {
			if (p->possible_crtcs & (1 << i)) {
				p->crtc_id = device->crtcs[i]->id;
				break;
			}
		}
	}

	for (i = 0; i < device->num_crtcs; i++) {
		if (device->crtcs[i]->id == p->crtc_id) {
			plane->crtc = device->crtcs[i];
			break;
		}
	}

	plane->formats = calloc(p->count_formats, sizeof(uint32_t));
	if (!plane->formats)
		return -ENOMEM;

	for (i = 0; i < p->count_formats; i++)
		plane->formats[i] = p->formats[i];

	plane->num_formats = p->count_formats;

	drmModeFreePlane(p);

	props = drmModeObjectGetProperties(device->fd, plane->id,
					   DRM_MODE_OBJECT_PLANE);
	if (!props)
		return -ENODEV;

	for (i = 0; i < props->count_props; i++) {
		drmModePropertyPtr prop;

		prop = drmModeGetProperty(device->fd, props->props[i]);
		if (prop) {
			DP_DBG("plane.%2d %d.%d [%s]\n",
				plane->id, props->count_props, i, prop->name);
			if (strcmp(prop->name, "type") == 0)
				plane->type = props->prop_values[i];

			drmModeFreeProperty(prop);
		}
	}

	drmModeFreeObjectProperties(props);

	return 0;
}

struct dp_plane *dp_plane_create(struct dp_device *device, uint32_t id)
{
	struct dp_plane *plane;

	plane = calloc(1, sizeof(*plane));
	if (!plane)
		return NULL;

	plane->device = device;
	plane->id = id;

	dp_plane_probe(plane);

	return plane;
}

void dp_plane_free(struct dp_plane *plane)
{
	free(plane);
}

int dp_plane_set(struct dp_plane *plane, struct dp_framebuffer *fb,
			int x, int y, int w, int h, int cx, int cy, int cw, int ch)
{
	struct dp_device *device = plane->device;
	int err;
	int screen_x = x, screen_y = y, screen_w = w, screen_h = h;
	int crop_x = cx, crop_y  = cy, crop_w = cw, crop_h = ch;

	err = drmModeSetPlane(device->fd, plane->id, plane->crtc->id, fb->id,
			      0, screen_x, screen_y, screen_w, screen_h,
			      crop_x << 16, crop_y << 16, crop_w << 16, crop_h << 16);
	if (err < 0)
		return -errno;

	return 0;
}

bool dp_plane_supports_format(struct dp_plane *plane, uint32_t format)
{
	unsigned int i;

	for (i = 0; i < plane->num_formats; i++) {
		if (plane->formats[i] == format) {
			DP_DBG("done  :%s - %s\n",
				dp_forcc_name(plane->formats[i]), dp_forcc_name(format));
			return true;
		}
	}

	return false;
}
