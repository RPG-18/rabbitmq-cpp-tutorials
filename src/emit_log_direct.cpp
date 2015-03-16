#include <iostream>

#include "SimplePocoHandler.h"
#include "tools.h"

int main(int argc, const char* argv[])
{
    const std::string severity = argc > 2 ? argv[1] : "info";
    const std::string msg =
            argc > 2 ? join(&argv[2], &argv[argc], " ") : "Hello World!";

    SimplePocoHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareExchange("direct_logs", AMQP::direct).onSuccess([&]()
    {
        channel.publish("direct_logs", severity, msg);
        std::cout << " [x] Sent "<<severity<<":"<<msg<< std::endl;
        handler.quit();
    });

    handler.loop();
    return 0;
}
