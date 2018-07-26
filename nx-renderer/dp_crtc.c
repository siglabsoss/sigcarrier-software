#include "dp.h"

struct dp_crtc *dp_crtc_create(struct dp_device *device, uint32_t id)
{
	struct dp_crtc *crtc;

	crtc = calloc(1, sizeof(*crtc));
	if (!crtc)
		return NULL;

	crtc->device = device;
	crtc->id = id;

	return crtc;
}

void dp_crtc_free(struct dp_crtc *crtc)
{
	free(crtc);
}
