#set(SRC_LIST userservice.cc ../user.pb.cc)

set(SRC_LIST friendservice.cc ../friend.pb.cc)


add_executable(provider ${SRC_LIST})



target_include_directories(provider PRIVATE ${THIRD_PARTY_INCLUDE_DIRS})
target_link_directories(provider PRIVATE ${THIRD_PARTY_LIBRARY_DIRS})
target_link_libraries(provider PRIVATE mprpc protobuf ${THIRD_PARTY_LIBS})
