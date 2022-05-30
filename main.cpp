#include <iostream>
#include "NetLib/log/log.hpp"
#include <vector>

int main() {

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.push_back(std::thread([]() {
            for(int i = 0 ;i<10;++i)
            {
                LOG_INFO("Just a FMT INFO Test! %d, %s", 1, "中文");
                //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }));
    }
    for (auto &thread: threads) {
        thread.join();
    }
    return 0;
}
