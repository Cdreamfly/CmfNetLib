#pragma once

#include <functional>
#include <memory>

#include "base/NonCopyable.hpp"
#include "base/Timestamp.hpp"

class EventLoop;

namespace cm {
    namespace net {
        class Channel : private NonCopyable {
        public:
            using EventCallback = std::function<void()>;
            using ReadEventCallback = std::function<void(Timestamp)>;
        public:

        };
    }
}