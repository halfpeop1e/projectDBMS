// TransactionManager.h
#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include <QObject>
#include <QMap>
#include "LockManager.h"
#include "Transaction.h"

/**
 * 事务管理器类：负责管理数据库中的所有事务
 */
class TransactionManager : public QObject
{
    Q_OBJECT

public:
    explicit TransactionManager(QObject *parent = nullptr);
    ~TransactionManager();

    /**
     * 开始一个新事务
     * @return 新事务的ID
     */
    int beginTransaction();

    /**
     * 提交事务
     * @param transactionId 事务ID
     * @return 是否成功提交
     */
    bool commitTransaction(int transactionId);

    /**
     * 回滚事务
     * @param transactionId 事务ID
     */
    void rollbackTransaction(int transactionId);

    /**
     * 获取事务
     * @param transactionId 事务ID
     * @return 事务对象的指针，如果不存在则返回nullptr
     */
    Transaction* getTransaction(int transactionId);

    /**
     * 检测并处理死锁
     * @return 是否检测到并处理了死锁
     */
    bool detectAndHandleDeadlock();

private:
    LockManager lockManager;                  // 锁管理器
    QMap<int, Transaction*> transactions;     // 事务ID到事务对象的映射
    int nextTransactionId;                    // 下一个事务ID
};

#endif // TRANSACTIONMANAGER_H
