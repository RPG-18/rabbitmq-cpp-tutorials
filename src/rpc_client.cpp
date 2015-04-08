#include <iostream>

#include "tools.h"
#include "SimplePocoHandler.h"

int main(int argc, const char* argv[])
{
    const std::string correlation(uuid());

    SimplePocoHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    AMQP::QueueCallback callback = [&](const std::string &name,
            int msgcount,
            int consumercount)
    {
        AMQP::Envelope env("30");
        env.setCorrelationID(correlation);
        env.setReplyTo(name);
        channel.publish("","rpc_queue",env);
        std::cout<<" [x] Requesting fib(30)"<<std::endl;

    };
    channel.declareQueue(AMQP::exclusive).onSuccess(callback);

    auto receiveCallback = [&](const AMQP::Message &message,
            uint64_t deliveryTag,
            bool redelivered)
    {
        if(message.correlationID() != correlation)
            return;

        std::cout<<" [.] Got "<<message.message()<<std::endl;
        handler.quit();
    };

    channel.consume("", AMQP::noack).onReceived(receiveCallback);

    handler.loop();
    return 0;
}
