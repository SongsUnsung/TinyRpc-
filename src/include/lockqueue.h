#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

template<typename T>
class LockQueue {
public:
    LockQueue() : m_exit(false) {}

    void Push(const T& data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_cond.notify_one();
    }

    void Push(T&& data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(data));
        m_cond.notify_one();
    }
    T Pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() { return !m_queue.empty() || m_exit; });
        
        if (m_queue.empty() && m_exit) {
            throw std::runtime_error("Queue is exiting");
        }
        T data = std::move(m_queue.front());
        m_queue.pop();
        return data;
    }
    bool PopBulk(std::vector<T>& output, size_t max_size = 100, int timeout_ms = 100) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_cond.wait_for(lock, std::chrono::milliseconds(timeout_ms),
            [this](){ return !m_queue.empty() || m_exit; })) 
        {
            while (!m_queue.empty() && output.size() < max_size) {
                output.push_back(std::move(m_queue.front()));
                m_queue.pop();
            }
            return !output.empty();
        }
        return false;
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void SetExit() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_exit = true;
        m_cond.notify_all();
    }

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_exit{false};
};