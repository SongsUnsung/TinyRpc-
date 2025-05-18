// lockqueue.h
#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <exception>

template<typename T>
class LockQueue
{
public:
    LockQueue() : m_exit(false) {}
    
    ~LockQueue() {
        // 析构时标记退出
        SetExit();
    }
    
    // 推送数据到队列
    void Push(const T& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_cond.notify_one();
    }
    
    // 推送右值数据，提高效率
    void Push(T&& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(data));
        m_cond.notify_one();
    }
    
    // 从队列取数据，如果队列为空则等待
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        // 等待条件：队列非空或者退出标志被设置
        m_cond.wait(lock, [this]() { 
            return !m_queue.empty() || m_exit; 
        });
        
        // 如果是因为退出标志被设置而被唤醒，则抛出异常
        if (m_queue.empty() && m_exit) {
            throw std::runtime_error("Queue is exiting");
        }
        
        T data = std::move(m_queue.front());
        m_queue.pop();
        return data;
    }
    
    // 检查队列是否为空
    bool Empty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
    
    // 获取队列大小
    size_t Size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
    
    // 清空队列
    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::queue<T> empty;
        std::swap(m_queue, empty);
    }
    
    // 设置退出标志
    void SetExit()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_exit = true;
        m_cond.notify_all(); // 通知所有等待线程
    }
    
    // 手动唤醒等待的线程
    void Notify()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cond.notify_all();
    }
    
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_exit{false}; // 退出标志
};