/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Sungwoo, Park <swpark@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GSTCAMERARSC_H__
#define __GSTCAMERARSC_H__

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <gst/video/colorbalance.h>
#include <gst/video/video-format.h>

/* FIXME: when testing, enable USE_NATIVE_DRM_BUFFER */
#define USE_NATIVE_DRM_BUFFER

#ifndef USE_NATIVE_DRM_BUFFER
#include <tbm_bufmgr.h>
#endif

G_BEGIN_DECLS

typedef struct _GstCameraSrc GstCameraSrc;
typedef struct _GstCameraSrcClass GstCameraSrcClass;
typedef struct _GstCameraBuffer GstCameraBuffer;

struct _GstCameraBuffer {
	GstBuffer *buffer;
	int v4l2_buffer_index;
	GstCameraSrc *camerasrc;
};

#define GST_TYPE_CAMERASRC		(gst_camerasrc_get_type())
#define GST_CAMERASRC(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_CAMERASRC, GstCameraSrc))
#define GST_CAMERASRC_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_CAMERASRC, GstCameraSrcClass))
#define GST_CAMERASRC_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS(obj), GST_TYPE_CAMERASRC, GstCameraSrcClass))
#define GST_IS_CAMERASRC(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_CAMERASRC))
#define GST_IS_CAMERASRC_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_CAMERASRC))

#define MAX_BUFFER_COUNT		12

/* FIXME: Currently I am following gst-plugins-camera-v4 model of samsung
 * But, Some codes seem to be ugly,
 * So, I define following and change scheme after testing
 */
#define FOLLOWING_SAMSUNG_SCHEME

#ifndef USE_NATIVE_DRM_BUFFER
typedef struct _camerasrc_buffer_t {
	tbm_bo bo;
	int dma_fd;
	unsigned char *vaddr;
} camerasrc_buffer_t;
#endif

struct _GstCameraSrc {
	GstPushSrc parent;

	guint32 module;

	int sensor_fd;
	int clipper_subdev_fd;
	int decimator_subdev_fd;
	int csi_subdev_fd;
	int clipper_video_fd;
	int decimator_video_fd;

	gboolean is_mipi;

	/* input attribute */
	guint32 width;
	guint32 height;
	guint32 pixel_format;
	guint32 fps;

	/* crop attribute */
	guint32 crop_x;
	guint32 crop_y;
	guint32 crop_width;
	guint32 crop_height;

	/* capture attribute */
	guint32 capture_fourcc;
	guint32 capture_width;
	guint32 capture_height;
	guint32 capture_interval;
	guint32 capture_count;
	guint32 capture_jpg_quality;
	gboolean capture_provide_exif;

	/* flip attribute */
	gboolean vflip;
	gboolean hflip;

	/* buffer type */
	guint32 buffer_type;

	/* buffer */
	guint32 buffer_count;
	guint32 buffer_size;
	guint num_queued;
	GCond empty_cond;
	gboolean is_stopping;

	GstCaps *caps;
#ifdef USE_NATIVE_DRM_BUFFER
	int drm_fd;
	int gem_fds[MAX_BUFFER_COUNT];
	int flinks[MAX_BUFFER_COUNT];
	int dma_fds[MAX_BUFFER_COUNT];
	void *vaddrs[MAX_BUFFER_COUNT];
	int buffer_length[MAX_BUFFER_COUNT];
#else
	/* TBM Buffer */
	tbm_bufmgr bufmgr;
	camerasrc_buffer_t buffer[MAX_BUFFER_COUNT];
#endif
};

struct _GstCameraSrcClass {
	GstPushSrcClass parent_class;

	/* signals */
	void (*still_capture)(GstElement *element,
			      GstBuffer *main,
			      GstBuffer *sub,
			      GstBuffer *scrnl);
};

GType gst_camerasrc_get_type(void);

G_END_DECLS

#endif
