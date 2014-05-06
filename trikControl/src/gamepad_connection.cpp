#include "src/gamepad_connection.h"

#include <QtCore/QObject>


namespace trikControl {

  GamepadConnection::GamepadConnection()
  : mId(-1)
  , mMask(0)
  {}  

  GamepadConnection::GamepadConnection(int _id, int _mask)
  {
    mId = _id;
    mMask = _mask;
  }

  GamepadConnection::GamepadConnection(const trikControl::GamepadConnection& _cn)
  {
    mId = _cn.id();
    mMask = _cn.mask();
  }
}

