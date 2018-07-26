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

#ifndef __GST_NX_RENDERER_H__
#define __GST_NX_RENDERER_H__

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>

#include <dp.h>
#include <dp_common.h>

#include "mm_types.h"

G_BEGIN_DECLS

#define GST_TYPE_NX_RENDERER \
	(gst_nx_renderer_get_type())
#define GST_NX_RENDERER(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_NX_RENDERER,\
				    GstNxRenderer))
#define GST_NX_RENDERER_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_NX_RENDERER,\
				 GstNxRendererClass))
#define GST_IS_NX_RENDERER(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_NX_RENDERER)
#define GST_NX_RENDERER_CAST(obj) \
	 ((GstNxRenderer *)obj)

typedef struct _GstNxRenderer GstNxRenderer;
typedef struct _GstNxRendererClass GstNxRendererClass;

#define MAX_BUFFER_COUNT	16

struct _GstNxRenderer {
	GstBaseSink element;

	int drm_fd;
	struct dp_device *dp_device;
	struct dp_framebuffer *fb[MAX_BUFFER_COUNT];
};

struct _GstNxRendererClass {
	GstBaseSinkClass parent_class;
};

G_GNUC_INTERNAL GType gst_nx_renderer_get_type(void);

G_END_DECLS

#endif
