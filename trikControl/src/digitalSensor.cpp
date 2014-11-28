/* Copyright 2013 Yurii Litvinov
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

#include "digitalSensor.h"

#include <QtCore/QDebug>

#include <QsLog.h>

using namespace trikControl;

DigitalSensor::DigitalSensor(int min, int max, QString const &deviceFile)
	: mMin(min)
	, mMax(max)
	, mDeviceFile(deviceFile)
{
}

int DigitalSensor::read()
{
	if (!mDeviceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QLOG_ERROR() << "File " << mDeviceFile.fileName() << " failed to open for reading";
		qDebug() << "File " << mDeviceFile.fileName() << " failed to open for reading";
		return 0;
	}

	mStream.setDevice(&mDeviceFile);

	if (mMax == mMin) {
		return mMin;
	}

	int value = 0;
	mStream >> value;

	mDeviceFile.close();

	value = qMin(value, mMax);
	value = qMax(value, mMin);

	double const scale = 100.0 / (static_cast<double>(mMax - mMin));

	value = (value - mMin) * scale;

	return value;
}

int DigitalSensor::readRawData()
{
	if (!mDeviceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QLOG_ERROR() << "File " << mDeviceFile.fileName() << " failed to open for reading";
		qDebug() << "File " << mDeviceFile.fileName() << " failed to open for reading";
		return 0;
	}

	mStream.setDevice(&mDeviceFile);

	int value = 0;
	mStream >> value;

	mDeviceFile.close();

	return value;
}
