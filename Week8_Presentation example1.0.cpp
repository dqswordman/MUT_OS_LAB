#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

// 模拟 P 核（性能核心）和 E 核（能效核心），增加重复次数
void P_core_task(int task_id, int repeat) {
    std::cout << "Task " << task_id << " is running on P-core (Performance core)." << std::endl;
    for (int i = 0; i < repeat; ++i) {
        // 模拟高负载任务（需要较长时间执行）
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Task " << task_id << " iteration " << (i + 1) << " on P-core completed." << std::endl;
    }
    std::cout << "Task " << task_id << " finished on P-core." << std::endl;
}

void E_core_task(int task_id, int repeat) {
    std::cout << "Task " << task_id << " is running on E-core (Efficiency core)." << std::endl;
    for (int i = 0; i < repeat; ++i) {
        // 模拟低负载任务（执行时间较短）
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        std::cout << "Task " << task_id << " iteration " << (i + 1) << " on E-core completed." << std::endl;
    }
    std::cout << "Task " << task_id << " finished on E-core." << std::endl;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 在任务分配间增加一些延迟，方便观察
    }

    // 为了防止主线程提前结束，等待所有线程完成
    std::this_thread::sleep_for(std::chrono::seconds(20));  // 延长等待时间，确保所有任务完成

    std::cout << "All tasks completed." << std::endl;
    return 0;
}
