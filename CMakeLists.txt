cmake_minimum_required(VERSION 3.0)
project(luwu)

# luwu 最终编译成 so 库路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 测试文件可执行程序生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置调试选项，设置 c++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 设置头文件路径，除了系统路径 /usr/include 还有下面这个
include_directories(./luwu)

# 是否编译测试文件
option(BUILD_TEST "ON for compile test" ON)

# 定义参与编译的源文件
aux_source_directory(./luwu BASE_LIST)
aux_source_directory(./luwu/utils UTIL_LIST)
aux_source_directory(./luwu/http HTTP_LIST)
aux_source_directory(./luwu/http/http-parser PARSER_LIST)
set(LIB_SRC ${BASE_LIST} ${UTIL_LIST} ${HTTP_LIST} ${PARSER_LIST} ${STREAM_LIST})

# 定义需要连接的动态库
set(LIBS pthread dl ssl crypto)

# 编译生成可执行程序
if (BUILD_TEST)
    add_executable(test_log "test/test_logger.cpp" ${LIB_SRC})
    target_link_libraries(test_log ${LIBS})

    add_executable(test_fiber "test/test_fiber.cpp" ${LIB_SRC})
    target_link_libraries(test_fiber ${LIBS})

    add_executable(test_thread "test/test_thread.cpp" ${LIB_SRC})
    target_link_libraries(test_thread ${LIBS})

    add_executable(test_scheduler "test/test_scheduler.cpp" ${LIB_SRC})
    target_link_libraries(test_scheduler ${LIBS})

    add_executable(test_clock "test/test_clock.cpp" ${LIB_SRC})
    target_link_libraries(test_clock ${LIBS})

    add_executable(test_reactor "test/test_reactor.cpp" ${LIB_SRC})
    target_link_libraries(test_reactor ${LIBS})

    add_executable(test_hook "test/test_hook.cpp" ${LIB_SRC})
    target_link_libraries(test_hook ${LIBS})

    add_executable(test_address "test/test_address.cpp" ${LIB_SRC})
    target_link_libraries(test_address ${LIBS})

    add_executable(test_socket "test/test_socket.cpp" ${LIB_SRC})
    target_link_libraries(test_socket ${LIBS})

    add_executable(test_bytearray "test/test_bytearray.cpp" ${LIB_SRC})
    target_link_libraries(test_bytearray ${LIBS})

    add_executable(test_tcpserver "test/test_tcpserver.cpp" ${LIB_SRC})
    target_link_libraries(test_tcpserver ${LIBS})

    add_executable(test_message "test/test_message.cpp" ${LIB_SRC})
    target_link_libraries(test_message ${LIBS})

    add_executable(test_parser "test/test_parser.cpp" ${LIB_SRC})
    target_link_libraries(test_parser ${LIBS})

    add_executable(test_http_server "test/test_http_server.cpp" ${LIB_SRC})
    target_link_libraries(test_http_server ${LIBS})

    add_executable(test_ws_server "test/test_ws_server.cpp" ${LIB_SRC})
    target_link_libraries(test_ws_server ${LIBS})
endif ()

# 编译生成动态库
#add_library(luwu SHARED ${LIB_SRC})