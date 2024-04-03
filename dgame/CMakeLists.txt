# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(espdoom)

# -Werror=unused-const-variable=
# -Werror=char-subscripts
# -Werror=format=  %d :)
# -Werror=unused-but-set-parameter
# -Werror=format-truncation=
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=format-truncation=" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=unused-but-set-parameter" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=format=" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=char-subscripts" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=unused-const-variable=" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=stringop-truncation" APPEND)
