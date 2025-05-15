#ifndef LOCKMANAGER_H
#define LOCKMANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QMutex>
#include <QWaitCondition>
#include <QString>

/**
 * 锁的类型枚举
 */
enum class LockType {
    SHARED,     // 共享锁(读锁)
    EXCLUSIVE   // 排他锁(写锁)
};

/**
 * 锁请求的状态枚举
 */
enum class LockStatus {
    GRANTED,    // 已授予
    WAITING,    // 等待中
    DENIED      // 被拒绝
};

/**
 * 锁请求结构体：记录事务对某资源的锁请求信息
 */
struct LockRequest {
    int transactionId;        // 请求锁的事务ID
    QString resourceId;       // 请求锁定的资源ID
    LockType lockType;        // 请求的锁类型
    LockStatus status;        // 请求的状态
};

/**
 * 锁管理器类：负责管理数据库中所有的锁
 */
class LockManager : public QObject
{
    Q_OBJECT

public:
    explicit LockManager(QObject *parent = nullptr);
    ~LockManager();

    /**
     * 请求锁
     * @param transactionId 事务ID
     * @param resourceId 资源ID
     * @param lockType 锁类型
     * @return 是否成功获取锁
     */
    bool acquireLock(int transactionId, const QString &resourceId, LockType lockType);

    /**
     * 释放锁
     * @param transactionId 事务ID
     * @param resourceId 资源ID
     * @return 是否成功释放锁
     */
    bool releaseLock(int transactionId, const QString &resourceId);

    /**
     * 释放事务的所有锁
     * @param transactionId 事务ID
     */
    void releaseAllLocks(int transactionId);

    /**
     * 检查死锁
     * @return 如果存在死锁，返回一个应该中止的事务ID；否则返回-1
     */
    int detectDeadlock();

private:
    QMutex mutex;                 // 用于保护锁管理器数据结构的互斥锁
    QWaitCondition condition;     // 用于等待锁的条件变量

    // 资源ID到持有该资源的锁请求的映射
    QMap<QString, QList<LockRequest>> resourceLocks;

    // 事务ID到该事务持有的所有资源的映射
    QMap<int, QSet<QString>> transactionResources;

    // 事务等待图：用于死锁检测（键为等待者ID，值为被等待的事务ID集合）
    QMap<int, QSet<int>> waitForGraph;

    /**
     * 判断锁是否兼容
     * @param existingType 已存在的锁类型
     * @param requestedType 请求的锁类型
     * @return 是否兼容
     */
    bool isCompatible(LockType existingType, LockType requestedType);

    /**
     * 更新等待图
     * @param waiter 等待者事务ID
     * @param holder 持有者事务ID
     */
    void updateWaitForGraph(int waiter, int holder);

    /**
     * 从等待图中移除事务
     * @param transactionId 事务ID
     */
    void removeFromWaitForGraph(int transactionId);

    /**
     * 检测图中是否有环（死锁检测的辅助函数）
     * @param start 开始检测的节点
     * @param visited 已访问节点集合
     * @param recursionStack 递归栈中的节点集合
     * @return 如果有环，返回环中的一个事务ID；否则返回-1
     */
    int hasCycle(int start, QSet<int> &visited, QSet<int> &recursionStack);
};

#endif // LOCKMANAGER_H
