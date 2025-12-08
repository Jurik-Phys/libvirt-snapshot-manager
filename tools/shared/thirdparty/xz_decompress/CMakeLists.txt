cmake_minimum_required(VERSION 3.20)

project(liblzma_decoder LANGUAGES C)

# Подключение файлов необходимых для decoder'а
set(LIBLZMA_SRCS
    src/common/tuklib_physmem.c
    src/liblzma/common/alone_decoder.c
    src/liblzma/common/auto_decoder.c
    src/liblzma/common/block_buffer_decoder.c
    src/liblzma/common/block_decoder.c
    src/liblzma/common/block_header_decoder.c
    src/liblzma/common/block_util.c
    src/liblzma/common/common.c
    src/liblzma/common/easy_decoder_memusage.c
    src/liblzma/common/filter_buffer_decoder.c
    src/liblzma/common/filter_common.c
    src/liblzma/common/filter_decoder.c
    src/liblzma/common/filter_flags_decoder.c
    src/liblzma/common/hardware_physmem.c
    src/liblzma/common/index.c
    src/liblzma/common/index_decoder.c
    src/liblzma/common/index_hash.c
    src/liblzma/common/lzip_decoder.c
    src/liblzma/common/microlzma_decoder.c
    src/liblzma/common/stream_buffer_decoder.c
    src/liblzma/common/stream_decoder.c
    src/liblzma/common/stream_flags_common.c
    src/liblzma/common/stream_flags_decoder.c
    src/liblzma/common/string_conversion.c
    src/liblzma/common/vli_decoder.c
    src/liblzma/common/vli_size.c

    src/liblzma/delta/delta_decoder.c
    src/liblzma/delta/delta_common.c

    src/liblzma/lzma/lzma_decoder.c
    src/liblzma/lzma/lzma2_decoder.c

    src/liblzma/lz/lz_decoder.c

    src/liblzma/simple/simple_coder.c
    src/liblzma/simple/simple_decoder.c
    src/liblzma/simple/arm.c
    src/liblzma/simple/armthumb.c
    src/liblzma/simple/arm64.c
    src/liblzma/simple/ia64.c
    src/liblzma/simple/powerpc.c
    src/liblzma/simple/sparc.c
    src/liblzma/simple/riscv.c
    src/liblzma/simple/x86.c

    src/liblzma/check/check.c
    src/liblzma/check/crc32_fast.c
    src/liblzma/check/crc64_fast.c
    src/liblzma/check/sha256.c
)

add_library(liblzma_decoder STATIC ${LIBLZMA_SRCS})

# Заголовки (для включения)
target_include_directories(liblzma_decoder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liblzma/api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liblzma/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liblzma/delta
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liblzma/lzma
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liblzma/lz
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liblzma/rangecoder
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liblzma/simple
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liblzma/check
)

target_compile_definitions(liblzma_decoder PRIVATE
    HAVE_DECODER_LZMA1
    HAVE_DECODER_LZMA2
    HAVE_DECODER_X86
    HAVE_DECODER_POWERPC
    HAVE_DECODER_IA64
    HAVE_DECODER_ARM
    HAVE_DECODER_ARMTHUMB
    HAVE_DECODER_ARM64
    HAVE_DECODER_SPARC
    HAVE_DECODER_RISCV
    HAVE_DECODER_DELTA

    HAVE_CHECK_CRC32
    HAVE_CHECK_CRC64
    HAVE_CHECK_SHA256
)
