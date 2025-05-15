#include "LockManager.h"
#include <QDebug>

LockManager::LockManager(QObject *parent) : QObject(parent)
{
    // 初始化锁管理器
}

LockManager::~LockManager()
{
    // 清理资源
}

bool LockManager::acquireLock(int transactionId, const QString &resourceId, LockType lockType)
{
    QMutexLocker locker(&mutex);

    // 检查资源是否已经被锁定
    if (resourceLocks.contains(resourceId)) {
        QList<LockRequest> &requests = resourceLocks[resourceId];

        // 检查该事务是否已经持有该资源的锁
        for (auto &req : requests) {
            if (req.transactionId == transactionId) {
                // 如果事务已经持有该资源的锁
                if (req.lockType == lockType || (req.lockType == LockType::EXCLUSIVE)) {
                    // 已经持有相同类型的锁或更强的锁，直接返回成功
                    return true;
                } else if (req.lockType == LockType::SHARED && lockType == LockType::EXCLUSIVE) {
                    // 锁升级：从共享锁升级到排他锁
                    // 检查是否有其他事务持有共享锁
                    bool canUpgrade = true;
                    for (const auto &otherReq : requests) {
                        if (otherReq.transactionId != transactionId && otherReq.status == LockStatus::GRANTED) {
                            canUpgrade = false;
                            break;
                        }
                    }

                    if (canUpgrade) {
                        req.lockType = LockType::EXCLUSIVE;
                        return true;
                    } else {
                        // 更新等待图
                        for (const auto &otherReq : requests) {
                            if (otherReq.transactionId != transactionId && otherReq.status == LockStatus::GRANTED) {
                                updateWaitForGraph(transactionId, otherReq.transactionId);
                            }
                        }
                        return false;
                    }
                }
            }
        }

        // 检查请求的锁是否与已有的锁兼容
        for (const auto &req : requests) {
            if (req.status == LockStatus::GRANTED && !isCompatible(req.lockType, lockType)) {
                // 不兼容，更新等待图并返回失败
                updateWaitForGraph(transactionId, req.transactionId);

                // 添加等待的锁请求
                LockRequest newRequest;
                newRequest.transactionId = transactionId;
                newRequest.resourceId = resourceId;
                newRequest.lockType = lockType;
                newRequest.status = LockStatus::WAITING;
                requests.append(newRequest);

                return false;
            }
        }
    }

    // 如果资源没有被锁定或者锁兼容，授予锁
    LockRequest newRequest;
    newRequest.transactionId = transactionId;
    newRequest.resourceId = resourceId;
    newRequest.lockType = lockType;
    newRequest.status = LockStatus::GRANTED;

    if (!resourceLocks.contains(resourceId)) {
        resourceLocks[resourceId] = QList<LockRequest>();
    }
    resourceLocks[resourceId].append(newRequest);

    // 更新事务持有的资源
    if (!transactionResources.contains(transactionId)) {
        transactionResources[transactionId] = QSet<QString>();
    }
    transactionResources[transactionId].insert(resourceId);

    return true;
}

bool LockManager::releaseLock(int transactionId, const QString &resourceId)
{
    QMutexLocker locker(&mutex);

    if (!resourceLocks.contains(resourceId)) {
        return false; // 资源没有被锁定
    }

    QList<LockRequest> &requests = resourceLocks[resourceId];
    for (int i = 0; i < requests.size(); ++i) {
        if (requests[i].transactionId == transactionId && requests[i].status == LockStatus::GRANTED) {
            // 移除锁
            requests.removeAt(i);

            // 从事务持有的资源中移除
            if (transactionResources.contains(transactionId)) {
                transactionResources[transactionId].remove(resourceId);
                if (transactionResources[transactionId].isEmpty()) {
                    transactionResources.remove(transactionId);
                }
            }

            // 从等待图中移除
            removeFromWaitForGraph(transactionId);

            // 尝试授予等待的锁请求
            for (int j = 0; j < requests.size(); ++j) {
                if (requests[j].status == LockStatus::WAITING) {
                    bool canGrant = true;
                    for (int k = 0; k < requests.size(); ++k) {
                        if (k != j && requests[k].status == LockStatus::GRANTED &&
                            !isCompatible(requests[k].lockType, requests[j].lockType)) {
                            canGrant = false;
                            break;
                        }
                    }

                    if (canGrant) {
                        requests[j].status = LockStatus::GRANTED;

                        // 添加到事务持有的资源
                        int grantedTransactionId = requests[j].transactionId;
                        if (!transactionResources.contains(grantedTransactionId)) {
                            transactionResources[grantedTransactionId] = QSet<QString>();
                        }
                        transactionResources[grantedTransactionId].insert(resourceId);

                        // 更新等待图
                        removeFromWaitForGraph(grantedTransactionId);
                    }
                }
            }

            // 唤醒等待的线程
            condition.wakeAll();

            return true;
        }
    }

    return false; // 事务没有持有该资源的锁
}

void LockManager::releaseAllLocks(int transactionId)
{
    QMutexLocker locker(&mutex);

    if (!transactionResources.contains(transactionId)) {
        return; // 事务没有持有锁
    }

    // 获取事务持有的所有资源的副本
    QSet<QString> resources = transactionResources[transactionId];
    for (const QString &resourceId : resources) {
        releaseLock(transactionId, resourceId);
    }

    // 从等待图中移除
    removeFromWaitForGraph(transactionId);
}

int LockManager::detectDeadlock()
{
    QMutexLocker locker(&mutex);

    // 使用DFS检测等待图中的环
    QSet<int> visited;
    QSet<int> recursionStack;

    for (auto it = waitForGraph.begin(); it != waitForGraph.end(); ++it) {
        int transactionId = it.key();
        if (!visited.contains(transactionId)) {
            int deadlockedTransaction = hasCycle(transactionId, visited, recursionStack);
            if (deadlockedTransaction != -1) {
                return deadlockedTransaction; // 返回一个应该中止的事务ID
            }
        }
    }

    return -1; // 没有检测到死锁
}

bool LockManager::isCompatible(LockType existingType, LockType requestedType)
{
    // 共享锁之间是兼容的，其他情况不兼容
    return existingType == LockType::SHARED && requestedType == LockType::SHARED;
}

void LockManager::updateWaitForGraph(int waiter, int holder)
{
    if (!waitForGraph.contains(waiter)) {
        waitForGraph[waiter] = QSet<int>();
    }
    waitForGraph[waiter].insert(holder);
}

void LockManager::removeFromWaitForGraph(int transactionId)
{
    // 从等待图中移除该事务
    waitForGraph.remove(transactionId);

    // 移除其他事务对该事务的等待
    for (auto it = waitForGraph.begin(); it != waitForGraph.end(); ++it) {
        it.value().remove(transactionId);
    }
}

int LockManager::hasCycle(int start, QSet<int> &visited, QSet<int> &recursionStack)
{
    visited.insert(start);
    recursionStack.insert(start);

    if (waitForGraph.contains(start)) {
        for (int neighbor : waitForGraph[start]) {
            if (!visited.contains(neighbor)) {
                int result = hasCycle(neighbor, visited, recursionStack);
                if (result != -1) {
                    return result;
                }
            } else if (recursionStack.contains(neighbor)) {
                return start; // 检测到环，返回环中的一个事务ID
            }
        }
    }

    recursionStack.remove(start);
    return -1; // 没有环
}
