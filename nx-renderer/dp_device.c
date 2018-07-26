#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dp.h"
#include "dp_common.h"

static const char *const connector_names[] = {
	"Unknown",
	"VGA",
	"DVI-I",
	"DVI-D",
	"DVI-A",
	"Composite",
	"SVIDEO",
	"LVDS",
	"Component",
	"9PinDIN",
	"DisplayPort",
	"HDMI-A",
	"HDMI-B",
	"TV",
	"eDP",
	"Virtual",
	"DSI",
};

static void dp_device_probe_screens(struct dp_device *device)
{
	unsigned int counts[ARRAY_SIZE(connector_names)];
	struct dp_screen *screen;
	drmModeRes *res;
	int i;

	memset(counts, 0, sizeof(counts));

	res = drmModeGetResources(device->fd);
	if (!res) {
		DP_ERR("fail : drmModeGetResources %m\n");
		return;
	}

	device->screens = calloc(res->count_connectors, sizeof(screen));
	if (!device->screens){
		DP_ERR("fail : calloc for res->count_connectors %m\n");
		return;
	}

	for (i = 0; i < res->count_connectors; i++) {
		unsigned int *count;
		const char *type;
		int len;

		screen = dp_screen_create(device, res->connectors[i]);
		if (!screen)
			continue;

		/* assign a unique name to this screen */
		type = connector_names[screen->type];
		count = &counts[screen->type];

		len = snprintf(NULL, 0, "%s-%u", type, *count);

		screen->name = malloc(len + 1);
		if (!screen->name) {
			free(screen);
			continue;
		}

		snprintf(screen->name, len + 1, "%s-%u", type, *count);
		(*count)++;

		device->screens[i] = screen;
		device->num_screens++;
	}

	drmModeFreeResources(res);
}

static void dp_device_probe_crtcs(struct dp_device *device)
{
	struct dp_crtc *crtc;
	drmModeRes *res;
	int i;

	res = drmModeGetResources(device->fd);
	if (!res) {
		DP_ERR("fail : drmModeGetResources %m\n");
		return;
	}

	device->crtcs = calloc(res->count_crtcs, sizeof(crtc));
	if (!device->crtcs) {
		DP_ERR("fail : calloc for res->count_crtcs %m\n");
		return;
	}

	for (i = 0; i < res->count_crtcs; i++) {
		crtc = dp_crtc_create(device, res->crtcs[i]);
		if (!crtc)
			continue;

		device->crtcs[i] = crtc;
		device->num_crtcs++;
	}

	drmModeFreeResources(res);
}

static void dp_device_probe_planes(struct dp_device *device)
{
	struct dp_plane *plane;
	drmModePlaneRes *res;
	unsigned int i;

	res = drmModeGetPlaneResources(device->fd);
	if (!res) {
		DP_ERR("fail : drmModeGetPlaneResources %m\n");
		return;
	}

	device->planes = calloc(res->count_planes, sizeof(plane));
	if (!device->planes) {
		DP_ERR("fail : calloc for res->count_planes %m\n");
		return;
	}

	for (i = 0; i < res->count_planes; i++) {
		plane = dp_plane_create(device, res->planes[i]);
		if (!plane)
			continue;

		device->planes[i] = plane;
		device->num_planes++;
	}

	drmModeFreePlaneResources(res);
}

static void dp_device_probe(struct dp_device *device)
{
	dp_device_probe_screens(device);
	dp_device_probe_crtcs(device);
	dp_device_probe_planes(device);
}

struct dp_device *dp_device_open(int fd)
{
	struct dp_device *device;

	device = calloc(1, sizeof(*device));
	if (!device) {
		DP_ERR("fail : calloc for dp_device %m\n");
		return NULL;
	}

	device->fd = fd;

	dp_device_probe(device);

	return device;
}

void dp_device_close(struct dp_device *device)
{
	unsigned int i;

	for (i = 0; i < device->num_planes; i++)
		dp_plane_free(device->planes[i]);

	free(device->planes);

	for (i = 0; i < device->num_crtcs; i++)
		dp_crtc_free(device->crtcs[i]);

	free(device->crtcs);

	for (i = 0; i < device->num_screens; i++)
		dp_screen_free(device->screens[i]);

	free(device->screens);

	if (device->fd >= 0)
		close(device->fd);

	free(device);
}

struct dp_plane *dp_device_find_plane_by_index(struct dp_device *device,
						unsigned int crtc_index, unsigned int plane_index)
{
	struct dp_crtc *crtc;
	unsigned int i;

	if (crtc_index > device->num_crtcs-1) {
		DP_ERR("fail : crtc index %d over max %d\n",
			crtc_index, device->num_crtcs);
		return NULL;
	}

	crtc = device->crtcs[crtc_index];

	for (i = 0; i < device->num_planes; i++) {
		if (crtc->id != device->planes[i]->crtc->id)
			continue;

		if ((i + plane_index) >= device->num_planes)
			return NULL;

		if (crtc->id != device->planes[i]->crtc->id) {
			DP_ERR("fail : crtc id %d not equal plane[%d]'s crtc id %d\n",
				crtc->id, i, device->planes[i]->crtc->id);
			return NULL;
		}

		DP_DBG("planes %d <%d.%d>: device->planes[%d]->type = 0x%x\n",
			device->num_planes, crtc_index, plane_index,
			i+plane_index, device->planes[i+plane_index]->type);

		return device->planes[i+plane_index];
	}

	DP_ERR("fail : planes not exist (num planes %d)\n",
		device->num_planes);

	return NULL;
}

struct dp_plane *dp_device_find_plane_by_type(struct dp_device *device,
						uint32_t type, unsigned int index)
{
	unsigned int i;

	for (i = 0; i < device->num_planes; i++) {
		DP_DBG("planes %d <%d>: device->planes[%d]->type = 0x%x : 0x%x\n",
			device->num_planes, index, i, device->planes[i]->type, type);
		if (device->planes[i]->type == type) {
			if (index == 0)
				return device->planes[i];

			index--;
		}
	}

	return NULL;
}
