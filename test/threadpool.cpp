//
// Created by Cmf on 2022/5/31.
//

#include <iostream>
#include "NetLib/log/Log.hpp"
#include "NetLib/thread/ThreadPool.hpp"

int main() {

    ThreadPool *pool = new ThreadPool(4);
    std::vector<std::future<int>> rets;
    for (int i = 0; i < 100; i++) {
        rets.emplace_back(std::move(pool->Commit([i]() -> int {
            LOG_INFO("%d", std::this_thread::get_id());
            return i;
        })));
    }
    for (auto &ret: rets) {
        LOG_INFO("%d", ret.get());
    }
    delete pool;
    return 0;
}