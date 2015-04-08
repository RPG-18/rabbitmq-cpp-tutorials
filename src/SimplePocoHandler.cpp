#include <vector>
#include <Poco/Net/StreamSocket.h>

#include "SimplePocoHandler.h"

struct SimplePocoHandlerImpl
{
    Poco::Net::StreamSocket socket;
};
SimplePocoHandler::SimplePocoHandler(const std::string& host, uint16_t port) :
        m_impl(new SimplePocoHandlerImpl), m_connected(false), m_connection(nullptr), m_needExit(false)
{
    const Poco::Net::SocketAddress address(host, port);
    m_impl->socket.connect(address);
}

SimplePocoHandler::~SimplePocoHandler()
{
    close();
}

void SimplePocoHandler::loop()
{
    std::vector<char> buff(1024, 0);
    while (!m_needExit)
    {
        int bytes = m_impl->socket.receiveBytes(buff.data(), buff.size());
        if (bytes > 0)
        {
            m_connection->parse(buff.data(), bytes);
        }
    }
}

void SimplePocoHandler::quit()
{
    m_needExit = true;
}

void SimplePocoHandler::SimplePocoHandler::close()
{
    m_impl->socket.close();
}

void SimplePocoHandler::onData(AMQP::Connection *connection, const char *data,
        size_t size)
{
    m_connection = connection;
    m_impl->socket.sendBytes(data, size);
}

void SimplePocoHandler::onConnected(AMQP::Connection *connection)
{
    m_connected = true;
}

void SimplePocoHandler::onError(AMQP::Connection *connection,
        const char *message)
{
}

void SimplePocoHandler::onClosed(AMQP::Connection *connection)
{
    m_needExit = true;
}

bool SimplePocoHandler::connected()const
{
    return m_connected;
}
