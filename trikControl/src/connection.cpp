#pragma once

#include "src/connection.h"

#include <QtCore/QObject>


namespace trikControl {

  Connection::Connection()
  : id(-1)
  , mask(0)
  {}  

  Connection::Connection(int _id, int _mask)
  {
    id = _id;
    mask = _mask;
  }

  Connection::Connection(const trikControl::Connection& _cn)
  {
    id = _cn.id;
    mask = _cn.mask;
  }
}

