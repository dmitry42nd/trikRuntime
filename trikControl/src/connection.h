#pragma once

#include <QtCore/QObject>

namespace trikControl {

class Connection : public QObject
{
  Q_OBJECT

public:
  Connection();
  Connection(int _id, int _mask);
  Connection(const trikControl::Connection& _cn);

/*
  int mask() { return mask };
  int id() { return id };

  void setMask(int _mask) { mMask = _mask; }
  void setId(int _id) { mId = _id; }
*/

public:
  int id;
  int mask;
};

}

