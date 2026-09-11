#include "AppContext.h"   // 对应头文件

#include "Student.h"      // 学生类（种子数据用）
#include "Teacher.h"      // 教师类
#include "Admin.h"        // 管理员类
#include "Equipment.h"    // 设备实体（查设备名用）

#include <memory>         // std::make_unique
#include <cstdio>         // snprintf 生成编号
#include <algorithm>      // 查找最大编号

// 取单例：函数内静态对象，全局唯一、首次调用时构造
AppContext& AppContext::get()
{
    static AppContext instance;   // 唯一实例
    return instance;              // 返回其引用
}

// 返回核心管理器引用，界面各页面通过它调用真实算法接口
ReservationManager& AppContext::manager()
{
    return m_manager;             // 返回成员引用
}

// 灌入演示种子数据（正式版由数据库组(C)替换为 SQLite 读取）
void AppContext::seed()
{
    if (m_seeded) return;         // 已灌过则直接返回，保证幂等
    m_seeded = true;              // 标记已灌

    // 三个演示账号：密码统一 123，id 与登录下拉框对应
    m_manager.add_user(std::make_unique<Student>(2001, "张三", "123"));    // 学生
    m_manager.add_user(std::make_unique<Teacher>(3001, "王老师", "123"));  // 教师（免审批）
    m_manager.add_user(std::make_unique<Admin>(1, "老王", "123"));         // 管理员（免审批）

    // 四台演示设备：id 为字符串；周期型按天、次数型按使用次数
    m_manager.add_equipment("EQ-0001", "数字示波器", "100MHz 四通道",
                            CycleType::Days, 30);   // 每30天周期保养
    m_manager.add_equipment("EQ-0002", "信号发生器", "双通道 25MHz",
                            CycleType::Uses, 20);   // 每使用20次保养
    m_manager.add_equipment("EQ-0003", "台式万用表", "六位半",
                            CycleType::Days, 60);   // 每60天周期保养
    m_manager.add_equipment("EQ-0004", "直流稳压电源", "0-30V/3A",
                            CycleType::Uses, 15);   // 每使用15次保养
}

// 返回当前登录用户；未登录返回空指针
User* AppContext::currentUser() const
{
    return m_currentUser;         // 直接返回成员指针
}

// 登录：用 id 找到用户并核对密码（密码是核心 User 的 std::string）
bool AppContext::login(int userId, const QString& password)
{
    User* u = m_manager.find_user(userId);                 // 核心按 id 查用户
    if (!u) return false;                                 // 用户不存在 -> 失败
    if (u->getPassword() != password.toStdString())       // 密码不一致 -> 失败
        return false;
    m_currentUser = u;                                    // 记录为当前登录用户
    return true;                                          // 登录成功
}

// 退出登录：只清空登录态，核心数据保留（可换账号再进）
void AppContext::logout()
{
    m_currentUser = nullptr;      // 置空当前用户
}

// Qt 日期时间 -> 核心 DateTime（逐字段转换，避免字符串解析误差）
DateTime AppContext::toCoreTime(const QDateTime& qdt)
{
    const QDate d = qdt.date();   // 取日期部分
    const QTime t = qdt.time();   // 取时间部分
    return DateTime(d.year(), d.month(), d.day(), t.hour(), t.minute()); // 构造核心时间
}

// std::string -> QString（界面显示统一用 QString）
QString AppContext::toQString(const std::string& s)
{
    return QString::fromStdString(s);   // Qt 标准转换
}

// 设备 id 查设备名；查不到时回退显示 id 本身，保证界面不留空
QString AppContext::equipmentName(const std::string& id)
{
    const Equipment* e = m_manager.find_equipment(id);    // 核心按字符串id查设备
    return e ? QString::fromStdString(e->name())          // 找到 -> 显示名称
             : QString::fromStdString(id);                // 找不到 -> 显示id
}

// 生成下一个预约编号：扫描已有预约，取最大 RS-nnnn 后 +1
QString AppContext::nextReservationId()
{
    int maxNo = 0;                                        // 记录当前最大序号
    for (const Reservation& r : m_manager.reservations()) // 遍历全部预约
    {
        const std::string& rid = r.id();                  // 取预约id字符串
        if (rid.rfind("RS-", 0) == 0 && rid.size() > 3)   // 仅统计 "RS-" 前缀
        {
            const int no = std::stoi(rid.substr(3));      // 截取后缀数字
            maxNo = std::max(maxNo, no);                  // 更新最大值
        }
    }
    char buf[16];                                         // 编号缓冲
    std::snprintf(buf, sizeof(buf), "RS-%04d", maxNo + 1);// 格式化为 RS-0001
    return QString::fromLatin1(buf);                      // 转 QString 返回
}
