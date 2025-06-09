aux_source_directory(. SRC_LIST)
add_library(mprpc ${SRC_LIST})
# muduo_net是核心模块，muduo_base会依赖net，所以muduo_net放在前面
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)