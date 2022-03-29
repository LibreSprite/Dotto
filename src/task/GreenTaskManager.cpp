// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(NO_THREADS)

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <task/TaskManager.hpp>

class GreenTask : public Task {
public:
    Value result;
    bool done = false;
    bool cancelled = false;
    bool isDone() override {return done;}
    bool isCancelled() override {return cancelled;}
    void cancel() override {cancelled = true;}
};

class GreenTaskManager : public TaskManager {
public:
    static constexpr U32 maxRunners = 8;
    PubSub<msg::Tick> pub{this};

    Vector<std::shared_ptr<GreenTask>> queue;

    TaskHandle add(Task::Run&& run, Task::Complete&& complete) override {
        auto task = std::make_shared<GreenTask>();
        task->run = std::move(run);
        task->complete = std::move(complete);
        queue.push_back(task);
        return std::static_pointer_cast<Task>(task);
    }

    void on(const msg::Tick&) {
        for (U32 i = 0, max = std::min<U32>(queue.size(), maxRunners); i < max; ++i) {
            auto task = queue[i];
            bool erase = task->isCancelled();
            if (!erase) {
                task->result = task->run();
                if (!task->result.empty()) {
                    erase = true;
                    task->done = true;
                    task->complete(std::move(task->result));
                }
            }
            if (erase) {
                queue.erase(queue.begin() + i);
                --i;
                max = std::min<U32>(queue.size(), maxRunners);
            }
        }
    }
};

static TaskManager::Shared<GreenTaskManager> reg{"new"};

#endif
