#include <iostream>
#include <thread>
#include <chrono>
#include <assert.h>
struct func{
    int *i;
    func(int *i_) : i(i_){};
    void operator()(){
        for (unsigned j = 0; j < 1e8;++j){
            std::cout << *i << "\n";
            ++(*i);
        }
    }
};
void oops(){
    int a = 0;
    func my_func(&a);
    std::thread my_thread(my_func);
    my_thread.detach();
    // assert(my_thread.joinable());
    std::this_thread::sleep_for(std::chrono::seconds(3));
}
int main(){
    oops();
    std::cout << "end\n";
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}