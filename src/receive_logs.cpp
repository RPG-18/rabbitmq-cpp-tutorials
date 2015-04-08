#include <iostream>

#include "SimplePocoHandler.h"

int main(void)
{
    SimplePocoHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    auto receiveMessageCallback = [](const AMQP::Message &message,
            uint64_t deliveryTag,
            bool redelivered)
    {

        std::cout <<" [x] "<<message.message() << std::endl;
    };

    AMQP::QueueCallback callback =
            [&](const std::string &name, int msgcount, int consumercount)
            {
                channel.bindQueue("logs", name,"");
                channel.consume(name, AMQP::noack).onReceived(receiveMessageCallback);
            };

    AMQP::SuccessCallback success = [&]()
            {
                channel.declareQueue(AMQP::exclusive).onSuccess(callback);
            };

    channel.declareExchange("logs", AMQP::fanout).onSuccess(success);

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";
    handler.loop();
    return 0;
}
