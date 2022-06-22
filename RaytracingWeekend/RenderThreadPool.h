#pragma once
#include <mutex>
#include <vector>
#include <queue>
#include <thread>

#include "raytracer.h"

class ThreadPool {
public:
    void Start();
    void QueueJob(const tile& job);
    void Stop();
    bool busy();

private:
    void ThreadLoop();

    bool should_terminate = false;           // Tells threads to stop looking for jobs
    std::mutex queue_mutex;                  // Prevents data races to the job queue
    std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
    std::vector<std::thread> threads;
    std::queue<tile> jobs;
};

void ThreadPool::ThreadLoop() {
    while (true) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        mutex_condition.wait(lock, [this] {
            return !jobs.empty() || should_terminate;
            });
        if (should_terminate) {
            return;
        }
        auto job = jobs.front();
        jobs.pop();
    }
}

void ThreadPool::QueueJob(const tile& job) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(job);
    }
    mutex_condition.notify_one();
}

bool ThreadPool::busy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        poolbusy = jobs.empty();
    }
    return poolbusy;
}

void ThreadPool::Stop() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread& active_thread : threads) {
        active_thread.join();
    }
    threads.clear();
}