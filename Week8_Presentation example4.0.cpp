#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <string>
#include <unordered_map>

// 任务结构体，表示应用程序的子任务
struct Task {
    int task_id;
    std::string task_type;  // 任务类型：图形处理、数据处理等
    int priority;           // 优先级：1-10，数字越大优先级越高
};

// 应用程序结构体，表示一个应用程序的多个任务
struct Application {
    int app_id;
    std::string name;
    std::vector<Task> tasks;  // 该应用程序的任务列表
};

// 全局变量，用于存储任务队列
std::queue<Task> p_core_tasks;  // P核任务队列
std::queue<Task> e_core_tasks;  // E核任务队列

std::mutex p_mutex, e_mutex;
std::condition_variable cv;
bool stop_simulation = false;

// 模拟 P 核（性能核心）的任务
void P_core_task(const Task& task) {
    std::cout << "[P-core] Task " << task.task_id << " (" << task.task_type
        << ") running with priority " << task.priority
        << " in thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500 * task.priority));  // 模拟任务的执行时间
    std::cout << "[P-core] Task " << task.task_id << " completed." << std::endl;
}

// 模拟 E 核（能效核心）的任务
void E_core_task(const Task& task) {
    std::cout << "[E-core] Task " << task.task_id << " (" << task.task_type
        << ") running with priority " << task.priority
        << " in thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(800 * task.priority));  // 模拟任务的执行时间
    std::cout << "[E-core] Task " << task.task_id << " completed." << std::endl;
}

// APO 调度器：根据任务类型和优先级分配任务到合适的核心
void APO_scheduler(const Task& task) {
    // 输出调度信息
    std::cout << "APO Scheduler: Evaluating task " << task.task_id << " (" << task.task_type
        << ") with priority " << task.priority << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 模拟调度决策延迟

    // 图形处理或高优先级任务分配给P核
    if (task.task_type == "Graphics" || task.priority >= 7) {
        std::lock_guard<std::mutex> lock(p_mutex);
        p_core_tasks.push(task);
        std::cout << "APO Scheduler: Task " << task.task_id << " assigned to P-core." << std::endl;
        cv.notify_one();  // 通知P核处理
    }
    // 低优先级任务分配给E核
    else {
        std::lock_guard<std::mutex> lock(e_mutex);
        e_core_tasks.push(task);
        std::cout << "APO Scheduler: Task " << task.task_id << " assigned to E-core." << std::endl;
        cv.notify_one();  // 通知E核处理
    }
}

// P 核线程池，持续处理队列中的任务
void P_core_thread_pool() {
    while (!stop_simulation) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(p_mutex);
            cv.wait(lock, [] { return !p_core_tasks.empty() || stop_simulation; });

            if (stop_simulation && p_core_tasks.empty()) return;

            task = p_core_tasks.front();
            p_core_tasks.pop();
        }
        P_core_task(task);  // 处理任务
    }
}

// E 核线程池，持续处理队列中的任务
void E_core_thread_pool() {
    while (!stop_simulation) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(e_mutex);
            cv.wait(lock, [] { return !e_core_tasks.empty() || stop_simulation; });

            if (stop_simulation && e_core_tasks.empty()) return;

            task = e_core_tasks.front();
            e_core_tasks.pop();
        }
        E_core_task(task);  // 处理任务
    }
}

// 模拟应用程序的任务提交
void submit_application_tasks(Application& app) {
    std::cout << "\nApplication " << app.name << " is submitting tasks:" << std::endl;
    for (const auto& task : app.tasks) {
        std::cout << "Submitting task " << task.task_id << " (" << task.task_type
            << ") with priority " << task.priority << std::endl;
        APO_scheduler(task);  // 通过APO调度器分配任务
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // 模拟任务提交的间隔
    }
}

// 主函数，模拟应用程序优化
int main() {
    // 启动P核和E核的线程池
    std::thread p_core_thread(P_core_thread_pool);
    std::thread e_core_thread(E_core_thread_pool);

    // 创建并提交应用程序任务
    Application app1 = { 1, "Video Editing",
                        {{101, "Graphics", 9}, {102, "Background Processing", 5}, {103, "Data Compression", 3}} };

    Application app2 = { 2, "Game Engine",
                        {{201, "Graphics", 10}, {202, "AI Processing", 6}, {203, "Physics", 4}} };

    // 模拟两个应用程序的任务提交
    std::thread app1_thread(submit_application_tasks, std::ref(app1));
    std::thread app2_thread(submit_application_tasks, std::ref(app2));

    app1_thread.join();
    app2_thread.join();

    // 停止模拟
    {
        std::lock_guard<std::mutex> lock(p_mutex);
        stop_simulation = true;
    }
    cv.notify_all();  // 通知线程池停止
    p_core_thread.join();
    e_core_thread.join();

    std::cout << "All tasks completed. Simulation ended." << std::endl;
    return 0;
}
