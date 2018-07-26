cmake_minimum_required (VERSION 3.0)
project (wakaama)

include(GNUInstallDirs)

option(DTLS "Enable DTLS" OFF)

add_subdirectory(core)
add_definitions(-DLWM2M_LITTLE_ENDIAN -DLWM2M_CLIENT_MODE -DLWM2M_SERVER_MODE -DLWM2M_BOOTSTRAP)
include_directories(core)

add_library(${PROJECT_NAME} SHARED ${CORE_SOURCES} platforms/Linux/platform.c)
set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION 1.0 SOVERSION 1 OUTPUT_NAME ${PROJECT_NAME}
	COMPILE_DEFINITIONS $<$<CONFIG:Debug>:WITH_LOGS>)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})


install(FILES ${CMAKE_SOURCE_DIR}/core/liblwm2m.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
