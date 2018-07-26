#include <errno.h>
#include <string.h>

#include "dp.h"
#include "dp_common.h"

static int dp_screen_probe(struct dp_screen *screen)
{
	struct dp_device *device = screen->device;
	drmModeConnectorPtr con;

	con = drmModeGetConnector(device->fd, screen->id);
	if (!con) {
		DP_ERR("fail : drmModeGetConnector %m\n");
		return -EFAULT;
	}
	screen->type = con->connector_type;

	if (con->connection == DRM_MODE_CONNECTED)
		screen->connected = true;
	else
		screen->connected = false;

	DP_DBG("connector %u (%s-%u)\n", con->connector_id,
		util_lookup_connector_type_name(con->connector_type),
		con->connector_type_id);

	if (false == screen->connected) {
		DP_DBG("fail : ignoring unused connector %u\n",
			con->connector_id);
		return -ENOENT;
	}

	if (con->count_modes == 0) {
		DP_DBG("fail : no valid mode for connector %u\n",
			con->connector_id);
		return -EFAULT;
	}

	memcpy(&screen->mode, &con->modes[0], sizeof(drmModeModeInfo));

	screen->width = screen->mode.hdisplay;
	screen->height = screen->mode.vdisplay;

	drmModeFreeConnector(con);

	return 1;
}

struct dp_screen *dp_screen_create(struct dp_device *device, uint32_t id)
{
	struct dp_screen *screen;
	int err;

	screen = calloc(1, sizeof(*screen));
	if (!screen){
		DP_ERR("fail : calloc for screen %m\n");
		return NULL;
	}

	screen->device = device;
	screen->id = id;

	err = dp_screen_probe(screen);
	if (0 > err) {
		free(screen);
		return NULL;
	}

	return screen;
}

void dp_screen_free(struct dp_screen *screen)
{
	if (screen)
		free(screen->name);

	free(screen);
}

int dp_screen_set(struct dp_screen *screen, struct dp_crtc *crtc,
		   struct dp_framebuffer *fb)
{
	struct dp_device *device = screen->device;
	int err;

	err = drmModeSetCrtc(device->fd, crtc->id, fb->id, 0, 0, &screen->id,
			     1, &screen->mode);
	if (err < 0) {
		DP_ERR("fail : drmModeSetCrtc %m\n");
		return -errno;
	}

	return 0;
}
