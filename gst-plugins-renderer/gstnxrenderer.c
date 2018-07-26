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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <xf86drm.h>
#include <drm_fourcc.h>

#include "nx-drm-allocator.h"

#include "gstnxrenderer.h"

/* FIXME
 * this define must be moved to nx-renderer library
 */
static const uint32_t dp_formats[] = {

	/* 1 plane */
	DRM_FORMAT_YUYV,
	DRM_FORMAT_YVYU,
	DRM_FORMAT_UYVY,
	DRM_FORMAT_VYUY,

	/* 2 plane */
	DRM_FORMAT_NV12, /* NV12 : 2x2 subsampled Cr:Cb plane */
	DRM_FORMAT_NV21, /* NV21 : 2x2 subsampled Cb:Cr plane */
	DRM_FORMAT_NV16, /* NV16 : 2x1 subsampled Cr:Cb plane */
	DRM_FORMAT_NV61, /* NV61 : 2x1 subsampled Cb:Cr plane */

	/* 3 plane */
	DRM_FORMAT_YUV420, /* YU12 : 2x2 subsampled Cb (1) and Cr (2) planes */
	DRM_FORMAT_YVU420, /* YV12 : 2x2 subsampled Cr (1) and Cb (2) planes */
	DRM_FORMAT_YUV422, /* YU16 : 2x1 subsampled Cb (1) and Cr (2) planes */
	DRM_FORMAT_YVU422, /* YV16 : 2x1 subsampled Cr (1) and Cb (2) planes */
	DRM_FORMAT_YUV444, /* YU24 : non-subsampled Cb (1) and Cr (2) planes */
	DRM_FORMAT_YVU444, /* YV24 : non-subsampled Cr (1) and Cb (2) planes */

	/* RGB 1 buffer */
	DRM_FORMAT_RGB565,
	DRM_FORMAT_BGR565,
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR8888,
};

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE("sink",
	GST_PAD_SINK,
	GST_PAD_ALWAYS,
	GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC(gst_nx_renderer_debug);
#define GST_CAT_DEFAULT gst_nx_renderer_debug

#define DEFAULT_SYNC	FALSE

#define _do_init \
	GST_DEBUG_CATEGORY_INIT(gst_nx_renderer_debug, "nxrenderer", 0, "nxrenderer element");
#define gst_nx_renderer_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE(GstNxRenderer, gst_nx_renderer, GST_TYPE_BASE_SINK,
			_do_init);

static void gst_nx_renderer_set_property(GObject *object,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *pspec);
static void gst_nx_renderer_get_property(GObject *object,
					 guint prop_id,
					 GValue *value,
					 GParamSpec *pspec);
static void gst_nx_renderer_finalize(GObject *obj);

static GstStateChangeReturn gst_nx_renderer_change_state(GstElement *element,
	GstStateChange transition);

static GstFlowReturn gst_nx_renderer_preroll(GstBaseSink *bsink,
					     GstBuffer *buffer);
static GstFlowReturn gst_nx_renderer_render(GstBaseSink *bsink,
					     GstBuffer *buffer);
static gboolean gst_nx_renderer_event(GstBaseSink *bsink, GstEvent *event);
static gboolean gst_nx_renderer_query(GstBaseSink *bsink, GstQuery *query);

static void gst_nx_renderer_class_init(GstNxRendererClass *klass)
{
	GObjectClass *gobject_class;
	GstElementClass *gstelement_class;
	GstBaseSinkClass *gstbase_sink_class;

	GST_DEBUG("ENTERED");

	gobject_class = G_OBJECT_CLASS(klass);
	gstelement_class = GST_ELEMENT_CLASS(klass);
	gstbase_sink_class = GST_BASE_SINK_CLASS(klass);

	gobject_class->set_property = gst_nx_renderer_set_property;
	gobject_class->get_property = gst_nx_renderer_get_property;
	gobject_class->finalize = gst_nx_renderer_finalize;

	/* TODO : install property */

	gst_element_class_set_static_metadata(gstelement_class,
		"Nx Renderer",
		"Sink",
		"Renderer to display GEM FD to Nexell KMS Driver",
		"Sungwoo Park <swpark@nexell.co.kr> ");
	gst_element_class_add_pad_template(gstelement_class,
		gst_static_pad_template_get(&sinktemplate));

	gstelement_class->change_state =
		GST_DEBUG_FUNCPTR(gst_nx_renderer_change_state);

	gstbase_sink_class->event =
		GST_DEBUG_FUNCPTR(gst_nx_renderer_event);
	gstbase_sink_class->preroll =
		GST_DEBUG_FUNCPTR(gst_nx_renderer_preroll);
	gstbase_sink_class->render =
		GST_DEBUG_FUNCPTR(gst_nx_renderer_render);
	gstbase_sink_class->query =
		GST_DEBUG_FUNCPTR(gst_nx_renderer_query);

	GST_DEBUG("LEAVED");
}
static guint32
_get_pixel_format(GstNxRenderer *me, guint32 mm_format)
{
	guint32 drm_format = -1;
	GST_DEBUG_OBJECT(me, "mm_format = %d", mm_format);
	switch(mm_format) {
		case MM_PIXEL_FORMAT_YUYV:
			drm_format = 0;//DRM_FORMAT_YUYV;
			break;
		case MM_PIXEL_FORMAT_I420:
			drm_format = 8;//DRM_FORMAT_YUV420;
		default:
			break;
	}
	GST_DEBUG_OBJECT(me, "drm_format = %d", drm_format);
	return drm_format;
}

static struct dp_device *dp_device_init(int fd)
{
	struct dp_device *device;
	int err;

	err = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	if (err < 0)
		GST_ERROR("drmSetClientCap() failed: %d %m\n", err);

	device = dp_device_open(fd);
	if (!device) {
		GST_ERROR("fail : device open() %m\n");
		return NULL;
	}

	return device;
}

/* FIXME: this function must be moved to nx-renderer library */
static int choose_format(struct dp_plane *plane, int select)
{
	int format;
	int size;

	size = ARRAY_SIZE(dp_formats);

	if (select > (size-1)) {
		GST_ERROR("fail : not support format index (%d) over size %d\n",
			  select, size);
		return -EINVAL;
	}
	format = dp_formats[select];

	if (!dp_plane_supports_format(plane, format)) {
		GST_ERROR("fail : not support %s\n", dp_forcc_name(format));
		return -EINVAL;
	}

	GST_DEBUG("format: %s\n", dp_forcc_name(format));
	return format;
}

static struct dp_framebuffer *framebuffer_alloc(struct dp_device *device,
						int format,
						MMVideoBuffer *mm_buf)
{
	struct dp_framebuffer *fb;
	int i;
	uint32_t offset;
	int err;
	int gem;

	fb = calloc(1, sizeof(struct dp_framebuffer));
	if (!fb) {
		GST_ERROR("failed to alloc dp_framebuffer");
		return NULL;
	}

	gem = import_gem_from_flink(device->fd, mm_buf->handle.gem[0]);
	if (gem < 0) {
		GST_ERROR("failed to import gem from flink(%d)",
			  mm_buf->handle.gem[0]);
		dp_framebuffer_free(fb);
		return NULL;
	}

	fb->device = device;
	fb->width = mm_buf->width[0];
	fb->height = mm_buf->height[0];
	fb->format = format;

	fb->seperated = mm_buf->handle_num > 1 ? true : false;
	fb->planes = mm_buf->handle_num;

	for (i = 0; i < mm_buf->handle_num; i++) {
		fb->sizes[i] = mm_buf->size[i];
	}

	/* FIXME; currently supports only YUV420 format */
	offset = 0;
	//for (i = 0; i < 3; i ++) {
	for (i = 0; i < mm_buf->plane_num; i ++) {
		/* fb->handles[i] = mm_buf->handle.gem[0]; */
		fb->handles[i] = gem;
		fb->pitches[i] = mm_buf->stride_width[i];
		fb->ptrs[i] = NULL;
		fb->offsets[i] = offset;
		offset += mm_buf->stride_width[i] * mm_buf->stride_height[i];
	}

	err = dp_framebuffer_addfb2(fb);
	if (err < 0) {
		GST_ERROR("failed to addfb2");
		dp_framebuffer_free(fb);
		return NULL;
	}

	return fb;
}

static struct dp_framebuffer *dp_buffer_init(struct dp_device *device,
					     int display_index,
					     int plane_index,
					     int op_format,
					     MMVideoBuffer *mm_buf)
{
	struct dp_plane *plane;
	int format;

	plane = dp_device_find_plane_by_index(device, display_index,
					      plane_index);
	if (!plane) {
		GST_ERROR("no overlay plane found for disp %d, plane %d",
			  display_index, plane_index);
		return NULL;
	}

	format = choose_format(plane, op_format);
	if (format < 0) {
		GST_ERROR("no matching format found for op_format %d",
			  op_format);
		return NULL;
	}

	return framebuffer_alloc(device, format, mm_buf);
}

static int dp_plane_update(struct dp_device *device, struct dp_framebuffer *fb,
			   int display_index, int plane_index)
{
	struct dp_plane *plane;

	plane = dp_device_find_plane_by_index(device, display_index,
					      plane_index);
	if (!plane) {
		GST_ERROR("no overlay plane found for disp %d, plane %d",
			  display_index, plane_index);
		return -EINVAL;
	}

	/* TODO : source crop, dest position */
	return dp_plane_set(plane, fb,
			    0, 0, fb->width, fb->height,
			    0, 0, fb->width, fb->height);
}

static void gst_nx_renderer_init(GstNxRenderer *me)
{
	int i;
	GST_DEBUG("ENTERED");
	GST_DEBUG_OBJECT(me, "init");

	me->drm_fd = open("/dev/dri/card0", O_RDWR);
	if (me->drm_fd < 0)
		GST_ERROR_OBJECT(me, "failed to open drm device");

	me->dp_device = dp_device_init(me->drm_fd);
	if (!me->dp_device)
		GST_ERROR_OBJECT(me, "failed to dp_device_init");

	for (i = 0; i < MAX_BUFFER_COUNT; i++)
		me->fb[i] = NULL;

	gst_base_sink_set_sync(GST_BASE_SINK(me), DEFAULT_SYNC);
	GST_DEBUG("LEAVED");
}

static void gst_nx_renderer_finalize(GObject *obj)
{
	int i;
	GstNxRenderer *me;

	me = GST_NX_RENDERER(obj);

	GST_DEBUG("ENTERED");

	free_gem(me->drm_fd, me->fb[0]->handles[0]);
	for (i = 0; i < MAX_BUFFER_COUNT; i++)
		if (me->fb[i])
			dp_framebuffer_delfb2(me->fb[i]);

	if (me->dp_device)
		dp_device_close(me->dp_device);
	if (me->drm_fd >= 0)
		close(me->drm_fd);

	GST_DEBUG_OBJECT(obj, "finalize");
	G_OBJECT_CLASS(parent_class)->finalize(obj);
	GST_DEBUG("LEAVED");
}

static void gst_nx_renderer_set_property(GObject *object, guint prop_id,
					 const GValue *value, GParamSpec *pspec)
{
	GstNxRenderer *me;

	me = GST_NX_RENDERER(object);

	GST_DEBUG_OBJECT(me, "set property");

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void gst_nx_renderer_get_property(GObject *object, guint prop_id,
					 GValue *value,
					 GParamSpec *pspec)
{
	GstNxRenderer *me;

	me = GST_NX_RENDERER(object);

	GST_DEBUG_OBJECT(me, "get property");

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean gst_nx_renderer_event(GstBaseSink *bsink, GstEvent *event)
{
	GstNxRenderer *me = GST_NX_RENDERER(bsink);

	GST_DEBUG_OBJECT(me, "event");

	return GST_BASE_SINK_CLASS(parent_class)->event(bsink, event);
}

static GstFlowReturn gst_nx_renderer_preroll(GstBaseSink *bsink,
					     GstBuffer *buffer)
{
	GstNxRenderer *me = GST_NX_RENDERER(bsink);

	GST_DEBUG_OBJECT(me, "preroll buffer");

	return GST_FLOW_OK;
}

static GstFlowReturn gst_nx_renderer_render(GstBaseSink *bsink, GstBuffer *buf)
{
	GstNxRenderer *me = GST_NX_RENDERER(bsink);
	GstMemory *meta_block = NULL;
	MMVideoBuffer *mm_buf = NULL;
	GstMapInfo info;
	GstFlowReturn ret = GST_FLOW_OK;

	GST_OBJECT_LOCK(me);

	GST_DEBUG_OBJECT(me, "render buffer");

	/* gst_buffer_map(buf, &info, GST_MAP_READ); */
	/* gst_util_dump_mem(info.data, info.size); */
	/* gst_buffer_unmap(buf, &info); */

	memset(&info, 0, sizeof(GstMapInfo));

	meta_block = gst_buffer_peek_memory(buf, 0);
	gst_memory_map(meta_block, &info, GST_MAP_READ);

	mm_buf = (MMVideoBuffer *)info.data;
	if (!mm_buf) {
		GST_ERROR_OBJECT(me, "failed to get MMVideoBuffer!");
	} else {
		GST_DEBUG_OBJECT(me, "get MMVideoBuffer");
		GST_DEBUG_OBJECT(me,
			"type: 0x%x, width: %d, height: %d, plane_num: %d, handle_num: %d, index: %d, format : %d",
			mm_buf->type, mm_buf->width[0],
			mm_buf->height[0], mm_buf->plane_num,
			mm_buf->handle_num, mm_buf->buffer_index,
			mm_buf->format);

		if (!me->fb[mm_buf->buffer_index]) {
			struct dp_framebuffer *fb;
			guint32 format = -1;

			format = _get_pixel_format(me, mm_buf->format);
			GST_DEBUG_OBJECT(me, "format is [%d]", format);
			fb = dp_buffer_init(me->dp_device, 0, 0,
					    format,
					    mm_buf);
			if (!fb) {
				GST_ERROR_OBJECT(me, "failed to dp_buffer_init for index %d",
						 mm_buf->buffer_index);
				ret = GST_FLOW_ERROR;
				goto done;
			}
			GST_DEBUG_OBJECT(me, "make fb for %d",
					 mm_buf->buffer_index);

			me->fb[mm_buf->buffer_index] = fb;
		}

		GST_DEBUG_OBJECT(me, "display fb %d", mm_buf->buffer_index);
		if(me->dp_device) {
			dp_plane_update(me->dp_device, me->fb[mm_buf->buffer_index],
				0, 0);
		}
	}

done:
	gst_memory_unmap(meta_block, &info);

	GST_OBJECT_UNLOCK(me);

	return ret;
}

static gboolean gst_nx_renderer_query(GstBaseSink *bsink, GstQuery *query)
{
	gboolean ret;

	switch (GST_QUERY_TYPE(query)) {
	case GST_QUERY_SEEKING:{
		GstFormat fmt;

		gst_query_parse_seeking(query, &fmt, NULL, NULL, NULL);
		gst_query_set_seeking(query, fmt, FALSE, 0, -1);
		ret = TRUE;
		break;
	}
	default:
		ret = GST_BASE_SINK_CLASS(parent_class)->query(bsink, query);
		break;
	}

	return ret;
}

static GstStateChangeReturn gst_nx_renderer_change_state(GstElement *element,
	GstStateChange transition)
{
	/* TODO */
	/* return GST_STATE_CHANGE_SUCCESS; */
	return GST_ELEMENT_CLASS(parent_class)->change_state(element,
							     transition);
}

static gboolean plugin_init(GstPlugin *plugin)
{
	return gst_element_register(plugin, "nxrenderer",
				    GST_RANK_PRIMARY + 101,
				    gst_nx_renderer_get_type());
}

#ifndef PACKAGE
#define PACKAGE "nexell.gst.renderer"
#endif
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
		  GST_VERSION_MINOR,
		  nxrenderer,
		  "Nexell Display DRM Video plug-in",
		  plugin_init,
		  "0.0.1",
		  "LGPL",
		  "Nexell Co",
		  "http://www.nexell.co.kr")
