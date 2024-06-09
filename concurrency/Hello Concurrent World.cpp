#include <iostream>
#include <thread>

void hello(){
    std::cout << "hello Concurrent world!\n";
}
int main(int argc, char const *argv[])
{
    std::thread t(hello);
    t.join();
    return 0;
}
