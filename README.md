## 创建线程

### std::thread

普通函数

```CPP
std::thread(func, args...)
```

```cpp
void printHelloWorld(const std::string &msg){
    std::cout<<msg;

}
int main(){
    std::thread thread1(printHelloWorld, "hello world");
    thread1.join();
    return 0;
}
```

类的非静态成员函数

```cpp
A a;
std::thread t(&A::func, &a);
```



#### 1、 join() 阻塞主线程，等待子线程完成



**joinable()**判断对象能够汇合，如果不能汇合将返回false，如果对已经汇合过的线程调用join()将产生错误行为。



#### 2、detach() 将子线程和主线程分离 



### 数据未定义错误

##### 1、传入临时变量导致构造函数语法与声明函数一致

如：std::thread my_thread(background_task())

解决方法： 1、使用列表初始化

​					2、使用lambda表达式编写局部函数



##### 2、传入临时变量，但形参为引用

临时变量被传入参数后销毁，形参的引用导致了未定义的行为。



##### 3、传递指针或者引用指向已经释放的内存



##### 4、入口函数为类的私有成员函数

解决方法：将私有成员函数变为友元函数





## 数据共享

#### std::mutex

创建互斥锁

1、lock()加锁

2、unlock()解锁



#### std::timed_mutex

支持延迟加锁的互斥锁，当使用延迟加锁时，就使用这个锁来控制互斥量。



#### std::lock_guard

构造时加锁系，析构时解锁，只在局部作用域中使用

传递第二个参数为std::adopt_lock表示获取锁之前已经加锁了



#### std::unique_lock

对互斥量的灵活加锁，构造时自动加锁，析构时解锁。

传递第二个参数std::adopt_lock指明对象管理互斥上的锁；

​						  std::defer_lock使互斥在完成构造时为无锁状态。

成员函数:

lock()自己加锁；

unlock()自己解锁；

try_lock_for()等待一段时间，如果获取不到锁就结束等待；

try_lock_until()等待到某个时间, 如果获取不到锁就结束等待；



#### std::call_once

在单例模式下使用

std::once_flag实例对应一次不同的初始化。





### 原子操作

#### std::atomic



```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
std::atomic<int> shared_var = 0;
void func(){
    for (int i = 0; i < 100000;++i){
        ++shared_var;
    }
}

int main(){
    auto last = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::thread thread1(func);
    std::thread thread2(func);
    thread1.join();
    thread2.join();
    auto now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::cout << now - last << std::endl;
    std::cout << shared_var << std ::endl;
    return 0;
}
```



## 死锁





### 防范死锁的准则：



1、使用固定的顺序依次获取锁



## 并发的同步



### std::condition_variable条件变量

成员函数：

**void wait( std::unique_lock< std::mutex> >& lock,  Predicate pred );**

wait导致当前线程被阻塞直到条件变量被通知，或者虚假唤醒发生，等待期间将解锁互斥。

> 虚假唤醒：线程重新获得互斥，并查验条件，而这一行为却不是通过另一个线程的通知。

```cpp
// 当传递谓词时，其等价于
While(!pred())
	wait(lock);
// 本质上是忙等的优化
```

当谓词返回false时，线程进入阻塞或等待状态，解锁互斥；

当谓词返回true时，线程从wait()中返回，互斥仍被锁住。



**wait_for()**

阻塞当前线程，直到条件变量被唤醒，或者到达指定时长

**wait_until()**

阻塞当前线程，直到条件变量被唤醒，或者到指定时间点

**notify_one**()

通知一个等待的线程

**notify_all**()

通知所有等待的线程



使用条件变量实现生产者消费者模型

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
std::mutex g_mutex;
std::condition_variable g_cv;
std::queue<int> g_queue;

void Producer() {
    for (int i = 0; i < 10; i++) {
        {            
            std::unique_lock<std::mutex> lock(g_mutex);
            g_queue.push(i);            
            std::cout << "Producer: produced " << i << std::endl;
        }
        g_cv.notify_one();        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
void Consumer() {    
    while (true) {        
        std::unique_lock<std::mutex> lock(g_mutex);
        g_cv.wait(lock, []() { return !g_queue.empty(); });        
        int value = g_queue.front();
        g_queue.pop();        
        std::cout << "Consumer: consumed " << value << std::endl;
    }
}
int main() {    
    std::thread producer_thread(Producer);    
    std::thread consumer_thread(Consumer);
    producer_thread.join();
    consumer_thread.join();    
    return 0;
}
```



### 异步并发

#### std::future std::async

std::async使用异步的方式启动任务，从其返回值中获得std::future对象。

可以指定第一个参数：std::lauch

**std::lauch::deferred** 	在当前线程上延后调用任务函数，直到遇到在futrue上get/wait才会执行任务函数；

**std::lauch::async**		开启专属的线程，在新线程上运行任务函数；



```cpp
#include <iostream>
#include <future>
#include <thread>
int find_the_answer_to_ltuae(){
    int a = 0;
    for (int i = 0; i < 100;++i){
        ++a;
    }
    return a;
}
int main(){
    std::future<int> future_result = std::async(std::launch::async, find_the_answer_to_ltuae);
    std::cout << find_the_answer_to_ltuae() << std::endl;
    std::cout << future_result.get() << std::endl;
```



#### std::packaged_task

连接**future对象**和**可调用函数**。

std::packaged_task是一个类模板，其模板参数是函数签名。例如：<void()> 代表了一个不接受参数和没有返回值的函数。



```cpp
#include <iostream>
#include <future>
#include <thread>
int find_the_answer_to_ltuae(){
    int a = 0;
    for (int i = 0; i < 100;++i){
        ++a;
    }
    return a;
}
int main(){
    std::packaged_task<int()> task(find_the_answer_to_ltuae);
    auto future_packged_result = task.get_future();
    std::thread t1(std::move(task));
    t1.join();
    std::cout << future_packged_result.get() << std::endl;
    return 0;
}
```



#### std::promise





### 线程间传递变量



#### 1、通过共享变量



#### 2、通过参数传递



#### 3、std::promise

```cpp
#include <iostream>
#include <future>

void func(std::promise<int> &f){
    f.set_value(1000);
}

int main(int argc, char const *argv[])
{
    std::promise<int> f;
    std::future<int> future_res = f.get_future();
    std::thread t1(func, std::ref(f));
    t1.join();
    std::cout << future_res.get() << std::endl;
    return 0;
}

```



## 线程池

简易的线程池模型

```cpp
#include <iostream>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>
#include <functional>
class ThreadPool{
public:
    ThreadPool(int numThreads) : stop(false){
        for (int i = 0; i < numThreads; ++i){
            threads.emplace_back([this]{
                while(1){
                    std::unique_lock<std::mutex> lock(mtx);
                    // 如果任务为或者stop为false就等待被唤醒
                    condition.wait(lock, [this]{ return !tasks.empty() || stop; });
                    if(stop && tasks.empty()){
                        return;
                    }
                    // 取任务
                    std::function<void()> task(std::move(tasks.front()));
                    tasks.pop();
                    // 解锁
                    lock.unlock();
                    // 执行任务
                    task();
                } });
        }
    }
    ~ThreadPool(){
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        // 把任务队列任务取完
        condition.notify_all();
        // 等待任务完成
        for(auto &t : threads){
            t.join();
        }
    }
    // 添加任务
    template <class F, class... Args>
    void enqueue(F &&f, Args &&...args){
        std::function<void()> task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // 对tasks加锁
        std::unique_lock<std::mutex> lock(mtx);
        tasks.emplace(std::move(task));
        lock.unlock();
        // 通知
        condition.notify_one();
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable condition;
    bool stop;
};

int main(){
    ThreadPool pool(4);
    for (int i = 0; i < 10; ++i){
        pool.enqueue([i]{ 
            std::cout << "task : " << i << "is running" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "task : " << i << "is done" << std::endl;
            });
    }
    return 0;
}
```



// TODO 线程池的单例模式实现
