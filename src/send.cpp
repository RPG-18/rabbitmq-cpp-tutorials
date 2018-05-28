#include <iostream>
#include <Poco/Net/IPAddress.h>

#include "SimplePocoHandler.h"

int main(void)
{
    const Poco::Net::IPAddress ip("127.0.0.1");
    const SimplePocoHandler handler(ip, 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    channel.onReady([&]()
    {
        if (handler.connected())
        {
            channel.publish("", "hello", "Hello World!");
            std::cout << " [x] Sent 'Hello World!'" << std::endl;
            handler.quit();
        }
    });

    handler.loop();
    return 0;
}
