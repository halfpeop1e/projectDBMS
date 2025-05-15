// Transaction.h
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QObject>
#include <QString>
#include "LockManager.h"

/**
 * 事务类：表示数据库中的一个事务
 */
class Transaction : public QObject
{
    Q_OBJECT

public:
    /**
     * 构造函数
     * @param id 事务ID
     * @param lockManager 锁管理器的指针
     * @param parent 父对象
     */
    explicit Transaction(int id, LockManager *lockManager, QObject *parent = nullptr);
    ~Transaction();

    /**
     * 开始事务
     */
    void begin();

    /**
     * 提交事务
     */
    bool commit();

    /**
     * 回滚事务
     */
    void rollback();

    /**
     * 读取资源
     * @param resourceId 资源ID
     * @return 是否成功读取
     */
    bool read(const QString &resourceId);

    /**
     * 写入资源
     * @param resourceId 资源ID
     * @return 是否成功写入
     */
    bool write(const QString &resourceId);

    /**
     * 获取事务ID
     * @return 事务ID
     */
    int getId() const;

private:
    int transactionId;            // 事务ID
    LockManager *lockManager;     // 锁管理器的指针
    bool active;                  // 事务是否处于活动状态
};

#endif // TRANSACTION_H
