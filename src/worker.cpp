#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

#include "SimplePocoHandler.h"

int main(void)
{
    SimplePocoHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareQueue("task_queue", AMQP::durable);
    channel.consume("task_queue", AMQP::noack).onReceived(
            [&channel](const AMQP::Message &message,
                       uint64_t deliveryTag,
                       bool redelivered)
            {
                const auto body = message.message();
                std::cout<<" [x] Received "<<body<<std::endl;

                size_t count = 0;
                std::for_each(body.cbegin(), body.cend(), [&](const char& ch)
                        {
                            if(ch =='.')
                            {
                                ++count;
                            }
                        });
                std::this_thread::sleep_for (std::chrono::seconds(count));

                std::cout<<" [x] Done"<<std::endl;
                channel.ack(deliveryTag);
            });

    channel.setQos(1);
    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";
    handler.loop();
    return 0;
}
