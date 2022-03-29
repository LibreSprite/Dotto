// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Value.hpp>

class TaskHandle;

class Task {
    friend class TaskHandle;
    U32 handleCount = 0;

public:
    using Run = std::function<Value()>;
    using Complete = std::function<void(Value&&)>;

    Run run;
    Complete complete;

    virtual bool isDone() = 0;
    virtual bool isCancelled() = 0;
    virtual void cancel() = 0;
};

class TaskHandle {
public:
    std::shared_ptr<Task> task;

    TaskHandle() = default;

    TaskHandle(std::shared_ptr<Task> task) : task{task} {
        task->handleCount++;
    }

    TaskHandle(const TaskHandle& other) : TaskHandle{other.task} {}

    TaskHandle(TaskHandle&& other) : TaskHandle(other.task) {}

    TaskHandle& operator = (const TaskHandle& other) {
        reset();
        task = other.task;
        if (task)
            task->handleCount++;
        return *this;
    }

    TaskHandle& operator = (TaskHandle&& other) {
        reset();
        task = other.task;
        other.task.reset();
        return *this;
    }

    void reset() {
        if (task) {
            task->handleCount--;
            if (!task->handleCount) {
                task->cancel();
            }
            task.reset();
        }
    }

    ~TaskHandle() {
        reset();
    }
};

class TaskManager : public Injectable<TaskManager> {
public:
    virtual TaskHandle add(Task::Run&& run, Task::Complete&& complete) = 0;
};
