// Transaction.cpp
#include "Transaction.h"
#include <QDebug>

Transaction::Transaction(int id, LockManager *lockManager, QObject *parent)
    : QObject(parent), transactionId(id), lockManager(lockManager), active(false)
{
}

Transaction::~Transaction()
{
    if (active) {
        rollback();
    }
}

void Transaction::begin()
{
    if (!active) {
        active = true;
        qDebug() << "Transaction" << transactionId << "started";
    }
}

bool Transaction::commit()
{
    if (active) {
        // 释放所有锁
        lockManager->releaseAllLocks(transactionId);
        active = false;
        qDebug() << "Transaction" << transactionId << "committed";
        return true;
    }
    return false;
}

void Transaction::rollback()
{
    if (active) {
        // 释放所有锁
        lockManager->releaseAllLocks(transactionId);
        active = false;
        qDebug() << "Transaction" << transactionId << "rolled back";
    }
}

bool Transaction::read(const QString &resourceId)
{
    if (!active) {
        qDebug() << "Transaction" << transactionId << "is not active";
        return false;
    }

    // 请求共享锁
    bool locked = lockManager->acquireLock(transactionId, resourceId, LockType::SHARED);
    if (locked) {
        qDebug() << "Transaction" << transactionId << "acquired shared lock on" << resourceId;
        // 在这里模拟读取操作
        return true;
    } else {
        qDebug() << "Transaction" << transactionId << "failed to acquire shared lock on" << resourceId;
        return false;
    }
}

bool Transaction::write(const QString &resourceId)
{
    if (!active) {
        qDebug() << "Transaction" << transactionId << "is not active";
        return false;
    }

    // 请求排他锁
    bool locked = lockManager->acquireLock(transactionId, resourceId, LockType::EXCLUSIVE);
    if (locked) {
        qDebug() << "Transaction" << transactionId << "acquired exclusive lock on" << resourceId;
        // 在这里模拟写入操作
        return true;
    } else {
        qDebug() << "Transaction" << transactionId << "failed to acquire exclusive lock on" << resourceId;
        return false;
    }
}

int Transaction::getId() const
{
    return transactionId;
}
