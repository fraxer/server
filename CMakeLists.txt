cmake_minimum_required(VERSION 2.8)

project(server LANGUAGES C)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#find_package(RapidJSON REQUIRED)
find_package(PCRE REQUIRED)
find_package(OpenSSL REQUIRED)

include(CTest)

add_executable(main src/main.c)

add_library(log STATIC src/log/log.c)

add_library(jsmn STATIC src/jsmn/jsmn.c)

add_library(config STATIC src/config/config.c)

add_library(moduleloader STATIC src/moduleloader/moduleloader.c)

add_library(helpers STATIC src/helpers/helpers.c)

add_library(signal STATIC src/signal/signal.c)

add_library(redirect STATIC src/redirect/redirect.c)

add_library(route STATIC src/route/route.c)

add_library(routeloader STATIC src/route/routeloader.c)

add_library(server STATIC src/server/server.c)

add_library(epoll STATIC src/epoll/epoll.c)

add_library(domain STATIC src/domain/domain.c)

add_library(socket STATIC src/socket/socket.c)

add_library(protocolmanager STATIC src/protocols/protocolmanager.c)

add_library(http1 STATIC src/protocols/http1.c)

add_library(http1common STATIC src/protocols/http1common.c)

add_library(http1parser STATIC src/protocols/http1parser.c)

add_library(http1request STATIC src/request/http1request.c)

add_library(http1response STATIC src/response/http1response.c)

add_library(websockets STATIC src/protocols/websockets.c)

add_library(websocketscommon STATIC src/protocols/websocketscommon.c)

add_library(websocketsparser STATIC src/protocols/websocketsparser.c)

add_library(websocketsrequest STATIC src/request/websocketsrequest.c)

add_library(websocketsresponse STATIC src/response/websocketsresponse.c)

add_library(tls STATIC src/protocols/tls.c)

add_library(connection STATIC src/connection/connection.c)

add_library(connection_queue STATIC src/connection/connection_queue.c)

add_library(database STATIC src/database/database.c)

add_library(openssl STATIC src/openssl/openssl.c)

add_library(mimetype STATIC src/mimetype/mimetype.c)

add_library(threadhandler STATIC src/thread/threadhandler.c)

add_library(threadworker STATIC src/thread/threadworker.c)

add_library(base64 STATIC src/base64/base64.c)

# Pages
add_library(indexpage SHARED src/handlers/index.c)



target_link_libraries(
	indexpage
	base64
)

target_link_libraries(
	config
	jsmn
	log
	helpers
)

target_link_libraries(
	routeloader
	${CMAKE_DL_LIBS}
)

target_link_libraries(
	moduleloader
	config
	redirect
	route
	routeloader
	database
	openssl
	mimetype
	threadhandler
	threadworker
	base64
	domain
	server
	epoll
	connection
	connection_queue
	socket
	protocolmanager

	http1
	http1parser
	http1request
	http1response
	http1common

	websockets
	websocketsparser
	websocketsrequest
	websocketsresponse
	websocketscommon

	tls

	pthread
	pcre
	${OPENSSL_LIBRARIES}
)

target_link_libraries(
	main
	moduleloader
	signal
)

#enable_testing()

#add_subdirectory(tests)