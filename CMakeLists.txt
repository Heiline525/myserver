# 指定CMake的最低版本
cmake_minimum_required(VERSION 2.8)

# 指定项目名称
project(xyz)

# 将cmake工具脚本引入，该文件提供了编译时所需的额外处理函数
include (cmake/utils.cmake)

# 用于控制是否输出Makefile的详细信息
set(CMAKE_VERBOSE_MAKEFILE ON)

# 指定编译选项
# -std=c++11 指定编译时采用c++11的标准
# -O0：这个等级（字母“O”后面跟个零）关闭所有优化选项，也是CFLAGS或CXXFLAGS中没有设置-O等级时的默认等级。这样就不会优化代码，这通常不是我们想要的。
# -O1：这是最基本的优化等级。编译器会在不花费太多编译时间的同时试图生成更快更小的代码。这些优化是非常基础的，但一般这些任务肯定能顺利完成。
# -O2：-O1的进阶。这是推荐的优化等级，除非你有特殊的需求。-O2会比-O1启用多一些标记。设置了-O2后，编译器会试图提高代码性能而不会增大体积和大量占用的编译时间。
# -O3：这是最高最危险的优化等级。用这个选项会延长编译代码的时间，并且在使用gcc4.x的系统里不应全局启用。自从3.x版本以来gcc的行为已经有了极大地改变。在3.x，-O3生成的代码也只是比-O2快一点点而已，而gcc4.x中还未必更快。用-O3来编译所有的软件包将产生更大体积更耗内存的二进制文件，大大增加编译失败的机会或不可预知的程序行为（包括错误）。这样做将得不偿失，记住过犹不及。在gcc 4.x.中使用-O3是不推荐的。
# -Os：这个等级用来优化代码尺寸。其中启用了-O2中不会增加磁盘空间占用的代码生成选项。这对于磁盘空间极其紧张或者CPU缓存较小的机器非常有用。但也可能产生些许问题，因此软件树中的大部分ebuild都过滤掉这个等级的优化。使用-Os是不推荐的
# -ggdb ：此选项将尽可能的生成gdb的可以使用的调试信息
# -Wall ：生成所有警告信息
# -Werror 所有警告变成error
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++11 -O0 -ggdb -Wall -Werror")

# -rdynamic: 将所有符号都加入到符号表中，便于使用dlopen或者backtrace追踪到符号
# -fPIC: 生成位置无关的代码，便于动态链接
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -fPIC")

# -Wno-unused-function: 不要警告未使用函数
# -Wno-builtin-macro-redefined: 不要警告内置宏重定义，用于重定义内置的__FILE__宏
# -Wno-deprecated: 不要警告过时的特性
# -Wno-deprecated-declarations: 不要警告使用带deprecated属性的变量，类型，函数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated -Wno-deprecated-declarations")

# 将当前目录添加到编译器的头文件搜索目录中
include_directories(.)
include_directories(/apps/xyz/include)
link_directories(/apps/xyz/lib)

# 将所有编写的源文件设置到变量 LIB_SRC 中
set(
    LIB_SRC
    xyz/util.cc
)

# 将LIB_SRC中的文件集体打包成动态库,命名为xyz
add_library(xyz SHARED ${LIB_SRC})

# 调用 cmake/utils.cmake 中的方法，重定义目标源码的 __FILE__ 宏，使用相对路径的形式，避免暴露敏感信息
force_redefine_file_macro_for_sources(xyz)

# find_library(YAMLCPP yaml-cpp)

# 将所有库文件设置到变量 LIBS 中
set(
    LIBS
    xyz
)


# 指定生成的可执行文件(这里的方法包含了以下四步，方法在cmake/utils.cmake文件中)
# add_executable(test tests/test.cc)
# add_dependencies(test cc)
# force_redefine_file_macro_for_sources(test)
# target_link_libraries(test ${LIBS})

self_add_executable(main "tests/main_test.cc" xyz "${LIBS}")


# 指定执行文件的输出目录为当前文件夹下的bin目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 指定库的生成目录为当前文件夹下的lib目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
