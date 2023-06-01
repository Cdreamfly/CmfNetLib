#pragma once

namespace muduo
{
    class noncopyable
    {
    private:
        noncopyable(const noncopyable &) = delete;
        const noncopyable &operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };
}