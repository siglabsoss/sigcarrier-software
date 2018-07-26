/*
 * Copyright (c) 2016 Nexell Co., Ltd.
 * Author: Sungwoo, Park <swpark@nexell.co.kr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#include <linux/media.h>

#include "nx-v4l2.h"

#define DEVNAME_SIZE	64
#define DEVNODE_SIZE	64
struct nx_v4l2_entry {
	bool exist;
	bool is_mipi; /* used only in camera sensor */
	int entity_id;
	int pads;
	int links;
	char devname[DEVNAME_SIZE];
	char devnode[DEVNODE_SIZE];
};

enum {
	type_category_subdev = 0,
	type_category_video = 1,
};

#define SYSFS_PATH_SIZE	128

#define MAX_CAMERA_INSTANCE_NUM	3
#define MAX_CSI_INSTANCE_NUM	1
static struct nx_v4l2_entry_cache {
	int media_fd;
	bool cached;
	struct nx_v4l2_entry nx_sensor_subdev[MAX_CAMERA_INSTANCE_NUM];
	struct nx_v4l2_entry nx_clipper_subdev[MAX_CAMERA_INSTANCE_NUM];
	struct nx_v4l2_entry nx_decimator_subdev[MAX_CAMERA_INSTANCE_NUM];
	struct nx_v4l2_entry nx_csi_subdev[MAX_CSI_INSTANCE_NUM];
	struct nx_v4l2_entry nx_clipper_video[MAX_CAMERA_INSTANCE_NUM];
	struct nx_v4l2_entry nx_decimator_video[MAX_CAMERA_INSTANCE_NUM];
} _nx_v4l2_entry_cache = {
	.media_fd = -1,
	.cached	= false,
};

static int get_type_category(uint32_t type)
{
	switch (type) {
	case nx_sensor_subdev:
	case nx_clipper_subdev:
	case nx_decimator_subdev:
	case nx_csi_subdev:
		return type_category_subdev;
	default:
		return type_category_video;
	}
}

static uint32_t get_buf_type(uint32_t type)
{
	switch (type) {
	case nx_clipper_video:
	case nx_decimator_video:
		return V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	default:
		return V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	}
}

static void print_nx_v4l2_entry(struct nx_v4l2_entry *e)
{
	if (e->exist) {
		printf("\n");
		printf("devname\t:\t%s\n", e->devname);
		printf("devnode\t:\t%s\n", e->devnode);
		printf("entity_id\t:\t%d\n", e->entity_id);
		printf("pads\t:\t%d\n", e->pads);
		printf("links\t:\t%d\n", e->links);
	}
}

static void print_all_nx_v4l2_entry(void)
{
	int i;
	struct nx_v4l2_entry_cache *cache = &_nx_v4l2_entry_cache;
	struct nx_v4l2_entry *entry;

	if (!cache->cached) {
		fprintf(stderr, "not cached\n");
		return;
	}

	for (i = 0; i < MAX_CAMERA_INSTANCE_NUM; i++) {
		entry = &cache->nx_sensor_subdev[i];
		print_nx_v4l2_entry(entry);
	}

	for (i = 0; i < MAX_CAMERA_INSTANCE_NUM; i++) {
		entry = &cache->nx_clipper_subdev[i];
		print_nx_v4l2_entry(entry);
	}

	for (i = 0; i < MAX_CAMERA_INSTANCE_NUM; i++) {
		entry = &cache->nx_decimator_subdev[i];
		print_nx_v4l2_entry(entry);
	}

	for (i = 0; i < MAX_CSI_INSTANCE_NUM; i++) {
		entry = &cache->nx_csi_subdev[i];
		print_nx_v4l2_entry(entry);
	}

	for (i = 0; i < MAX_CAMERA_INSTANCE_NUM; i++) {
		entry = &cache->nx_clipper_video[i];
		print_nx_v4l2_entry(entry);
	}

	for (i = 0; i < MAX_CAMERA_INSTANCE_NUM; i++) {
		entry = &cache->nx_decimator_video[i];
		print_nx_v4l2_entry(entry);
	}
}

static struct nx_v4l2_entry *find_v4l2_entry(int type, int module)
{
	struct nx_v4l2_entry_cache *cache = &_nx_v4l2_entry_cache;

	switch (type) {
	case nx_sensor_subdev:
		return &cache->nx_sensor_subdev[module];
	case nx_clipper_subdev:
		return &cache->nx_clipper_subdev[module];
	case nx_decimator_subdev:
		return &cache->nx_decimator_subdev[module];
	case nx_csi_subdev:
		return &cache->nx_csi_subdev[0];
	case nx_clipper_video:
		return &cache->nx_clipper_video[module];
	case nx_decimator_video:
		return &cache->nx_decimator_video[module];
	default:
		return NULL;
	}
}

#define NX_CLIPPER_SUBDEV_NAME		"nx-clipper"
#define NX_DECIMATOR_SUBDEV_NAME	"nx-decimator"
#define NX_CSI_SUBDEV_NAME		"nx-csi"
#define NX_CLIPPER_VIDEO_NAME		"VIDEO CLIPPER"
#define NX_DECIMATOR_VIDEO_NAME		"VIDEO DECIMATOR"

static int get_type_by_name(char *type_name)
{
	if (!strncmp(type_name, NX_CLIPPER_SUBDEV_NAME,
		     strlen(NX_CLIPPER_SUBDEV_NAME))) {
		return nx_clipper_subdev;
	} else if (!strncmp(type_name, NX_DECIMATOR_SUBDEV_NAME,
			    strlen(NX_DECIMATOR_SUBDEV_NAME))) {
		return nx_decimator_subdev;
	} else if (!strncmp(type_name, NX_CSI_SUBDEV_NAME,
			    strlen(NX_CSI_SUBDEV_NAME))) {
		return nx_csi_subdev;
	} else if (!strncmp(type_name, NX_CLIPPER_VIDEO_NAME,
			    strlen(NX_CLIPPER_VIDEO_NAME))) {
		return nx_clipper_video;
	} else if (!strncmp(type_name, NX_DECIMATOR_VIDEO_NAME,
			    strlen(NX_DECIMATOR_VIDEO_NAME))) {
		return nx_decimator_video;
	} else {
		/* fprintf(stderr, "can't find type for name %s\n", type_name); */
		return -EINVAL;
	}
}

static int get_sensor_info(char *name, int *module)
{
	int i;
	struct nx_v4l2_entry *e = &_nx_v4l2_entry_cache.nx_sensor_subdev[0];

	for (i = 0; i < MAX_CAMERA_INSTANCE_NUM; i++, e++) {
		if (e->exist &&
			!strncmp(name, e->devname, strlen(e->devname))) {
			*module = i;
			return nx_sensor_subdev;
		}
	}

	return -ENODEV;
}

static struct nx_v4l2_entry *find_v4l2_entry_by_name(char *name)
{
	int type;
	int module;
	char type_name[DEVNAME_SIZE] = {0, };
	char sensor_name[DEVNAME_SIZE] = {0, };

	memset(type_name, 0, DEVNAME_SIZE);
	memset(sensor_name, 0, DEVNAME_SIZE);

	sscanf(name, "%[^0-9]%d", type_name, &module);

	type = get_type_by_name(type_name);
	if (type < 0) {
		type = get_sensor_info(name, &module);
		if (type < 0)
			return NULL;
	}

	return find_v4l2_entry(type, module);
}

/* camera sensor sysfs entry
 * /sys/devices/platform/camerasensor[MODULE]/info
 * ex> /sys/devices/platform/camerasensor0/info
 */

static void enum_camera_sensor(void)
{
	int sys_fd;
	int i;
	char sysfs_path[64] = {0, };
	struct nx_v4l2_entry *e = &_nx_v4l2_entry_cache.nx_sensor_subdev[0];

	for (i = 0; i < MAX_CAMERA_INSTANCE_NUM; i++, e++) {
		sprintf(sysfs_path,
			"/sys/devices/platform/camerasensor%d/info",i);
		sys_fd = open(sysfs_path, O_RDONLY);
		if (sys_fd < 0) {
			e->exist = false;
		} else {
			char buf[512] = {0, };
			int size = read(sys_fd, buf, sizeof(buf));

			close(sys_fd);
			if (size < 0) {
				fprintf(stderr, "failed to read %s\n",
					sysfs_path);
				e->exist = false;
			} else {
				if (!strcmp("no exist", buf)) {
					e->exist = false;
				} else {
					char *c;

					e->exist = true;
					c = &buf[strlen("is_mipi:")];
					e->is_mipi = *c - '0';
					c += 7; /* ,name: */
					strncpy(e->devname, c, DEVNAME_SIZE);
				}
			}
		}
	}
}

static int enum_all_v4l2_devices(void)
{
	char *cur_dir;
	struct dirent **items;
	int nitems;
	int i;

	enum_camera_sensor();

	cur_dir = getcwd(NULL, 0);
	chdir("/sys/class/video4linux");

	nitems = scandir(".", &items, NULL, alphasort);

	for (i = 0; i < nitems; i++) {
		struct stat fstat;
		char entry_sys_path[SYSFS_PATH_SIZE];
		char entry_name[DEVNAME_SIZE];
		int fd;
		int read_count;
		struct nx_v4l2_entry *e;

		if ((!strcmp(items[i]->d_name, ".")) ||
		    (!strcmp(items[i]->d_name, "..")))
			continue;

		if (lstat(items[i]->d_name, &fstat) < 0) {
			fprintf(stderr, "lstat %s ", items[i]->d_name);
			continue;
		}

		memset(entry_sys_path, 0, SYSFS_PATH_SIZE);
		memset(entry_name, 0, DEVNAME_SIZE);

		sprintf(entry_sys_path, "%s/name", items[i]->d_name);

		fd = open(entry_sys_path, O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, "can't open %s", entry_sys_path);
			continue;
		}

		read_count = read(fd, entry_name, DEVNAME_SIZE - 1);
		close(fd);

		if (read_count <= 0) {
			fprintf(stderr, "can't read %s\n", entry_sys_path);
			continue;
		}

		e = find_v4l2_entry_by_name(entry_name);
		if (!e) {
			/* fprintf(stderr, "can't v4l2 entry for %s\n", */
			/* 	entry_name); */
			continue;
		}

		e->exist = true;
		sprintf(e->devnode, "/dev/%s", items[i]->d_name);
	}

	if (cur_dir) {
		chdir(cur_dir);
		free(cur_dir);
	}

	_nx_v4l2_entry_cache.cached = true;

	return 0;
}

static int enum_all_media_entities(void)
{
	int ret;
	int index;
	struct media_entity_desc entity;
	struct nx_v4l2_entry_cache *cache = &_nx_v4l2_entry_cache;
	struct nx_v4l2_entry *entry;

	if (cache->cached == false) {
		fprintf(stderr, "%s: not cached\n", __func__);
		return -EAGAIN;
	}

	if (cache->media_fd < 0) {
		cache->media_fd = open("/dev/media0", O_RDWR);
		if (cache->media_fd < 0) {
			fprintf(stderr, "failed to open media device\n");
			return -ENODEV;
		}
	}

	index = 0;
	do {
		entity.id = index | MEDIA_ENT_ID_FLAG_NEXT;
		ret = ioctl(cache->media_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
		if (ret == 0) {
			entry = find_v4l2_entry_by_name(entity.name);
			if (entry) {
				entry->entity_id = entity.id;
				entry->pads = entity.pads;
				entry->links = entity.links;
			}
		}
		index++;
	} while (ret == 0);

	return 0;
}

/****************************************************************
 * public api
 */
int nx_v4l2_open_device(int type, int module)
{
	struct nx_v4l2_entry *entry = NULL;

	if (_nx_v4l2_entry_cache.cached == false) {
		enum_all_v4l2_devices();
		enum_all_media_entities();
		/* print_all_nx_v4l2_entry(); */
	}

	entry = find_v4l2_entry(type, module);
	if (entry) {
		int fd = open(entry->devnode, O_RDWR);

		if (fd < 0)
			fprintf(stderr, "open failed for %s\n", entry->devname);
		return fd;
	} else {
		fprintf(stderr, "can't find device for type %d, module %d\n",
			type, module);
		return -ENODEV;
	}
}

void nx_v4l2_cleanup(void)
{
	struct nx_v4l2_entry_cache *cache = &_nx_v4l2_entry_cache;

	if (cache->media_fd >= 0) {
		close(cache->media_fd);
		cache->media_fd = -1;
	}
	cache->cached = false;
}

bool nx_v4l2_is_mipi_camera(int module)
{
	struct nx_v4l2_entry *e;

	e = find_v4l2_entry(nx_sensor_subdev, module);
	if (e && e->is_mipi)
		return true;

	return false;
}

int enum_link(int id, int pads, int links, struct media_links_enum *enumlink)
{
	int ret;
	int i;

	memset(enumlink, 0, sizeof(*enumlink));

	enumlink->entity = id;
	enumlink->pads = malloc(sizeof(struct media_pad_desc) * pads);
	enumlink->links = malloc(sizeof(struct media_link_desc) * links);

	ret = ioctl(_nx_v4l2_entry_cache.media_fd,
			MEDIA_IOC_ENUM_LINKS, enumlink);
	if (ret < 0) {
		fprintf(stderr,
			"failed to enum link foir %d\n", id);
		free(enumlink->pads);
		free(enumlink->links);
		return -EINVAL;
	}

	struct media_pad_desc *pad_desc = enumlink->pads;
	struct media_link_desc *link_desc = enumlink->links;

	for (i = 0; i < pads; i++) {
		fprintf(stdout,
			"(%d, %s) ",
			pad_desc->index,
			(pad_desc->flags & MEDIA_PAD_FL_SINK)
			? "INPUT" : "OUTPUT");
		pad_desc++;
	}

	for (i = 0; i < links; i++) {
		fprintf(stdout,
			"[%x:%x] ------------> [%x:%x] ",
			link_desc->source.entity,
			link_desc->source.index,
			link_desc->sink.entity,
			link_desc->sink.index);
		if (link_desc->flags & MEDIA_LNK_FL_ENABLED) {
			fprintf(stdout, "ACTIVE");
		} else {
			fprintf(stdout, "INACTIVE");
		}
		link_desc++;
	}

	fprintf(stdout, "\n");
	return 0;
}

int nx_v4l2_link(bool link, int module, int src_type, int src_pad,
	int sink_type, int sink_pad)
{
	struct nx_v4l2_entry *src_entry;
	struct nx_v4l2_entry *sink_entry;
	struct media_link_desc desc;

	src_entry = find_v4l2_entry(src_type, module);
	if (!src_entry) {
		fprintf(stderr,
			"can't find src v4l2 device for module %d, type %d\n",
			module, src_type);
		return -ENODEV;
	}

	if (src_pad >= src_entry->pads) {
		fprintf(stderr,
			"invalid src pad %d/%d\n", src_pad, src_entry->pads);
		return -EINVAL;
	}

	sink_entry = find_v4l2_entry(sink_type, module);
	if (!sink_entry) {
		fprintf(stderr,
			"can't find sink v4l2 device for module %d, type %d\n",
			module, sink_type);
		return -ENODEV;
	}

	if (sink_pad >= sink_entry->pads) {
		fprintf(stderr,
			"invalid sink pad %d/%d\n", sink_pad, sink_entry->pads);
		return -EINVAL;
	}

	/* This is for debugging */
#if 0
	{
		struct media_links_enum link_enum;
		enum_link(src_entry->entity_id, src_entry->pads,
			  src_entry->links, &link_enum);
		enum_link(sink_entry->entity_id, sink_entry->pads,
			  sink_entry->links, &link_enum);
	}
#endif

	memset(&desc, 0, sizeof(desc));

	if (link)
		desc.flags |= MEDIA_LNK_FL_ENABLED;
	else
		desc.flags &= ~MEDIA_LNK_FL_ENABLED;

	desc.source.entity = src_entry->entity_id;
	desc.source.index = src_pad;
	desc.source.flags = MEDIA_PAD_FL_SOURCE;

	desc.sink.entity = sink_entry->entity_id;
	desc.sink.index = sink_pad;
	desc.sink.flags = MEDIA_PAD_FL_SINK;

	return ioctl(_nx_v4l2_entry_cache.media_fd, MEDIA_IOC_SETUP_LINK,
		     &desc);
}

static int subdev_set_format(int fd, uint32_t w, uint32_t h, uint32_t format)
{
	struct v4l2_subdev_format fmt;
	bzero(&fmt, sizeof(fmt));
	fmt.pad = 0;
	fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
	fmt.format.code = format;
	fmt.format.width = w;
	fmt.format.height = h;
	fmt.format.field = V4L2_FIELD_NONE;
	return ioctl(fd, VIDIOC_SUBDEV_S_FMT, &fmt);
}

static int video_set_format(int fd, uint32_t w, uint32_t h, uint32_t format,
			    uint32_t buf_type)
{
	struct v4l2_format v4l2_fmt;

	bzero(&v4l2_fmt, sizeof(v4l2_fmt));
	v4l2_fmt.type = buf_type;
	v4l2_fmt.fmt.pix_mp.width = w;
	v4l2_fmt.fmt.pix_mp.height = h;
	v4l2_fmt.fmt.pix_mp.pixelformat = format;
	v4l2_fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
	return ioctl(fd, VIDIOC_S_FMT, &v4l2_fmt);
}

static int video_set_format_mmap(int fd, uint32_t w, uint32_t h,
				 uint32_t format, uint32_t buf_type)
{
	struct v4l2_format v4l2_fmt;

	bzero(&v4l2_fmt, sizeof(v4l2_fmt));
	v4l2_fmt.type = buf_type;
	v4l2_fmt.fmt.pix.width = w;
	v4l2_fmt.fmt.pix.height = h;
	v4l2_fmt.fmt.pix.pixelformat = format;
	v4l2_fmt.fmt.pix.field = V4L2_FIELD_ANY;
	return ioctl(fd, VIDIOC_S_FMT, &v4l2_fmt);
}

int nx_v4l2_set_format(int fd, int type, uint32_t w, uint32_t h,
	uint32_t format)
{
	if (get_type_category(type) == type_category_subdev)
		return subdev_set_format(fd, w, h, format);
	else
		return video_set_format(fd, w, h, format, get_buf_type(type));
}

int nx_v4l2_set_format_mmap(int fd, int type, uint32_t w, uint32_t h,
			    uint32_t format)
{
	if (get_type_category(type) == type_category_subdev)
		return subdev_set_format(fd, w, h, format);
	else
		return video_set_format_mmap(fd, w, h, format,
					     V4L2_BUF_TYPE_VIDEO_CAPTURE);
}

static int subdev_get_format(int fd, uint32_t *w, uint32_t *h, uint32_t *format)
{
	int ret;
	struct v4l2_subdev_format fmt;

	bzero(&fmt, sizeof(fmt));

	ret = ioctl(fd, VIDIOC_SUBDEV_G_FMT, &fmt);
	if (ret)
		return ret;

	*w = fmt.format.width;
	*h = fmt.format.height;
	*format = fmt.format.code;

	return 0;
}

static int video_get_format(int fd, uint32_t *w, uint32_t *h, uint32_t *format,
			    uint32_t buf_type)
{
	int ret;
	struct v4l2_format v4l2_fmt;

	bzero(&v4l2_fmt, sizeof(v4l2_fmt));
	v4l2_fmt.type = buf_type;
	v4l2_fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
	ret = ioctl(fd, VIDIOC_G_FMT, &v4l2_fmt);
	if (ret)
		return ret;

	*w = v4l2_fmt.fmt.pix_mp.width;
	*h = v4l2_fmt.fmt.pix_mp.height;
	*format = v4l2_fmt.fmt.pix_mp.pixelformat;
	return 0;
}

int nx_v4l2_get_format(int fd, int type, uint32_t *w, uint32_t *h,
		       uint32_t *format)
{
	if (get_type_category(type) == type_category_subdev)
		return subdev_get_format(fd, w, h, format);
	else
		return video_get_format(fd, w, h, format, get_buf_type(type));
}

static int subdev_set_crop(int fd, uint32_t x, uint32_t y, uint32_t w,
			   uint32_t h)
{
	struct v4l2_subdev_crop crop;

	bzero(&crop, sizeof(crop));
	crop.rect.left = x;
	crop.rect.top = y;
	crop.rect.width = w;
	crop.rect.height = h;
	return ioctl(fd, VIDIOC_SUBDEV_S_CROP, &crop);
}

static int video_set_crop(int fd, uint32_t x, uint32_t y, uint32_t w,
			  uint32_t h, uint32_t buf_type)
{
	struct v4l2_crop crop;

	bzero(&crop, sizeof(crop));
	crop.type = buf_type;
	crop.c.left = x;
	crop.c.top = y;
	crop.c.width = w;
	crop.c.height = h;
	return ioctl(fd, VIDIOC_S_CROP, &crop);
}

int nx_v4l2_set_crop(int fd, int type, uint32_t x, uint32_t y,
		     uint32_t w, uint32_t h)
{
	if (get_type_category(type) == type_category_subdev)
		return subdev_set_crop(fd, x, y, w, h);
	else
		return video_set_crop(fd, x, y, w, h, get_buf_type(type));
}

int nx_v4l2_set_crop_mmap(int fd, int type, uint32_t x, uint32_t y,
			  uint32_t w, uint32_t h)
{
	if (get_type_category(type) == type_category_subdev)
		return subdev_set_crop(fd, x, y, w, h);
	else
		return video_set_crop(fd, x, y, w, h,
				      V4L2_BUF_TYPE_VIDEO_CAPTURE);
}

static int subdev_get_crop(int fd, uint32_t *x, uint32_t *y, uint32_t *w,
			   uint32_t *h)
{
	int ret;
	struct v4l2_subdev_crop crop;

	bzero(&crop, sizeof(crop));
	ret = ioctl(fd, VIDIOC_SUBDEV_G_CROP, &crop);
	if (ret)
		return ret;

	*x = crop.rect.left;
	*y = crop.rect.top;
	*w = crop.rect.width;
	*h = crop.rect.height;
	return 0;
}

static int video_get_crop(int fd, uint32_t *x, uint32_t *y, uint32_t *w,
			  uint32_t *h, uint32_t buf_type)
{
	int ret;
	struct v4l2_crop crop;

	bzero(&crop, sizeof(crop));
	crop.type = buf_type;
	ret = ioctl(fd, VIDIOC_G_CROP, &crop);
	if (ret)
		return ret;

	*x = crop.c.left;
	*y = crop.c.top;
	*w = crop.c.width;
	*h = crop.c.height;
	return 0;
}

int nx_v4l2_get_crop(int fd, int type, uint32_t *x, uint32_t *y, uint32_t *w,
		     uint32_t *h)
{
	if (get_type_category(type) == type_category_subdev)
		return subdev_get_crop(fd, x, y, w, h);
	else
		return video_get_crop(fd, x, y, w, h, get_buf_type(type));
}

int nx_v4l2_set_ctrl(int fd, int type, uint32_t ctrl_id, int value)
{
	struct v4l2_control ctrl;

	bzero(&ctrl, sizeof(ctrl));
	ctrl.id = ctrl_id;
	ctrl.value = value;
	return ioctl(fd, VIDIOC_S_CTRL, &ctrl);
}

int nx_v4l2_get_ctrl(int fd, int type, uint32_t ctrl_id, int *value)
{
	int ret;
	struct v4l2_control ctrl;

	bzero(&ctrl, sizeof(ctrl));
	ctrl.id = ctrl_id;
	ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
	if (ret)
		return ret;
	*value = ctrl.value;
	return 0;
}

int nx_v4l2_reqbuf(int fd, int type, int count)
{
	struct v4l2_requestbuffers req;

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	bzero(&req, sizeof(req));
	req.count = count;
	req.memory = V4L2_MEMORY_DMABUF;
	req.type = get_buf_type(type);
	return ioctl(fd, VIDIOC_REQBUFS, &req);
}

int nx_v4l2_reqbuf_mmap(int fd, int type, int count)
{
	struct v4l2_requestbuffers req;

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	bzero(&req, sizeof(req));
	req.count = count;
	req.memory = V4L2_MEMORY_MMAP;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	return ioctl(fd, VIDIOC_REQBUFS, &req);
}

#define MAX_PLANES	3
int nx_v4l2_qbuf(int fd, int type, int plane_num, int index, int *fds,
		 int *sizes)
{
	struct v4l2_buffer v4l2_buf;
	struct v4l2_plane planes[MAX_PLANES];
	int i;

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	if (plane_num > MAX_PLANES) {
		fprintf(stderr, "plane_num(%d) is over MAX_PLANES\n",
			plane_num);
		return -EINVAL;
	}

	bzero(&v4l2_buf, sizeof(v4l2_buf));
	v4l2_buf.m.planes = planes;
	v4l2_buf.type = get_buf_type(type);
	v4l2_buf.memory = V4L2_MEMORY_DMABUF;
	v4l2_buf.index = index;
	v4l2_buf.length = plane_num;
	for (i = 0; i < plane_num; i++) {
		v4l2_buf.m.planes[i].m.fd = fds[i];
		v4l2_buf.m.planes[i].length = sizes[i];
	}

	return ioctl(fd, VIDIOC_QBUF, &v4l2_buf);
}

int nx_v4l2_qbuf_mmap(int fd, int type, int index)
{
	struct v4l2_buffer v4l2_buf;
	int i;

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	bzero(&v4l2_buf, sizeof(v4l2_buf));
	v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf.memory = V4L2_MEMORY_MMAP;
	v4l2_buf.index = index;

	return ioctl(fd, VIDIOC_QBUF, &v4l2_buf);
}

int nx_v4l2_dqbuf(int fd, int type, int plane_num, int *index)
{
	int ret;
	struct v4l2_buffer v4l2_buf;
	struct v4l2_plane planes[MAX_PLANES];

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	if (plane_num > MAX_PLANES) {
		fprintf(stderr, "plane_num(%d) is over MAX_PLANES\n",
			plane_num);
		return -EINVAL;
	}

	bzero(&v4l2_buf, sizeof(v4l2_buf));
	v4l2_buf.m.planes = planes;
	v4l2_buf.type = get_buf_type(type);
	v4l2_buf.memory = V4L2_MEMORY_DMABUF;
	v4l2_buf.length = plane_num;

	ret = ioctl(fd, VIDIOC_DQBUF, &v4l2_buf);
	if (ret)
		return ret;

	*index = v4l2_buf.index;

	return 0;
}

int nx_v4l2_dqbuf_with_timestamp(int fd, int type, int plane_num, int *index,
				 struct timeval *timeval)
{
	int ret;
	struct v4l2_buffer v4l2_buf;
	struct v4l2_plane planes[MAX_PLANES];

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	if (plane_num > MAX_PLANES) {
		fprintf(stderr, "plane_num(%d) is over MAX_PLANES\n",
			plane_num);
		return -EINVAL;
	}

	bzero(&v4l2_buf, sizeof(v4l2_buf));
	v4l2_buf.m.planes = planes;
	v4l2_buf.type = get_buf_type(type);
	v4l2_buf.memory = V4L2_MEMORY_DMABUF;
	v4l2_buf.length = plane_num;

	ret = ioctl(fd, VIDIOC_DQBUF, &v4l2_buf);
	if (ret)
		return ret;

	*index = v4l2_buf.index;

	memcpy(timeval, &v4l2_buf.timestamp, sizeof(*timeval));

	return 0;
}

int nx_v4l2_dqbuf_mmap(int fd, int type, int *index)
{
	int ret;
	struct v4l2_buffer v4l2_buf;

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	bzero(&v4l2_buf, sizeof(v4l2_buf));
	v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(fd, VIDIOC_DQBUF, &v4l2_buf);
	if (ret)
		return ret;

	*index = v4l2_buf.index;

	return 0;
}

int nx_v4l2_dqbuf_mmap_with_timestamp(int fd, int type, int *index,
				      struct timeval *timeval)
{
	int ret;
	struct v4l2_buffer v4l2_buf;

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	bzero(&v4l2_buf, sizeof(v4l2_buf));
	v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(fd, VIDIOC_DQBUF, &v4l2_buf);
	if (ret)
		return ret;

	*index = v4l2_buf.index;

	memcpy(timeval, &v4l2_buf.timestamp, sizeof(*timeval));

	return 0;
}

int nx_v4l2_streamon(int fd, int type)
{
	uint32_t buf_type;

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	buf_type = get_buf_type(type);
	return ioctl(fd, VIDIOC_STREAMON, &buf_type);
}

int nx_v4l2_streamon_mmap(int fd, int type)
{
	uint32_t buf_type;

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	return ioctl(fd, VIDIOC_STREAMON, &buf_type);
}

int nx_v4l2_streamoff(int fd, int type)
{
	uint32_t buf_type;
	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	buf_type = get_buf_type(type);
	return ioctl(fd, VIDIOC_STREAMOFF, &buf_type);
}

int nx_v4l2_streamoff_mmap(int fd, int type)
{
	uint32_t buf_type;
	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	return ioctl(fd, VIDIOC_STREAMOFF, &buf_type);
}

int nx_v4l2_query_buf_mmap(int fd, int type, int index,
			   struct v4l2_buffer *v4l2_buf)
{
	int ret;
	/* struct v4l2_buffer v4l2_buf; */

	if (get_type_category(type) == type_category_subdev)
		return -EINVAL;

	bzero(v4l2_buf, sizeof(*v4l2_buf));
	v4l2_buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf->memory = V4L2_MEMORY_MMAP;
	v4l2_buf->index = index;
	return ioctl(fd, VIDIOC_QUERYBUF, v4l2_buf);
}

int nx_v4l2_set_parm(int fd, int type, struct v4l2_streamparm *parm)
{
	uint32_t buf_type;
	parm->type = get_buf_type(type);
	return ioctl(fd, VIDIOC_S_PARM, parm);
}
