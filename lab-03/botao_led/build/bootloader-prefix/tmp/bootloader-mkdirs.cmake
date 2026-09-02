# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/sis-emb-2026-bancada-07/esp/esp-idf/components/bootloader/subproject"
  "/home/sis-emb-2026-bancada-07/sis-emb/lab3/botao_led/build/bootloader"
  "/home/sis-emb-2026-bancada-07/sis-emb/lab3/botao_led/build/bootloader-prefix"
  "/home/sis-emb-2026-bancada-07/sis-emb/lab3/botao_led/build/bootloader-prefix/tmp"
  "/home/sis-emb-2026-bancada-07/sis-emb/lab3/botao_led/build/bootloader-prefix/src/bootloader-stamp"
  "/home/sis-emb-2026-bancada-07/sis-emb/lab3/botao_led/build/bootloader-prefix/src"
  "/home/sis-emb-2026-bancada-07/sis-emb/lab3/botao_led/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/sis-emb-2026-bancada-07/sis-emb/lab3/botao_led/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/sis-emb-2026-bancada-07/sis-emb/lab3/botao_led/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
