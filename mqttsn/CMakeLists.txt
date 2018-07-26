cmake_minimum_required (VERSION 3.0)
project (paho_mqttsn)

include(GNUInstallDirs)

set(PACKET "mqttsnpacket")

include_directories (
	MQTTSNPacket/src
)

SET(SRCS_PACKET
	MQTTSNPacket/src/MQTTSNPacket.c
	MQTTSNPacket/src/MQTTSNConnectClient.c
	MQTTSNPacket/src/MQTTSNConnectServer.c
	MQTTSNPacket/src/MQTTSNSearchClient.c
	MQTTSNPacket/src/MQTTSNSearchServer.c
	MQTTSNPacket/src/MQTTSNDeserializePublish.c
	MQTTSNPacket/src/MQTTSNSerializePublish.c
	MQTTSNPacket/src/MQTTSNSubscribeClient.c
	MQTTSNPacket/src/MQTTSNSubscribeServer.c
	MQTTSNPacket/src/MQTTSNUnsubscribeClient.c
	MQTTSNPacket/src/MQTTSNUnsubscribeServer.c
)

add_library(${PACKET} SHARED ${SRCS_PACKET})
set_target_properties(${PACKET} PROPERTIES
	VERSION 1.0 SOVERSION 1 OUTPUT_NAME ${PACKET})
install(TARGETS ${PACKET} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/MQTTSNPacket/src/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MQTTSNPacket
	FILES_MATCHING PATTERN "MQTTSN*.h")
