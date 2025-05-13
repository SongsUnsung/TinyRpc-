#pragma once

#include<google/protobuf/service.h>
#include<string>
#include<atomic>

// MprpcController 继承自 protobuf 中的 RpcController，用于控制 RPC 的状态与错误信息。
class MprpcController : public google::protobuf::RpcController {
public:
    // 构造函数，初始化控制器状态
    MprpcController();

    // 重置控制器状态，清除之前的错误信息等
    void Reset() override;

    // 判断 RPC 调用是否失败
    bool Failed() const override;

    // 返回错误信息的文本描述
    std::string ErrorText() const override;

    // 设置失败状态，并提供失败原因
    void SetFailed(const std::string& reason) override;

    // 启动取消操作
    void StartCancel() override;

    // 判断 RPC 是否已经被取消
    bool IsCanceled() const override;

    // 设置取消时的回调函数（用于 RPC 被取消后执行某些清理逻辑）
    void NotifyOnCancel(google::protobuf::Closure* callback) override;

private:
    // 表示当前 RPC 是否失败
    bool m_failed;

    // 保存失败时的错误描述信息
    std::string m_errText;

    std::atomic<bool> m_canceled; 

    google::protobuf::Closure* m_cancelCallback; 
};