#set(SRC_LIST calluserservice.cc ../user.pb.cc)
set(SRC_LIST callfriendservice.cc ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})



target_include_directories(consumer PRIVATE ${THIRD_PARTY_INCLUDE_DIRS})
target_link_directories(consumer  PRIVATE ${THIRD_PARTY_LIBRARY_DIRS})
target_link_libraries(consumer  PRIVATE mprpc protobuf ${THIRD_PARTY_LIBS})
