ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CFLAGS = \
	$(WARN_CFLAGS) \
	$(GLIB_CFLAGS) \
	-I${includedir} \
	-I${includedir}/libdrm \
	-I./ \
	-I${EXTRAPATH_INCLUDE} \
	-I${EXTRAPATH}/lib/glib-2.0/include \
	-I${EXTRAPATH_INCLUDE}/glib-2.0 \
	-I${EXTRAPATH_INCLUDE}/gstreamer-${GST_API_VERSION} \
	-I${libdir}/gstreamer-${GST_API_VERSION}/include

libgstnxrenderer_la_LTLIBRARIES = libgstnxrenderer.la
libgstnxrenderer_ladir = ${libdir}/${GST_PLUGIN_PATH}
libgstnxrenderer_la_LDFLAGS = -L${libdir} -L${EXTRAPATH_LIB}
libgstnxrenderer_la_LIBADD = -lgstapp-1.0 -lgstallocators-1.0 -lgstbase-1.0 -lgmodule-2.0 -lgobject-2.0 -lglib-2.0
libgstnxrenderer_la_LIBADD += -lnx_drm_allocator -lnx_renderer -lkms -ldrm -ldrm_nexell
libgstnxrenderer_la_SOURCES = \
	gstnxrenderer.c
