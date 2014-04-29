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
  , mTcpSocketReadyReadSignalMapper()
  , mTcpSocketDisconnectedSignalMapper()
	, mTcpServer(new QTcpServer())
  , mCn()
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

  mTcpSocketDisconnectedSignalMapper.reset(new QSignalMapper);
  mTcpSocketReadyReadSignalMapper.reset(new QSignalMapper);

  connect(mTcpSocketDisconnectedSignalMapper.data(), SIGNAL(mapped(int)), this, SLOT(tcpDisconnected(int)));
  connect(mTcpSocketReadyReadSignalMapper.data(), SIGNAL(mapped(QObject*)), this, SLOT(networkRead(QObject*)));

}

void TcpConnector::removeMappings(int _cnId)
{
      mTcpSocketDisconnectedSignalMapper->removeMappings(mTcpSocket[_cnId].data());
      mTcpSocketReadyReadSignalMapper->removeMappings(mTcpSocket[_cnId].data());
}

void TcpConnector::resetMappings()
{
  int openConnections = pop(mOpenConnectionsMask);

  for(int i = 0; i < mMaxControls; ++i)
    mCn[i].reset(new GamepadConnection());

  for(int i = 0; i < mMaxControls; ++i)
  {
    int cnNum = (i*openConnections/mMaxControls);
    mCn[cnNum]->addToMask(1<<i);
  }

  int cnCtr = 0;
  for(int i = 0; i < mMaxControls; ++i)
  {
    removeMappings(i);
    if(mOpenConnectionsMask & 1<<i)
    {
      mCn[cnCtr]->setId(i);

      disconnect(mTcpSocket[i].data(), SIGNAL(disconnected()), mTcpSocketDisconnectedSignalMapper.data(), SLOT(map()));
      disconnect(mTcpSocket[i].data(), SIGNAL(readyRead()), mTcpSocketReadyReadSignalMapper.data(), SLOT(map()));

      connect(mTcpSocket[i].data(), SIGNAL(disconnected()), mTcpSocketDisconnectedSignalMapper.data(), SLOT(map()));
      connect(mTcpSocket[i].data(), SIGNAL(readyRead()), mTcpSocketReadyReadSignalMapper.data(), SLOT(map()));

      mTcpSocketDisconnectedSignalMapper->setMapping(mTcpSocket[i].data(), i);
      mTcpSocketReadyReadSignalMapper->setMapping(mTcpSocket[i].data(), mCn[cnCtr].data());

      cnCtr++;
    }
  }

}

/*
  _cnId sets on first free yet connection
  function returns amount of free yet connections
*/
int TcpConnector::getFreeSocket(int *_cnId)
{
  int freeCns = 0;
  bool cnIdSet = false;
  
  for (int i = 0; i < mMaxControls; i++)
    if(!(mOpenConnectionsMask & 1<<i))
    {
      freeCns++;
      if(!cnIdSet)
      {
        *_cnId = i;
        cnIdSet = true;
      }
    }

  return freeCns;
}

void TcpConnector::connection()
{
  int cnId;
  if((getFreeSocket(&cnId)) == 0)
  {
		qDebug() << "Unable to open new connection. Max amount of open connections is " << mMaxControls;
    return;
  }

  mTcpSocket[cnId].reset(mTcpServer->nextPendingConnection());
  mTcpSocket[cnId]->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
  mTcpSocket[cnId]->setSocketOption(QAbstractSocket::LowDelayOption, 1);

  int fd = mTcpSocket[cnId]->socketDescriptor();
  setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &mCnt, sizeof(mCnt));
  setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &mIdle, sizeof(mIdle));
  setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &mIntvl, sizeof(mIntvl));

  qDebug() << "Set new connection";

  mOpenConnectionsMask += 1 << cnId;
  resetMappings();
}

void TcpConnector::tcpDisconnected(int _cnId)
{

	mTcpSocket[_cnId]->abort();
  mOpenConnectionsMask -= 1<<_cnId;
  resetMappings();
}

void TcpConnector::networkRead(QObject* cn)
{
  GamepadConnection* _cn = qobject_cast<GamepadConnection*>(cn);

	if (!mTcpSocket[_cn->id()]->isValid()) {
		return;
	}

	QString line;
	while (mTcpSocket[_cn->id()]->bytesAvailable() > 0) {
		char data[100];
		mTcpSocket[_cn->id()]->readLine(data, 100);
		line += data;
	}

	emit dataReady(line, _cn->mask());
}
