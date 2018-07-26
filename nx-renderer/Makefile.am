AM_CFLAGS = \
	$(WARN_CFLAGS) \
	-I./include \
	-I${includedir} \
	-I${includedir}/libdrm

libnx_renderer_la_LTLIBRARIES = libnx_renderer.la
libnx_renderer_ladir = ${libdir}
libnx_renderer_la_LDFLAGS = -L${libdir}
libnx_renderer_la_LIBADD = -ldrm -lkms

libnx_renderer_la_SOURCES = \
	dp_buffer.c \
	dp_crtc.c \
	dp_device.c \
	dp_framebuffer.c \
	dp_plane.c \
	dp_screen.c \
	dp_util.c \
	format.c \
	kms.c \
	pattern.c

libnx_rendererincludedir = ${includedir}
libnx_rendererinclude_HEADERS = \
	include/dp.h \
	include/dp_common.h


