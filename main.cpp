#include <iostream>
#include "NetLib/log/log.hpp"

int main() {

    Logger& logger = Logger::GetInstance();
    LogEvent::ptr event = std::make_shared<LogEvent>("DEBUG",__FILE__,__LINE__,logger.GetCurrentSystemTime(),"hello");
    logger.Log(event);
    return 0;
}
