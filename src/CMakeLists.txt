aux_source_directory(. SRC_LIST)
#add_library(mprpc SHARED ${SRC_LIST})
#静态库
add_library(mprpc STATIC ${SRC_LIST})

target_link_libraries(mprpc  muduo_net muduo_base pthread zookeeper_mt)