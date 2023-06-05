#pragma once

#include <cstdint>
#include <string>

namespace cm {
    class Timestamp {
    public:
        Timestamp();

        explicit Timestamp(int64_t);

        static Timestamp now();

        std::string toString() const;

    private:
        int64_t _microSecondsSinceEpoch;
    };
}