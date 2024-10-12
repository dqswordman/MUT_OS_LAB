#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool start_simulation = false;  // 用于同步线程的启动

std::queue<std::pair<int, int>> P_core_queue;  // P核任务队列
std::queue<std::pair<int, int>> E_core_queue;  // E核任务队列
std::mutex p_mtx, e_mtx;  // 用于分别保护P核和E核的任务队列

// 模拟 P 核（性能核心）的任务
void P_core_task(int task_id, int repeat) {
    for (int i = 0; i < repeat; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 模拟高负载任务
        std::cout << "Task " << task_id << " iteration " << (i + 1)
            << " on P-core in thread " << std::this_thread::get_id() << " completed."
            << std::endl;
    }
    std::cout << "Task " << task_id << " finished on P-core in thread "
        << std::this_thread::get_id() << "." << std::endl;
}

// 模拟 E 核（能效核心）的任务
void E_core_task(int task_id, int repeat) {
    for (int i = 0; i < repeat; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));  // 模拟低负载任务
        std::cout << "Task " << task_id << " iteration " << (i + 1)
            << " on E-core in thread " << std::this_thread::get_id() << " completed."
            << std::endl;
    }
    std::cout << "Task " << task_id << " finished on E-core in thread "
        << std::this_thread::get_id() << "." << std::endl;
}

// P 核线程池，持续处理队列中的任务
void P_core_thread_pool(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(p_mtx);
        if (!P_core_queue.empty()) {
            auto task = P_core_queue.front();
            P_core_queue.pop();
            lock.unlock();
            P_core_task(task.first, task.second);  // 处理任务
        }
        else {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 短暂等待
        }
    }
}

// E 核线程池，持续处理队列中的任务
void E_core_thread_pool(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(e_mtx);
        if (!E_core_queue.empty()) {
            auto task = E_core_queue.front();
            E_core_queue.pop();
            lock.unlock();
            E_core_task(task.first, task.second);  // 处理任务
        }
        else {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 短暂等待
        }
    }
}

// Intel Thread Director 模拟器：根据任务负载分配给 P 核或 E 核
void thread_director(int task_id, int workload) {
    int repeat = 3;  // 每个任务重复3次，模拟更长的执行过程
    if (workload > 5) {
        std::lock_guard<std::mutex> lock(p_mtx);
        P_core_queue.push({ task_id, repeat });  // 将任务加入P核队列
        std::cout << "Thread Director: Task " << task_id << " with workload " << workload
            << " assigned to P-core." << std::endl;
    }
    else {
        std::lock_guard<std::mutex> lock(e_mtx);
        E_core_queue.push({ task_id, repeat });  // 将任务加入E核队列
        std::cout << "Thread Director: Task " << task_id << " with workload " << workload
            << " assigned to E-core." << std::endl;
    }
}

int main() {
    int p_cores, e_cores;
    std::cout << "Enter the number of P-cores (Performance cores): ";
    std::cin >> p_cores;
    std::cout << "Enter the number of E-cores (Efficiency cores): ";
    std::cin >> e_cores;

    // 启动指定数量的P核和E核的线程池
    std::vector<std::thread> p_core_threads;
    std::vector<std::thread> e_core_threads;

    // 启动P核线程池
    for (int i = 0; i < p_cores; ++i) {
        p_core_threads.emplace_back(P_core_thread_pool, i);
    }

    // 启动E核线程池
    for (int i = 0; i < e_cores; ++i) {
        e_core_threads.emplace_back(E_core_thread_pool, i);
    }

    // 模拟一组任务，任务ID和负载
    std::vector<std::pair<int, int>> tasks = { {1, 3}, {2, 8}, {3, 1}, {4, 6}, {5, 4},
                                              {6, 7}, {7, 2}, {8, 9}, {9, 5}, {10, 3} };

    // 使用Intel Thread Director模拟任务分配
    for (const auto& task : tasks) {
        int task_id = task.first;
        int workload = task.second; // 工作负载，决定分配给P核还是E核

        std::cout << "Assigning Task " << task_id << " with workload " << workload << "."
            << std::endl;
        thread_director(task_id, workload);
    }

    // 等待所有任务执行完成，简单模拟，实际场景可以加入更复杂的控制机制
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // 在实际的程序中，线程池不会立即退出，因此这里不加入线程结束的控制
    for (auto& t : p_core_threads) {
        t.detach();  // 确保主线程退出时子线程不会挂起
    }
    for (auto& t : e_core_threads) {
        t.detach();
    }

    std::cout << "All tasks completed." << std::endl;
    return 0;
}
