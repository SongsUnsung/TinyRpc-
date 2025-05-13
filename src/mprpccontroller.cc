#include "mprpccontroller.h"

MprpcController::MprpcController()
    : m_failed(false),
      m_errText(""),
      m_canceled(false),
      m_cancelCallback(nullptr) {}


void MprpcController::Reset()
{
    m_failed=false;
    m_errText="";

    m_canceled.store(false); 
    m_cancelCallback = nullptr;
}

bool MprpcController::Failed()const
{
    return m_failed;
}

std::string MprpcController::ErrorText()const
{
    return m_errText;
}

void MprpcController::SetFailed(const std::string & reason)
{
    m_failed=true;
    m_errText=reason;
}

void MprpcController::StartCancel()
{
    // 设置取消标志为 true
    m_canceled.store(true);
    
    // 如果存在回调，则执行它
    if (m_cancelCallback) {
        m_cancelCallback->Run();
    }
}

bool MprpcController::IsCanceled()const
{
    //return false;
    return m_canceled.load(); // 原子操作读取取消状态
}

void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback)
{
    m_cancelCallback = callback;
    
    // 如果已经取消，立即执行回调
    if (m_canceled.load() && m_cancelCallback) {
        m_cancelCallback->Run();
    }
}