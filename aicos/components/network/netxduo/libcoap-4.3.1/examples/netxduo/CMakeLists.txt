
set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/../../src/net.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/mem.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_prng.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_time.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_cache.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_option.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/resource.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/pdu.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/encode.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_subscribe.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_io.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/block.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/str.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_session.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_notls.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_hashkey.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_address.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_tcp.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_async.c
    ${CMAKE_CURRENT_LIST_DIR}/client.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_event.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/coap_mbedtls.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/uri.c
)

aic_compile_options("SHELL: $<TARGET_PROPERTY:compiler,imacros> ${CMAKE_CURRENT_LIST_DIR}/coap_config.h")

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../include
)

