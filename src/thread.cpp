//
// Created by grant on 11/19/20.
//

/**
 * @file stms/async.hpp
 * @brief `ThreadPool`s for async task scheduling and execution.
 * Created by grant on 12/30/19.
 */

#include "thread.hpp"

#include "log.hpp"

namespace stms {

    void ThreadPool::destroy() {
        if (!tasks.empty()) {
            WARN("ThreadPool destroyed with unfinished tasks! {} tasks will never be executed!", tasks.size());
        }

        if (this->running) {
            WARN("ThreadPool destroyed while running! Stopping it now (with block=true)");
            waitIdle(1000);
            stop(true);
        }
    }

    void ThreadPool::start(unsigned threads) {
        if (running) {
            WARN("ThreadPool::start() called when already started! Ignoring...");
            return;
        }

        if (threads == 0) {
            threads = (unsigned) (std::thread::hardware_concurrency() - 1); // subtract 1 bc of the main thread :D
            if (threads == 0) { // If that's STILL 0, default to 8 threads.
                threads = 8;
            }
        }

        this->running = true;
        {
            std::lock_guard<std::mutex> lg(this->workerMtx);
            for (unsigned i = 0; i < threads; i++) {
                this->workers.emplace_back(std::thread(workerFunc, this, i + 1));
            }
        }
    }

    void ThreadPool::stop(bool block) {
        if (!running) {
            WARN("ThreadPool::stop() called when already stopped! Ignoring...");
            return;
        }

        this->running = false;
        taskQueueCv.notify_all(); // Notify all workers that we are stopped!

        bool workersEmpty;
        {
            std::lock_guard<std::mutex> lg(this->workerMtx);
            workersEmpty = this->workers.empty();
        }

        while (!workersEmpty) {
            std::thread front;

            {
                std::lock_guard<std::mutex> lg(this->workerMtx);
                front = std::move(this->workers.front());
                this->workers.pop_front();
                workersEmpty = this->workers.empty();
            }

            if (front.joinable() && block) {
                front.join();
            } else {
                front.detach();  // Simply forget thread if not joinable.
            }
        }
    }

    std::future<void> ThreadPool::submitTask(const std::function<void(void)> &func) {
        {
            std::lock_guard<std::mutex> lg(unfinishedTaskMtx);
            unfinishedTasks++;
        }
        auto task = std::packaged_task<void(void)>(func);

        // Save future to variable since `task` is moved.
        auto future = task.get_future();

        std::lock_guard<std::mutex> lg(this->taskQueueMtx);
        this->tasks.emplace(std::move(task));
        taskQueueCv.notify_one(); // should this be changed to notify_all?

        return future;
    }

    void ThreadPool::pushThread() {
        if (!this->running) {
            WARN(
                    "`ThreadPool::pushThread()` called while the thread pool was stopped! Starting the thread pool!");
            this->running = true;
        }

        {
            std::lock_guard<std::mutex> lg(this->workerMtx);
            this->workers.emplace_back(workerFunc, this, this->workers.size() + 1);
        }
    }

    void ThreadPool::popThread(bool block) {
        if (!running) {
            WARN("ThreadPool::popThread() called when stopped! Ignoring invocation!");
            return;
        }

        std::thread back;
        {
            std::lock_guard<std::mutex> lg(this->workerMtx);
            back = std::move(this->workers.back());
            this->stopRequest = this->workers.size(); // Request the last worker to stop.
            taskQueueCv.notify_all(); // Notify this worker that we requested it to stop

            this->workers.pop_back();
            if (this->workers.empty()) {
                WARN("The last thread was popped from ThreadPool! Stopping the pool!");
                this->running = false;
                taskQueueCv.notify_all(); // Notify all workers that we've stopped
            }
        }

        if (back.joinable() && block) {
            back.join();
        } else {
            back.detach();
        }
    }

    ThreadPool &ThreadPool::operator=(ThreadPool &&rhs) noexcept {
        if (&rhs == this) {
            return *this;
        }

        unsigned nThreads = 0;
        if (rhs.isRunning()) {
            WARN("ThreadPool moved while running! It will be restarted for the move!");
            nThreads = rhs.getNumThreads();
            rhs.waitIdle(1000);
            rhs.stop(true);
        }

        this->destroy();

        {
            // Hopefully locking ALL mutexes during the move is enough to prevent aforementioned race conditions.
            std::lock_guard<std::mutex> rhsWorkerLg(rhs.workerMtx);
            std::lock_guard<std::mutex> thisWorkerLg(this->workerMtx);
            std::lock_guard<std::mutex> rhsTaskLg(rhs.taskQueueMtx);
            std::lock_guard<std::mutex> thisTaskLg(this->taskQueueMtx);

            std::lock_guard<std::mutex> thisTaskCountLg(this->unfinishedTaskMtx);
            std::lock_guard<std::mutex> rhsTaskCountLg(rhs.unfinishedTaskMtx);

            // We cannot move the mutex so we quietly skip it and hope nobody notices. (Watch it crash and burn later)
            // Likewise, we cannot move the condition variables so we just quietly leave it be
            this->stopRequest = rhs.stopRequest;
            this->running = rhs.running.load();
            this->tasks = std::move(rhs.tasks);
            this->workers = std::move(rhs.workers);
            this->unfinishedTasks = rhs.unfinishedTasks;
        }

        if (nThreads > 0) {
            this->start(nThreads);
        }

        return *this;
    }

    ThreadPool::ThreadPool(ThreadPool &&rhs) noexcept {
        *this = std::move(rhs);
    }

    ThreadPool::~ThreadPool() {
        this->destroy();
    }

    void ThreadPool::waitIdle(unsigned timeout) {
        std::unique_lock<std::mutex> lg(unfinishedTaskMtx);
        if (unfinishedTasks == 0) {
            return;
        }

        auto predicate = [&]() { return unfinishedTasks == 0; };
        if (timeout == 0) {
            unfinishedTasksCv.wait(lg, predicate);
        } else {
            unfinishedTasksCv.wait_for(lg, std::chrono::milliseconds(timeout), predicate);
        }
    }

}
