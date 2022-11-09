cmake_minimum_required(VERSION 3.0)
project(luwu)

# luwu 最终编译成 so 库
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 测试文件可执行程序生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置调试选项，设置 c++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g -std=c++11")

# 设置头文件路径，除了系统路径 /usr/include 还有下面这个
include_directories(./luwu)

# 定义参与编译的源文件
aux_source_directory(./luwu BASE_LIST)
aux_source_directory(./luwu/utils UTIL_LIST)

# 编译生成可执行程序
add_executable(test_log "test/test_logger.cpp" ${BASE_LIST} ${UTIL_LIST})
add_executable(test_assert "test/test_asserts.cpp" ${BASE_LIST} ${UTIL_LIST})
add_executable(test_fiber "test/test_fiber.cpp" ${BASE_LIST} ${UTIL_LIST})
add_executable(test_thread "test/test_thread.cpp" ${BASE_LIST} ${UTIL_LIST})
add_executable(test_scheduler "test/test_scheduler.cpp" ${BASE_LIST} ${UTIL_LIST})
# 编译生成动态库
# add_library()