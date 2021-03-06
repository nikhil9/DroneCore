#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdio>

namespace dronecore {

/*
 * Thread-safe queue taken from:
 * http://stackoverflow.com/questions/15278343/c11-thread-safe-queue#answer-16075550
 */

template <class T>
class SafeQueue
{
public:
    SafeQueue() :
        _queue(),
        _mutex(),
        _condition_var()
    {}
    ~SafeQueue() {}

    void enqueue(T item)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(item);
        _condition_var.notify_one();
    }

    T dequeue()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_queue.empty()) {
            if (_should_exit) {
                return nullptr;
            }
            // Release lock during the wait and re-aquire it afterwards.
            _condition_var.wait(lock);
        }
        T item = _queue.front();
        _queue.pop();
        return item;
    }

    void stop()
    {
        // This can be used if the wait needs to be interrupted, e.g.
        // when trying to stop a worker thread.
        std::lock_guard<std::mutex> lock(_mutex);
        _should_exit = true;
        _condition_var.notify_all();
    }

private:
    std::queue<T> _queue;
    mutable std::mutex _mutex;
    std::condition_variable _condition_var;
    bool _should_exit = false;
};

} // namespace dronecore
