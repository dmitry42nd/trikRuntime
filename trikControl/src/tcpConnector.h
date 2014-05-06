/* Copyright 2014 CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#pragma once

#include "src/gamepad_connection.h"

#include <vector>

#include <QtCore/QObject>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtCore/QScopedPointer>
#include <QSignalMapper>

namespace trikControl {

const int mCnt = 5;
const int mIdle = 5;
const int mIntvl = 5;
const int mMaxControls = 4; // MaxOpenConnections <= mMaxControls

/// TCP server.
class TcpConnector : public QObject
{
	Q_OBJECT

public:
	/// Constructor.
	/// @param port - TCP port of a server.
	TcpConnector(int port);

signals:
	/// Emitted when there is incoming TCP message.
	void dataReady(QString const &message, int const connectionId);
  void tcpDisconnectedSignal(int _cnId);

public slots:
	/// Starts a server and begins listening port for incoming connections.
	void startServer();

private slots:
	void connection();
  void tcpDisconnected(int _cnId);
  void networkRead(QObject* _cn);

private:
	int mPort;
  int mOpenConnectionsMask; // [0x0000..0x1111]

  QScopedPointer<QSignalMapper> mTcpSocketReadyReadSignalMapper;
  QScopedPointer<QSignalMapper> mTcpSocketDisconnectedSignalMapper;
	QScopedPointer<QTcpServer> mTcpServer;
	QScopedPointer<QTcpSocket> mTcpSocket[mMaxControls]; // !

  QScopedPointer<GamepadConnection> mCn[mMaxControls];


  void removeMappings(int _cnId);
  void resetMappings();
  int getFreeSocket(int* _cnId);
  int pop(int x)
  {
    x = x - ((x >> 1) & 0x5555);
    x = (x & 0x3333) + ((x >> 2) & 0x3333);
    x = (x + (x >> 4)) & 0x0f0f;
    x = x + (x >> 8);
    x = x + (x >> 16);

    return x & 0x003f;
  }

};

}
