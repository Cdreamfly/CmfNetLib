//
// Created by Cmf on 2022/5/23.
//

#ifndef CMFNETLIB_NONCOPYABLE_H
#define CMFNETLIB_NONCOPYABLE_H

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif //CMFNETLIB_NONCOPYABLE_H
