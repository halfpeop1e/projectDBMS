// TransactionManager.cpp
#include "TransactionManager.h"

TransactionManager::TransactionManager(QObject *parent)
    : QObject(parent), nextTransactionId(1)
{
}

TransactionManager::~TransactionManager()
{
    // 清理所有事务
    for (auto it = transactions.begin(); it != transactions.end(); ++it) {
        delete it.value();
    }
    transactions.clear();
}

int TransactionManager::beginTransaction()
{
    int transactionId = nextTransactionId++;
    Transaction *transaction = new Transaction(transactionId, &lockManager, this);
    transactions[transactionId] = transaction;
    transaction->begin();
    return transactionId;
}

bool TransactionManager::commitTransaction(int transactionId)
{
    if (transactions.contains(transactionId)) {
        Transaction *transaction = transactions[transactionId];
        bool success = transaction->commit();
        if (success) {
            transactions.remove(transactionId);
            delete transaction;
        }
        return success;
    }
    return false;
}

void TransactionManager::rollbackTransaction(int transactionId)
{
    if (transactions.contains(transactionId)) {
        Transaction *transaction = transactions[transactionId];
        transaction->rollback();
        transactions.remove(transactionId);
        delete transaction;
    }
}

Transaction* TransactionManager::getTransaction(int transactionId)
{
    return transactions.value(transactionId, nullptr);
}

bool TransactionManager::detectAndHandleDeadlock()
{
    int deadlockedTransactionId = lockManager.detectDeadlock();
    if (deadlockedTransactionId != -1) {
        // 检测到死锁，中止选定的事务
        rollbackTransaction(deadlockedTransactionId);
        return true;
    }
    return false;
}
