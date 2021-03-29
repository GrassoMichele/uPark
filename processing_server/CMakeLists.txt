cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin )

project("upark_processing_server")

find_package(cpprestsdk CONFIG REQUIRED)


add_executable(${PROJECT_NAME}        ./main.cpp
                                      )

set(CPPRESTSDK_LIBRARY cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_boost_internal cpprestsdk::cpprestsdk_openssl_internal)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CPPRESTSDK_LIBRARY})
