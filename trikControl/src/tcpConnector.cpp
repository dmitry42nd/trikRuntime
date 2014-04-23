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

#include "src/tcpConnector.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <QtCore/QDataStream>

using namespace trikControl;

TcpConnector::TcpConnector(int port)
	: mPort(port)
  , mOpenConnectionsMask()
  , mTcpSocketReadyReadSignalMapper(new QSignalMapper())
  , mTcpSocketDisconnectedSignalMapper(new QSignalMapper())
	, mTcpServer(new QTcpServer())
{
}

void TcpConnector::startServer()
{
	if (!mTcpServer->listen(QHostAddress::Any, mPort)) {
		qDebug() << "Unable to start the server:" << mTcpServer->errorString();
		mTcpServer->close();
		return;
	}

	qDebug() << "TcpServer started";
	connect(mTcpServer.data(), SIGNAL(newConnection()), this, SLOT(connection()));
}


int TcpConnector::getFreeSocket()
{
  for (int i = 0; i < 4; ++i)
    if(!(mOpenConnectionsMask >> i & 1))
      return i;

  return -1;
}

void TcpConnector::connection()
{
  int connectionId;
  if((connectionId = getFreeSocket()) < 0)
  {
		qDebug() << "Unable to open new connection. Max amount of open connections is " << mMaxOpenConnections;
    return;
  }

  disconnect(mTcpSocketDisconnectedSignalMapper.data(), SIGNAL(mapped(int)), this, SLOT(tcpDisconnected(int)));
  disconnect(mTcpSocketReadyReadSignalMapper.data(), SIGNAL(mapped(int)), this, SLOT(networkRead(int)));

/*
  mTcpSocketDisconnectedSignalMapper.reset(new QSignalMapper());
  mTcpSocketReadyReadSignalMapper.reset(new QSignalMapper());
*/

  mTcpSocket[connectionId].reset(mTcpServer->nextPendingConnection());

  mTcpSocket[connectionId]->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
  mTcpSocket[connectionId]->setSocketOption(QAbstractSocket::LowDelayOption, 1);

  int fd = mTcpSocket[connectionId]->socketDescriptor();
  setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &mCnt, sizeof(mCnt));
  setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &mIdle, sizeof(mIdle));
  setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &mIntvl, sizeof(mIntvl));

  qDebug() << "Set new connection";

/*
  connect(mTcpSocket[connectionId].data(), SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
  connect(mTcpSocket[connectionId].data(), SIGNAL(readyRead()), this, SLOT(networkRead()));
*/

  connect(mTcpSocket[connectionId].data(), SIGNAL(disconnected()), mTcpSocketDisconnectedSignalMapper.data(), SLOT(map()));
  connect(mTcpSocket[connectionId].data(), SIGNAL(readyRead()), mTcpSocketReadyReadSignalMapper.data(), SLOT(map()));

  mTcpSocketDisconnectedSignalMapper->setMapping(mTcpSocket[connectionId].data(), connectionId);
  mTcpSocketReadyReadSignalMapper->setMapping(mTcpSocket[connectionId].data(), connectionId);

  connect(mTcpSocketDisconnectedSignalMapper.data(), SIGNAL(mapped(int)), this, SLOT(tcpDisconnected(int)));
  connect(mTcpSocketReadyReadSignalMapper.data(), SIGNAL(mapped(int)), this, SLOT(networkRead(int)));

  mOpenConnectionsMask += 1 << connectionId;
  qDebug() << "openConnectionsMask connected: " << mOpenConnectionsMask;
}

void TcpConnector::tcpDisconnected(/*int _connectionId*/)
{
//  qDebug() << _connectionId;

	mTcpSocket[0/*_connectionId*/]->abort();
//  mOpenConnectionsMask -= 1 << _connectionId;
  qDebug() << "openConnectionsMask disconnected: " << mOpenConnectionsMask;
}

void TcpConnector::networkRead(/*int _connectionId*/)
{
  //qDebug() << _connectionId;

	if (!mTcpSocket[0/*_connectionId*/]->isValid()) {
		return;
	}

	QString line;
	while (mTcpSocket[0/*_connectionId*/]->bytesAvailable() > 0) {
		char data[100];
		mTcpSocket[0/*_connectionId*/]->readLine(data, 100);
		line += data;
	}

	emit dataReady(line, 0/*_connectionId*/);
}
