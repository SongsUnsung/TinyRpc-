#include "test.pb.h"

#include <iostream>

#include<string>
using namespace fixbug;


int main()
{
    /* LoginResponse rsp;
    ResultCode *rc=rsp.mutable_result();

    rc->set_errcode(1);
    rc->set_errmsg("login error");
 */
    GetFriendListsResponse rsp;
    ResultCode *rc=rsp.mutable_result();
    rc->set_errcode(0);

    User *user1=rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2=rsp.add_friend_list();
    user1->set_name("wan wu");
    user1->set_age(21);
    user1->set_sex(User::MAN);


    User *user3=rsp.add_friend_list();
    user1->set_name("lisi");
    user1->set_age(22);
    user1->set_sex(User::MAN);

    std::cout<<rsp.friend_list_size()<<std::endl;


}

/* int main()
{
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    std::string send_str;
    if(req.SerializeToString(&send_str))
    {
        std::cout<<send_str.c_str()<<std::endl;
    }

    LoginRequest reqB;
    if(reqB.ParseFromString(send_str))
    {
        std::cout<<reqB.name()<<std::endl;
        std::cout<<reqB.pwd()<<std::endl;
    }
    return 0;

} */