#pragma once

#include <deque>
#include <vector>
#include <memory>
#include <amqpcpp.h>

#include <boost/asio.hpp>

class AmqpBuffer;
class AsioHandler: public AMQP::ConnectionHandler
{
public:

    static constexpr size_t ASIO_INPUT_BUFFER_SIZE = 4*1024; //4kb

    AsioHandler(boost::asio::io_service& ioService);

    void connect(const std::string& host, uint16_t port);

    virtual ~AsioHandler();

    AsioHandler(const AsioHandler&) = delete;
    AsioHandler& operator=(const AsioHandler&)=delete;

    bool connected()const;

private:

    typedef std::deque<std::vector<char>> OutputBuffers;

    virtual void onData(AMQP::Connection *connection, const char *data, size_t size);
    virtual void onConnected(AMQP::Connection *connection);
    virtual void onError(AMQP::Connection *connection, const char *message);
    virtual void onClosed(AMQP::Connection *connection);

    void doConnect(const std::string& host, uint16_t port);

    void doRead();

    void doWrite();

    void parseData();

private:

    boost::asio::io_service& m_ioService;
    boost::asio::ip::tcp::socket m_socket;

    std::vector<char> m_asioInputBuffer;
    std::shared_ptr<AmqpBuffer> m_amqpBuffer;
    AMQP::Connection* m_connection;
    OutputBuffers m_outputBuffer;
    bool m_writeInProgress;
    bool m_connected;
    bool m_quit;
};

