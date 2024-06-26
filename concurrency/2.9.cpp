// 并发版本的accumulate
#include <iostream>
#include <thread>
#include <algorithm>
#include <numeric>
#include <vector>
#include <chrono>
#include <exception>
struct empty_stack : public std::exception{
    const char *what() const throw();
};
template<typename Iterator, typename T>
struct accumlate_block{
    void operator()(Iterator first, Iterator last, T& result){
        result = std::accumulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init){
    auto length = std::distance(first, last);
    if(!length)
        return init;
    const long min_per_thread = 25;
    const long max_threads = (length + min_per_thread - 1) / min_per_thread;
    const long hardware_threads = std::thread::hardware_concurrency();
    const long num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2,
                                        max_threads);
    const long block_size = length / num_threads;
    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1);++i){
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(
            accumlate_block<Iterator, T>(), block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    accumlate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);
    for(auto& entry: threads){
        entry.join();
    }
    return std::accumulate(results.begin(), results.end(), init);
}

int main(int argc, char const *argv[])
{
    std::vector<int> a =  { 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                            1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                            1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                            1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                            1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    auto beforeTime = std::chrono::steady_clock::now();
    std::cout << parallel_accumulate(a.begin(), a.end(), 0) << std::endl;
    auto afterTime = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration<double>(afterTime - beforeTime).count() << std::endl;
    beforeTime = std::chrono::steady_clock::now();
    std::cout << std::accumulate(a.begin(), a.end(), 0)<<std::endl;
    afterTime = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration<double>(afterTime - beforeTime).count() << std::endl;
    beforeTime = std::chrono::steady_clock::now();
    std::cout << std::reduce(a.begin(), a.end(), 0) << std::endl;
    afterTime = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration<double>(afterTime - beforeTime).count() << std::endl;
    return 0;
}
