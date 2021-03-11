cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin )

project("upark_rest_server")

find_package(cpprestsdk CONFIG REQUIRED)

#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

include_directories(/home/michele/Downloads/mysql-connector-c++-8.0.22-linux-glibc2.12-x86-32bit/include)

link_directories(/home/michele/Downloads/mysql-connector-c++-8.0.22-linux-glibc2.12-x86-32bit/lib)

add_executable(${PROJECT_NAME}        ./src/main.cpp
                                      ./src/controller/upark_controller.cpp
                                      ./src/controller/basic_controller.cpp
                                      ./src/entities/hourly_rate/hourly_rate.cpp
                                      ./src/entities/user_category/user_category.cpp
                                      ./src/entities/user/user.cpp
                                      ./src/entities/vehicle_type/vehicle_type.cpp
                                      ./src/entities/vehicle/vehicle.cpp
                                      ./src/entities/parking_lot/parking_lot.cpp
                                      ./src/entities/parking_categories_allowed/parking_categories_allowed.cpp
                                      ./src/entities/parking_slot/parking_slot.cpp
                                      ./src/entities/booking/booking.cpp
                                      ./src/database/database.cpp
                                      )

set(CPPRESTSDK_LIBRARY cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_boost_internal cpprestsdk::cpprestsdk_openssl_internal)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CPPRESTSDK_LIBRARY} mysqlcppconn)
