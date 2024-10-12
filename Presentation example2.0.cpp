#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool start_simulation = false;  // 用于同步线程的启动

// 模拟 P 核（性能核心）和 E 核（能效核心），增加并发调度
void P_core_task(int task_id, int repeat) {
    // 等待同步信号，确保所有线程同时开始
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return start_simulation; });

    // 输出线程ID，表明是不同线程
    std::cout << "Task " << task_id << " is running on P-core (Performance core) in thread "
        << std::this_thread::get_id() << "." << std::endl;

    for (int i = 0; i < repeat; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 模拟高负载任务
        std::cout << "Task " << task_id << " iteration " << (i + 1) << " on P-core in thread "
            << std::this_thread::get_id() << " completed." << std::endl;
    }
    std::cout << "Task " << task_id << " finished on P-core in thread "
        << std::this_thread::get_id() << "." << std::endl;
}

void E_core_task(int task_id, int repeat) {
    // 等待同步信号，确保所有线程同时开始
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return start_simulation; });

    // 输出线程ID，表明是不同线程
    std::cout << "Task " << task_id << " is running on E-core (Efficiency core) in thread "
        << std::this_thread::get_id() << "." << std::endl;

    for (int i = 0; i < repeat; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));  // 模拟低负载任务
        std::cout << "Task " << task_id << " iteration " << (i + 1) << " on E-core in thread "
            << std::this_thread::get_id() << " completed." << std::endl;
    }
    std::cout << "Task " << task_id << " finished on E-core in thread "
        << std::this_thread::get_id() << "." << std::endl;
}

// Intel Thread Director 模拟器：根据任务负载分配给 P 核或 E 核
void thread_director(int task_id, int workload) {
    int repeat = 3;  // 每个任务重复3次，模拟更长的执行过程
    if (workload > 5) {
        // 如果任务的负载大于5，分配给 P 核
        std::thread(P_core_task, task_id, repeat).detach();
    }
    else {
        // 如果任务的负载小于或等于5，分配给 E 核
        std::thread(E_core_task, task_id, repeat).detach();
    }
}

int main() {
    // 模拟一组任务，任务ID和负载
    std::vector<std::pair<int, int>> tasks = { {1, 3}, {2, 8}, {3, 1}, {4, 6}, {5, 4} };

    // 使用Intel Thread Director模拟任务分配
    for (const auto& task : tasks) {
        int task_id = task.first;
        int workload = task.second; // 工作负载，决定分配给P核还是E核

        std::cout << "Assigning Task " << task_id << " with workload " << workload << "." << std::endl;
        thread_director(task_id, workload);
    }

    // 等待2秒，以便所有线程被创建
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 通过条件变量同步，启动所有任务
    {
        std::lock_guard<std::mutex> lock(mtx);
        start_simulation = true;
    }
    cv.notify_all();  // 通知所有等待的线程同时开始

    // 为了防止主线程提前结束，等待所有线程完成
    std::this_thread::sleep_for(std::chrono::seconds(20));  // 延长等待时间，确保所有任务完成

    std::cout << "All tasks completed." << std::endl;
    return 0;
}
