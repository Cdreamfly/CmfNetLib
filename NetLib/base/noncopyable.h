//
// Created by Cmf on 2022/5/23.
//

#ifndef CMFNETLIB_NONCOPYABLE_H
#define CMFNETLIB_NONCOPYABLE_H

/*
 * 允许构造和析构禁止拷贝和赋值
 */
class noncopyable {
public:
    noncopyable(const noncopyable &) = delete;

    noncopyable operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;

    ~noncopyable() = default;
};

#endif //CMFNETLIB_NONCOPYABLE_H
