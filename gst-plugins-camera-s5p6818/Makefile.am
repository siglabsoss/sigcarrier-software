ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CFLAGS = \
	$(WARN_CFLAGS) \
	$(GLIB_CFLAGS) \
	-I${includedir} \
	-I./ \
	-I${EXTRAPATH_INCLUDE} \
	-I${EXTRAPATH}/lib/glib-2.0/include \
	-I${EXTRAPATH_INCLUDE}/glib-2.0 \
	-I${EXTRAPATH_INCLUDE}/gstreamer-${GST_API_VERSION} \
	-I${libdir}/gstreamer-${GST_API_VERSION}/include

libgstnxcamerasrc_la_LTLIBRARIES = libgstnxcamerasrc.la
libgstnxcamerasrc_ladir = ${libdir}/${GST_PLUGIN_PATH}
libgstnxcamerasrc_la_LDFLAGS = -L${libdir} -L${libdir}/${GST_PLUGIN_PATH} -L${EXTRAPATH_LIB}
libgstnxcamerasrc_la_LIBADD = -lgstvideo-1.0 -lgstallocators-1.0 -lgstbase-1.0 -lgmodule-2.0 -lgobject-2.0 -lglib-2.0
libgstnxcamerasrc_la_LIBADD += -lnx_drm_allocator -lnx_v4l2 -lnxgstmeta
libgstnxcamerasrc_la_SOURCES = \
	gstcamerasrc.c

gstnxcamerasrcincludedir = ${includedir}
gstnxcamerasrcinclude_HEADERS =
