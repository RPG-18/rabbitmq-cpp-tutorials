#include <iostream>

#include "SimplePocoHandler.h"
#include "tools.h"

int main(int argc, const char* argv[])
{
    const std::string msg =
            argc > 1 ? join(&argv[2], &argv[argc], " ") : "Hello World!";
    const std::string routing_key = argc > 1 ? argv[1] : "anonymous.info";

    SimplePocoHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareExchange("topic_logs", AMQP::topic).onSuccess([&]()
    {
        channel.publish("topic_logs", routing_key, msg);
        std::cout << " [x] Sent "<<routing_key<<":"<<msg<< std::endl;
        handler.quit();
    });

    handler.loop();
    return 0;
}
