#pragma once

#include "base/timestamp.h"

#include <chrono>

cm::Timestamp::Timestamp() : _microSecondsSinceEpoch(0) {}

cm::Timestamp::Timestamp(const int64_t microSecondsSinceEpoch) : _microSecondsSinceEpoch(microSecondsSinceEpoch) {}

cm::Timestamp cm::Timestamp::now() {
    return Timestamp(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

std::string cm::Timestamp::toString() const {
    tm *ptm = localtime(&_microSecondsSinceEpoch);
    char date[128] = {0};
    sprintf(date, "%4d-%02d-%02d %02d:%02d:%02d",
            static_cast<int>(ptm->tm_year + 1900),
            static_cast<int>(ptm->tm_mon + 1),
            static_cast<int>(ptm->tm_mday),
            static_cast<int>(ptm->tm_hour),
            static_cast<int>(ptm->tm_min),
            static_cast<int>(ptm->tm_sec));
    return date;
}