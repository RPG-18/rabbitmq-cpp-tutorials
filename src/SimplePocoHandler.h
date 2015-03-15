#ifndef SRC_SIMPLEPOCOHANDLER_H_
#define SRC_SIMPLEPOCOHANDLER_H_

#include <amqpcpp.h>
#include <Poco/Net/StreamSocket.h>

class SimplePocoHandler: public AMQP::ConnectionHandler
{
public:

    SimplePocoHandler(const std::string& host, uint16_t port);
    virtual ~SimplePocoHandler();

    void loop();
    void quit();

    bool connected()const;

private:

    SimplePocoHandler(const SimplePocoHandler&);
    SimplePocoHandler& operator=(const SimplePocoHandler&);

    void close();

    virtual void onData(AMQP::Connection *connection, const char *data,
            size_t size);

    virtual void onConnected(AMQP::Connection *connection);

    virtual void onError(AMQP::Connection *connection, const char *message);

    virtual void onClosed(AMQP::Connection *connection);

private:

    Poco::Net::StreamSocket m_socket;
    bool m_connected;
    AMQP::Connection* m_connection;
    bool m_needExit;
};

#endif /* SRC_SIMPLEPOCOHANDLER_H_ */
