#include "singleapplication.h"

DSingleApplication::DSingleApplication(const QString &id, int &argc, char *argv[]):
  QApplication(argc, argv) {
    other_instance_running = false;
    app_id = id;
    port = d_unique_port_start;

    tcp_server = new DTalker(app_id, this);
    tcp_socket = NULL;

    // start at d_unique_port_start and go until find a free port or a port
    // that answers correctly
    other_instance_running = false;

    DPortChecker checker(app_id, d_unique_port_start, this);

    // first go over the range of ports and check for other instance
    // if not, then listen on the forst port available
    DPortList ports;

    while ( port <= d_unique_port_finish ) {

        // here check if the stuff running on port is our instance if not procede
        checker.check( port );
        checker.wait();
        DPortChecker::PortStatus port_status = checker.status();

        if ( port_status == DPortChecker::us ) {
            other_instance_running = true;
            // here we have to connect to other instance to send messages
            tcp_socket = checker.transferSocketOwnership();
            return;
        }

        DPortInfo pi(port, checker.status() == DPortChecker::free);
        ports << pi;
        ++port;
    }

    port = ports.firstFreePort();

    // other instance is not running in the range and there's available port
    if ( port == -1 )
        return;

    // this port is free and current instance is in listening mode
    bool listening = tcp_server->listen( QHostAddress::LocalHost, port );
    if ( listening )
        connect(tcp_server, SIGNAL(messageReceived(const QString&)), this, SLOT(onClientMessage(const QString&)));
}

DSingleApplication::~DSingleApplication() {
    if ( tcp_server )
        delete(tcp_server);
    if ( tcp_socket )
        delete(tcp_socket);
}

QString DSingleApplication::id() {
    return app_id;
}

bool DSingleApplication::isRunning () {
    // may require some checks here
    return other_instance_running;
}

bool DSingleApplication::sendMessage(const QString &message) {
    if ( ! other_instance_running )
        return false;
    if ( ! tcp_socket )
        return false;
    if ( tcp_socket->state() != QAbstractSocket::ConnectedState )
        return false;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(out.version()); // set to the current Qt version
  
    QString msg = app_id + ":" + message;
    out << (quint32) msg.size();
    out << msg;

    tcp_socket->write(block);
    tcp_socket->flush();
    tcp_socket->waitForBytesWritten(d_timeout_try_write);

    return true;
}

void DSingleApplication::onClientMessage(const QString &message) {
    emit messageReceived(message);
}

// This class is used to check specific port if it has an instance of this app.

DPortChecker::DPortChecker(const QString &id, int port, QObject *parent): QThread(parent) {
    app_id = QString(id);
    tcp_socket = NULL;
    this->port = port;
    result = DPortChecker::free;
}

DPortChecker::~DPortChecker() {
    if ( tcp_socket )
        delete(tcp_socket);
}

DPortChecker::PortStatus DPortChecker::status() {
    return result;
}

void DPortChecker::check(int port) {
    this->port = port;
    start();
}

void DPortChecker::run() {
    result = DPortChecker::free;

    if ( tcp_socket == NULL )
        tcp_socket = new QTcpSocket();

    tcp_socket->connectToHost(QHostAddress(QHostAddress::LocalHost), port);
    if ( ! tcp_socket->waitForConnected(d_timeout_try_connect) ) {
        tcp_socket->abort();
        return;
    }

    result = DPortChecker::others;
    if ( ! tcp_socket->waitForReadyRead(d_timeout_try_read) ) {
        tcp_socket->abort();
        return;
    }

    // now compare received bytes with app_id
    QDataStream in(tcp_socket);
    in.setVersion(in.version()); // set to the current Qt version

    if ( tcp_socket->bytesAvailable() > 0 ) {
        QString msgString;
        in >> msgString;
        if ( msgString.size() <= 1 ) {
            tcp_socket->abort();
            return;
        }
        int s = qMin(msgString.size(), app_id.size());
        if ( QString::compare(msgString.left(s), app_id.left(s)) == 0 )
            result = DPortChecker::us;
    }
}

QTcpSocket *DPortChecker::transferSocketOwnership() {
    QTcpSocket *tmp = tcp_socket;
    tcp_socket = NULL;
    return tmp;
}

// This is a server responsible to talking to incoming connections.

DTalker::DTalker(const QString &id, QObject *parent): QTcpServer(parent) {
    app_id = QString(id);
}

void DTalker::incomingConnection(int socket_descriptor ) {
    DListner *listner = new DListner(app_id, socket_descriptor, this);
    connect(listner, SIGNAL(messageReceived(const QString&)), this, SLOT(onClientMessage(const QString&)));
    connect(listner, SIGNAL(finished()), listner, SLOT(deleteLater()));
    listner->start();
}

void DTalker::onClientMessage(const QString &message) {
    emit messageReceived( message );
}

// This thread is used to communicate with connected client.

DListner::DListner(const QString &id, int _socket_descriptor, QObject *parent): QThread(parent) {
    app_id = QString(id);
    socket_descriptor = _socket_descriptor;
    block_size = 0;
}

void DListner::run() {
    QTcpSocket tcp_socket;
    if ( ! tcp_socket.setSocketDescriptor(socket_descriptor) )
        return;

    // send app_id to client
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(out.version()); // set to the current Qt version instead
    out << app_id;
    tcp_socket.write(block);
    //waitForBytesWritten ( int msecs )

    while ( true ) {
        if ( tcp_socket.state() != QAbstractSocket::ConnectedState )
            return;
        tcp_socket.waitForReadyRead(-1);
        read(&tcp_socket);
    }
}

void DListner::read( QTcpSocket *tcp_socket ) {
    if (tcp_socket == NULL)
        return;
    if ( tcp_socket->state() != QAbstractSocket::ConnectedState )
        return;

    QDataStream in(tcp_socket);
    in.setVersion(in.version()); // set to the current Qt version instead

    if ( block_size == 0 ) {
        if ( tcp_socket->bytesAvailable() < (int) sizeof(quint32) )
            return;
        in >> block_size;
    }
    if ( tcp_socket->bytesAvailable() < block_size )
        return;
    QString msgString;
    in >> msgString;
  
    // if header matches, trim and emit
    QString magic = app_id + ":";
    if ( QString::compare( msgString.left(magic.size()), magic ) == 0)
        emit messageReceived( msgString.remove(0, magic.size()) );

    block_size = 0;
    if ( tcp_socket->bytesAvailable() > 0 )
        read(tcp_socket);
}

int DPortList::firstFreePort() {
    DPortList::iterator it = this->begin();
    while ( it < this->end() ) {
        if ( it->free )
            return it->port;
        ++it;
    }
    return -1;
}

bool DPortList::freePortAvailable() {
    int p = firstFreePort();
    return p != -1;
}
