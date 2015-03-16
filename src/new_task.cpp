#include <iostream>

#include "SimplePocoHandler.h"
#include "tools.h"

int main(int argc, const char* argv[])
{
    const std::string msg =
            argc > 1 ? join(&argv[1], &argv[argc], " ") : "Hello World!";

    SimplePocoHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    auto callback =
            [&](const std::string &name, int msgcount, int consumercount)
            {
                channel.publish("", "task_queue", msg);
                std::cout<<" [x] Sent '"<<msg<<"'\n";
                handler.quit();
            };

    channel.declareQueue("task_queue", AMQP::durable).onSuccess(callback);
    handler.loop();
    return 0;
}
