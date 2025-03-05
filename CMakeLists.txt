# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral)


# Automatically include all .c files in the src directory
file(GLOB APP_SOURCES "src/*.c")

target_sources(app PRIVATE
  ${APP_SOURCES}
)
