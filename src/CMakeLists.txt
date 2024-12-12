set(SOURCES
	ai-proofread-plugin.c
	m-msg-composer-extension.c
	m-chatgpt-api.c)

set(HEADERS
	m-msg-composer-extension.h
	m-chatgpt-api.h
	m-version.h)

add_library(ai-proofread-plugin MODULE
	${SOURCES}
	${HEADERS})

target_include_directories(ai-proofread-plugin PRIVATE
	${EVOLUTION_INCLUDE_DIRS}
	${EVOLUTION_SHELL_INCLUDE_DIRS}
	${LIBSOUP_INCLUDE_DIRS}
	${CMAKE_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src)

target_link_libraries(ai-proofread-plugin
	${EVOLUTION_LDFLAGS}
	${EVOLUTION_SHELL_LDFLAGS}
	${EVOLUTION_MAIL_LDFLAGS}
	${LIBSOUP_LIBRARIES})

install(TARGETS ai-proofread-plugin
	DESTINATION ${EVOLUTION_MODULE_DIR})
