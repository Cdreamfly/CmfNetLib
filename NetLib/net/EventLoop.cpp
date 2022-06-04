//
// Created by Cmf on 2022/6/2.
//

#ifndef CMFNETLIB_EVENTLOOP_CPP
#define CMFNETLIB_EVENTLOOP_CPP

#include<memory>

class Channel;

class EventLoop {
public:
    using ptr = std::shared_ptr<EventLoop>;

    void UpdateChannel(Channel *channel);

    void RemoveChannel(Channel *channel);

private:

};

#endif //CMFNETLIB_EVENTLOOP_CPP
