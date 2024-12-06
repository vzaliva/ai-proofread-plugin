list(APPEND SOURCES
	ai-proofread-plugin.c
	m-msg-composer-extension.h
	m-msg-composer-extension.c
)

add_library(ai-proofread-plugin MODULE ${SOURCES})

target_compile_definitions(ai-proofread-plugin PRIVATE
	G_LOG_DOMAIN=\"ai-proofread-plugin\"
)

target_compile_options(ai-proofread-plugin PUBLIC
	${EVOLUTION_SHELL_CFLAGS}
	${EVOLUTION_MAIL_CFLAGS}
	${LIBECAL_CFLAGS}
)

target_include_directories(ai-proofread-plugin PUBLIC
	${EVOLUTION_SHELL_INCLUDE_DIRS}
	${EVOLUTION_MAIL_INCLUDE_DIRS}
	${LIBECAL_INCLUDE_DIRS}
)

target_link_libraries(ai-proofread-plugin
	${EVOLUTION_SHELL_LDFLAGS}
	${EVOLUTION_MAIL_LDFLAGS}
	${LIBECAL_LDFLAGS}
)

install(TARGETS ai-proofread-plugin
	DESTINATION ${EVOLUTION_MODULE_DIR}
)
