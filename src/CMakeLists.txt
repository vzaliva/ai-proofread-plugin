list(APPEND SOURCES
	example-module.c
	m-calendar-ui.h
	m-calendar-ui.c
	m-mail-ui.h
	m-mail-ui.c
	m-msg-composer-extension.h
	m-msg-composer-extension.c
	m-shell-view-extension.h
	m-shell-view-extension.c
	m-utils.h
	m-utils.c
)

add_library(example-module MODULE ${SOURCES})

target_compile_definitions(example-module PRIVATE
	G_LOG_DOMAIN=\"example-module\"
)

target_compile_options(example-module PUBLIC
	${EVOLUTION_SHELL_CFLAGS}
	${EVOLUTION_CALENDAR_CFLAGS}
	${EVOLUTION_MAIL_CFLAGS}
	${LIBECAL_CFLAGS}
)

target_include_directories(example-module PUBLIC
	${EVOLUTION_SHELL_INCLUDE_DIRS}
	${EVOLUTION_CALENDAR_INCLUDE_DIRS}
	${EVOLUTION_MAIL_INCLUDE_DIRS}
	${LIBECAL_INCLUDE_DIRS}
)

target_link_libraries(example-module
	${EVOLUTION_SHELL_LDFLAGS}
	${EVOLUTION_CALENDAR_LDFLAGS}
	${EVOLUTION_MAIL_LDFLAGS}
	${LIBECAL_LDFLAGS}
)

install(TARGETS example-module
	DESTINATION ${EVOLUTION_MODULE_DIR}
)
