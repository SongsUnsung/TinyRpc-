#include "mprpcconfig.h"
#include<iostream>

// 加载配置文件，解析其中的键值对，存储到配置映射表中
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    // 打开配置文件（只读模式）
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf)
    {
        // 如果文件不存在或打开失败，打印错误并退出程序
        std::cout << config_file << " is not exists!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 文件逐行读取直到结束
    while (!feof(pf))
    {   
        char buf[523] = {0}; // 定义缓冲区并初始化
        fgets(buf, 512, pf); // 读取一行配置内容到 buf

        std::string read_buf(buf); // 将 char* 转换成 string 类型以便处理
        Trim(read_buf); // 去除前后空格

        // 忽略注释行（以 '#' 开头）或空行
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        // 查找等号位置，用于分割键值对
        int idx = read_buf.find('=');
        if (idx == -1)
        {
            // 若找不到等号，则跳过该行
            continue;
        }

        std::string key;
        std::string value;

        // 提取等号左边为 key，去除空格
        key = read_buf.substr(0, idx);
        Trim(key);

        // 提取等号右边为 value，去除空格和换行符
        int endidx = read_buf.find('\n', idx);
        value = read_buf.substr(idx + 1, endidx - idx - 1);
        Trim(value);

        // 将 key-value 键值对插入配置映射表中
        m_configMap.insert({key, value});
    }
}

// 根据 key 从配置映射表中获取对应的 value
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key); // 查找键值
    if (it == m_configMap.end())
    {
        // 如果找不到对应的 key，返回空字符串
        return "";
    }
    return it->second; // 返回 value
}

// 去除字符串左右两侧的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    // 查找字符串中第一个不是空格的位置
    int idx = src_buf.find_first_not_of(' ');

    // 如果存在非空格字符，则去除左侧空格
    if (idx != -1)
    {
        // 从第一个非空格字符开始截取字符串，去除左边的空格
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }

    // 查找字符串中最后一个不是空格的位置
    idx = src_buf.find_last_not_of(' ');

    // 如果存在非空格字符，则去除右侧空格
    if (idx != -1)
    {
        // 从开头到最后一个非空格字符的位置截取字符串，去除右边的空格
        src_buf = src_buf.substr(0, idx + 1);
    }
}
