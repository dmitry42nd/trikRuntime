#pragma once

#include <QtCore/QObject>

namespace trikControl {

class GamepadConnection : public QObject
{
  Q_OBJECT

public:
  GamepadConnection();
  GamepadConnection(int _id, int _mask);
  GamepadConnection(const trikControl::GamepadConnection& _cn);


  int mask() { return mMask; };
  int id() { return mId; };

  void addToMask(int _mask) { mMask += _mask; };
  void setMask(int _mask) { mMask = _mask; };
  void setId(int _id) { mId = _id; };


public:
  int mId;
  int mMask;
};

}

