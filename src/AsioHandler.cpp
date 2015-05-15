#include <iostream>

#include "AsioHandler.h"

using boost::asio::ip::tcp;

class AmqpBuffer
{
public:
    AmqpBuffer(size_t size) :
            m_data(size, 0),
            m_use(0)
    {
    }

    size_t write(const char* data, size_t size)
    {
        if (m_use == m_data.size())
        {
            return 0;
        }

        const size_t length = (size + m_use);
        size_t write = length < m_data.size() ? size : m_data.size() - m_use;
        memcpy(m_data.data() + m_use, data, write);
        m_use += write;
        return write;
    }

    void drain()
    {
        m_use = 0;
    }

    size_t available() const
    {
        return m_use;
    }

    const char* data() const
    {
        return m_data.data();
    }

    void shl(size_t count)
    {
        assert(count < m_use);

        const size_t diff = m_use - count;
        std::memmove(m_data.data(), m_data.data() + count, diff);
        m_use = m_use - count;
    }

private:
    std::vector<char> m_data;
    size_t m_use;
};

AsioHandler::AsioHandler(boost::asio::io_service& ioService) :
        m_ioService(ioService),
        m_socket(ioService),
        m_asioInputBuffer(ASIO_INPUT_BUFFER_SIZE, 0),
        m_amqpBuffer(new AmqpBuffer(ASIO_INPUT_BUFFER_SIZE * 2)),
        m_connection(nullptr),
        m_writeInProgress(false),
        m_connected(false),
        m_quit(false)
{
}

AsioHandler::~AsioHandler()
{
}

void AsioHandler::connect(const std::string& host, uint16_t port)
{
    doConnect(host, port);
}

void AsioHandler::doConnect(const std::string& host, uint16_t port)
{
    tcp::resolver::query query(host, std::to_string(port));
    tcp::resolver::iterator iter = tcp::resolver(m_ioService).resolve(query);

    boost::asio::async_connect(m_socket, iter,
            [this](boost::system::error_code ec, tcp::resolver::iterator)
            {
                m_connected = true;
                if (!ec)
                {
                    doRead();

                    if(!m_outputBuffer.empty())
                    {
                        doWrite();
                    }
                }
                else
                {
                    std::cerr<<ec<<std::endl;
                }
            });

}

void AsioHandler::onData(
        AMQP::Connection *connection, const char *data, size_t size)
{
    m_connection = connection;

    m_outputBuffer.push_back(std::vector<char>(data, data + size));
    if (!m_writeInProgress && m_connected)
    {
        doWrite();
    }
}

void AsioHandler::doRead()
{
    m_socket.async_receive(boost::asio::buffer(m_asioInputBuffer),
            [this](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    m_amqpBuffer->write(m_asioInputBuffer.data(), length);
                    parseData();
                    doRead();
                }
                else
                {
                    std::cerr<<ec<<std::endl;
                    exit(1);
                }
            });
}

void AsioHandler::doWrite()
{
    m_writeInProgress = true;
    boost::asio::async_write(m_socket,
            boost::asio::buffer(m_outputBuffer.front()),
            [this](boost::system::error_code ec, std::size_t length )
            {
                if(!ec)
                {
                    m_outputBuffer.pop_front();
                    if(!m_outputBuffer.empty())
                    {
                        doWrite();
                    }
                    else
                    {
                        m_writeInProgress = false;
                    }

                    if(m_quit)
                    {
                        m_socket.close();
                    }
                }
                else
                {
                    std::cerr<<ec<<std::endl;
                    m_socket.close();
                }
            });
}

void AsioHandler::parseData()
{
    if (m_connection == nullptr)
    {
        return;
    }

    const size_t count = m_connection->parse(m_amqpBuffer->data(),
            m_amqpBuffer->available());

    if (count == m_amqpBuffer->available())
    {
        m_amqpBuffer->drain();
    }
    else if (count > 0)
    {
        m_amqpBuffer->shl(count);
    }
}

void AsioHandler::onConnected(AMQP::Connection *connection)
{
    //m_connected = true;
}
bool AsioHandler::connected() const
{
    return m_connected;
}

void AsioHandler::onError(AMQP::Connection *connection, const char *message)
{
    std::cerr << "AMQP error " << message << std::endl;
}

void AsioHandler::onClosed(AMQP::Connection *connection)
{
    std::cout << "AMQP closed connection" << std::endl;
    m_quit = true;
    if (!m_writeInProgress)
    {
        m_socket.close();
    }
}
