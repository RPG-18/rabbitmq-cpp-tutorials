#include <iostream>

#include "SimplePocoHandler.h"
#include "tools.h"

int main(int argc, const char* argv[])
{
    std::string severity = "info";
    std::string msg = "Hello World!";

    if (argc > 2)
    {
        severity = argv[1];
        msg = join(&argv[2], &argv[argc], " ");
    }

    SimplePocoHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareExchange("direct_logs",AMQP::direct).onSuccess([&]() {
        channel.publish("direct_logs", severity, msg);
        std::cout << " [x] Sent "<<severity<<":"<<msg<< std::endl;
        handler.quit();
    });

    handler.loop();
    return 0;
}
