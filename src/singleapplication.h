/*
 * DSingleApplication
 * Copyright (C) 2013 Sorokin Alexei <sor.alexei@meowr.ru>
 * Copyright (C) 2007 Dima Fedorov Levit <dimin@dimin.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://gnu.org/licenses/>.
 */

#ifndef SINGLE_APPLICATION_H
#define SINGLE_APPLICATION_H

const int d_unique_port_start  = 33232;
const int d_unique_port_finish = 33242;

// timeouts are in ms
const int d_timeout_try_connect = 10; 
const int d_timeout_try_read = 1000;
const int d_timeout_try_write = 30000;

#include <QtCore>
#include <QtGui>
#ifndef IS_QT4
#include <QtWidgets>
#endif
#include <QtNetwork>

class DTalker;

class DSingleApplication: public QApplication {
    Q_OBJECT

public:
    DSingleApplication(const QString &id, int &argc, char *argv[]);
    ~DSingleApplication();
    QString id();
    bool isRunning();
 
public slots:
    bool sendMessage(const QString &message);

signals:
    void messageReceived(const QString &message);

protected slots:
  void onClientMessage(const QString &message);

private:
    // server is used if no other instance was found to start the port and wait for others
    DTalker *tcp_server;
    // socket is used if other instance was found to communicate with it
    QTcpSocket *tcp_socket;

    int port;
    QString app_id;
    bool other_instance_running;
};

// This class is used to check specific port if it has an instance of this app.

class DPortChecker: public QThread {
    Q_OBJECT

public:
    enum PortStatus { free=0, us=1, others=2 };

    DPortChecker(const QString &id, int port, QObject *parent = 0);
    ~DPortChecker();

    PortStatus status();
    void check(int port);
    QTcpSocket *transferSocketOwnership();

protected:
    void run();

private:
    PortStatus result;
    QTcpSocket *tcp_socket;
    int port;
    QString app_id;
};

// This is a server responsible to talking to incoming connections.

class DTalker: public QTcpServer {
    Q_OBJECT

public:
    DTalker(const QString &id, QObject *parent = 0);

signals:
    void messageReceived (const QString &message);

protected slots:
    void onClientMessage(const QString &message);

protected:
    void incomingConnection(int socket_descriptor);

private:
    QString app_id;
};

// This thread is used to communicate with connected client.

class DListner: public QThread {
    Q_OBJECT

public:
    DListner(const QString &id, int _socket_descriptor, QObject *parent);

signals:
    void messageReceived (const QString &message);

protected:
    void run();

private:
    int socket_descriptor;
    QString app_id;
    quint32 block_size;

    void read(QTcpSocket *tcp_socket );
};

class DPortInfo {

public:
    DPortInfo(int p, bool f): port(p), free(f) { }
    int port;
    bool free;
};

class DPortList: public QList<DPortInfo> {
public:
    int firstFreePort();
    bool freePortAvailable();
};

#endif // SINGLE_APPLICATION_H
