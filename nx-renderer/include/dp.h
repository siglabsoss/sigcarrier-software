#ifndef _DP_HEADER_H_
#define _DP_HEADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <xf86drmMode.h>

struct dp_device {
	int fd;

	struct dp_screen **screens;
	unsigned int num_screens;

	struct dp_crtc **crtcs;
	unsigned int num_crtcs;

	struct dp_plane **planes;
	unsigned int num_planes;
};

struct dp_crtc {
	struct dp_device *device;
	uint32_t id;
};

struct dp_plane {
	struct dp_device *device;
	struct dp_crtc *crtc;
	unsigned int type;
	uint32_t id;

	uint32_t *formats;
	unsigned int num_formats;
};

struct dp_framebuffer {
	struct dp_device *device;
	uint32_t id;
	int width;
	int height;
	uint32_t format;

	bool seperated;
	int planes;
	uint32_t handles[4];
	int pitches[4];
	uint32_t offsets[4];
	size_t sizes[4];
	void *ptrs[4];
};

struct dp_screen {
	struct dp_device *device;
	bool connected;
	uint32_t type;
	uint32_t id;

	unsigned int width;
	unsigned int height;
	char *name;

	drmModeModeInfo mode;
};



/***
 * display device APIs
 ***/
struct dp_device *dp_device_open(int fd);
void dp_device_close(struct dp_device *device);
struct dp_plane *dp_device_find_plane_by_type(struct dp_device *device,
						uint32_t type,
						unsigned int plane_index);
struct dp_plane *dp_device_find_plane_by_index(struct dp_device *device,
						unsigned int crtc_index, unsigned int plane_index);


/***
 * display CRTC APIs
 ***/
struct dp_crtc *dp_crtc_create(struct dp_device *device, uint32_t id);
void dp_crtc_free(struct dp_crtc *crtc);


/***
 * display buffer APIs
 ***/
unsigned int dp_buffer_alloc(int fd, int width, int height, int bpp,
			unsigned int *handle, int *pitch, int *size);
void dp_buffer_free(int fd, unsigned int handle);

/***
 * display framebuffer APIs
 ***/

struct dp_framebuffer *dp_framebuffer_config(struct dp_device *device,
			uint32_t format, int width, int height, bool seperate, int gem_fd, int size);

struct dp_framebuffer *dp_framebuffer_create(struct dp_device *device,
			uint32_t format, int width, int height, bool seperate);
void dp_framebuffer_free(struct dp_framebuffer *db);
int  dp_framebuffer_addfb2(struct dp_framebuffer *db);
void dp_framebuffer_delfb2(struct dp_framebuffer *db);
int dp_framebuffer_map(struct dp_framebuffer *db);
void dp_framebuffer_unmap(struct dp_framebuffer *db);

/***
 * display screen APIs
 ***/
struct dp_screen *dp_screen_create(struct dp_device *device, uint32_t id);
void dp_screen_free(struct dp_screen *screen);
int dp_screen_set(struct dp_screen *screen, struct dp_crtc *crtc,
		   struct dp_framebuffer *fb);

/***
 * display plane APIs
 ***/
struct dp_plane *dp_plane_create(struct dp_device *device, uint32_t id);
void dp_plane_free(struct dp_plane *plane);

int dp_plane_set(struct dp_plane *plane, struct dp_framebuffer *fb,
			int x, int y, int w, int h, int cx, int cy, int cw, int ch);
bool dp_plane_supports_format(struct dp_plane *plane, uint32_t format);


/***
 * display buffers APIs
 ***/

#ifdef __cplusplus
}
#endif

#endif
