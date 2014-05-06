/* Copyright 2013 Matvey Bryksin, Yurii Litvinov
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

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QThread>

#include "declSpec.h"

namespace trikControl {

class TcpConnector;

/// Class to support remote control of a robot using TCP client.
class TRIKCONTROL_EXPORT Gamepad : public QObject
{
	Q_OBJECT

public:
	/// Constructor.
	/// @param port - TCP port of a gamepad server.
	Gamepad(int port);

	/// Destructor declared here for QScopedPointer to be able to clean up forward-declared TcpConnector.
	virtual ~Gamepad();

signals:
	// ??!
	void padUp(int pad, int _cnMask);
	void wheel(int percent, int _cnMask);
	void pad(int pad, int x, int y, int _cnMask);
	void button(int button, int pressed, int _cnMask);
  void tcpDisconnected(int _cnId);

public slots:
  void resetConnection();


private slots:
	void parse(QString const &message, int const _cnMask);

private:
	Q_DISABLE_COPY(Gamepad)

	QScopedPointer<TcpConnector> mListener;
	QThread mNetworkThread;
  int mPort;
};

}
