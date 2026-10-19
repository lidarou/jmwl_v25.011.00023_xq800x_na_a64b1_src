cmake_minimum_required(VERSION 3.13)

aic_library_sources_ifdef(
                CONFIG_DRIVER_LZMA_DEC
                LzmaDec.c
                )
