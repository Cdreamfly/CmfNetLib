#pragma once

#include <functional>
#include <memory>

#include "base/noncopyable.hpp"
#include "base/Timestamp.h"

class EventLoop;

namespace cm {
    namespace net {
        class Channel : private noncopyable {
        public:
            using EventCallback = std::function<void()>;
            using ReadEventCallback = std::function<void(Timestamp)>;
        public:
            Channel(EventLoop *, int);

            ~Channel();

            bool isNoneEvent() const { return _events == kNoneEvent; }

            bool IsWriting() const { return _events == kWriteEvent; }

            bool IsReading() const { return _events == kReadEvent; }

            void handleEvent(Timestamp);

            void setReadCallback(ReadEventCallback cb) { _readEventCallback = std::move(cb); }

            void setWriteCallback(EventCallback cb) { _writeCallback = std::move(cb); }

            void setCloseCallback(EventCallback cb) { _closeCallback = std::move(cb); }

            void setErrorCallback(EventCallback cb) { _errorCallback = std::move(cb); }

            void enableReading() {
                _events |= kReadEvent;
                update();
            }

            void disableReading() {
                _events &= ~kReadEvent;
                update();
            }

            void enableWriting() {
                _events |= kWriteEvent;
                update();
            }

            void disableWriting() {
                _events &= ~kWriteEvent;
                update();
            }

            void disableAll() {
                _events = kNoneEvent;
                update();
            }

            void remove();

            void tie(const std::shared_ptr<void> &);

            void setRevents(const int events) { _revents = events; }

            int fd() const { return _fd; }

            int events() const { return _events; }

            int index() const { return _index; }

            void setIndex(const int index) { _index = index; }

            EventLoop *ownerLoop() { return _loop; }

        private:
            void update();

            void handleEventWithGuard(Timestamp);

        private:
            static int kNoneEvent;
            static int kReadEvent;
            static int kWriteEvent;

            EventLoop *_loop{};
            const int _fd;
            int _events{};
            int _revents{};
            int _index{};
            std::weak_ptr<void> _tie;
            bool _tied{};

            ReadEventCallback _readEventCallback;
            EventCallback _writeCallback;
            EventCallback _closeCallback;
            EventCallback _errorCallback;
        };
    }
}