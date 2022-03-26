// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if !defined(NO_THREADS)

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Value.hpp>
#include <task/TaskManager.hpp>

class NativeTask : public Task {
public:
    Value result;
    std::atomic_bool done = false;
    std::atomic_bool cancelled = false;
    bool isDone() override {return done;}
    bool isCancelled() override {return cancelled;}
    void cancel() override {cancelled = true;}
};

class ThreadedTaskManager : public TaskManager {
public:
    static constexpr U32 maxRunners = 8;
    using Mutex = std::mutex;
    using Guard = std::lock_guard<Mutex>;

    PubSub<msg::Tick> pub{this};

    Vector<std::shared_ptr<NativeTask>> done;
    Mutex doneMut;

    Vector<std::shared_ptr<NativeTask>> queue;
    Mutex queueMut;

    std::vector<std::thread> threads;
    std::atomic_bool isLive = false;

    void init() {
        isLive = true;
        threads.resize(maxRunners);
        for (auto& thread : threads)
            thread = std::thread([=]{run();});
    }

    ~ThreadedTaskManager() {
        isLive = false;
        for (auto& thread : threads)
            thread.join();
        threads.clear();
    }

    void run() {
        std::shared_ptr<NativeTask> task;
        while (isLive) {
            if (!task) {
                Guard queueLock{queueMut};
                if (!queue.empty()) {
                    task = queue.front();
                    queue.erase(queue.begin());
                }
            }
            if (!task) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            if (task->isCancelled()) {
                task.reset();
                continue;
            }

            task->result = task->run();
            if (task->result.empty()) {
                continue;
            }

            {
                Guard doneLock{doneMut};
                done.push_back(task);
                task->done = true;
            }
            task.reset();
        }
    }

    void on(const msg::Tick&) {
        while (true) {
            std::shared_ptr<NativeTask> task;
            {
                Guard doneLock{doneMut};
                if (done.empty())
                    return;
                task = done.front();
                done.erase(done.begin());
            }
            if (!task->isCancelled()) {
                task->complete(std::move(task->result));
            }
        }
    }

    TaskHandle add(Task::Run&& run, Task::Complete&& complete) override {
        if (!isLive)
            init();
        auto task = std::make_shared<NativeTask>();
        task->run = std::move(run);
        task->complete = std::move(complete);

        {
            Guard queueLock{queueMut};
            queue.push_back(task);
        }

        return std::static_pointer_cast<Task>(task);
    }
};

TaskManager::Shared<ThreadedTaskManager> reg{"new"};
#endif
