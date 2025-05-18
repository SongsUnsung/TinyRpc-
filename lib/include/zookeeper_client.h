#pragma once

#include<semaphore.h>
#include<zookeeper/zookeeper.h>
#include<string>


class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    //zkclient启动连接zkserver
    void Start();
    //在zkserver上根据指定path创建zonde节点
    void Create(const char *path,const char *data,int datalen,int state=0);
    //更具参数指定的znode节点路径，获取znode节点的值
    std::string GetData(const char *path);
private:
    zhandle_t *m_zhandle;

};