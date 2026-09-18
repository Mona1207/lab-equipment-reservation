// ============================================================
// 文件说明：AppContext.h —— 全局上下文（“大管家”）头文件
// AppContext 是整个界面层的“总入口”
// 所有页面都通过它来访问核心数据，不用直接碰核心层
// 这叫“门面模式”：外面只跟一个简单的接口打交道，里面复杂的东西都藏起来
// ============================================================

#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QString>              // Qt 字符串类
#include <QDateTime>            // Qt 日期时间类
#include "ReservationManager.h" // 核心层的业务管理器（真正干活的）

class User;   // 前向声明用户类

// ============================================================
// AppContext 类：全局上下文（单例模式）
// 单例：整个程序里只有一个实例，全局都能访问
// 就像你家只有一个冰箱，所有人都去同一个冰箱拿东西
// ============================================================
class AppContext
{
public:
    // static 静态成员函数：不需要创建对象就能调用
    // get() 返回唯一的实例，外面用 AppContext::get() 就能拿到
    static AppContext& get();

    // seed() 灌入演示数据（演示版用，正式版从数据库读）
    void seed();

    // manager() 返回核心业务管理器的引用
    // 界面通过它来增删改查设备、预约等
    ReservationManager& manager();

    // currentUser() 返回当前登录的用户指针
    // 没登录就返回 nullptr（空指针）
    User* currentUser() const;

    // login() 登录：传用户id和密码，成功返回 true
    bool login(int userId, const QString& password);

    // logout() 退出登录：清空当前用户
    void logout();

    // toCoreTime() 把 Qt 的 QDateTime 转成核心层的 DateTime
    // 因为界面层用 Qt 的时间类，核心层用自己的时间类，需要转换
    static DateTime toCoreTime(const QDateTime& qdt);

    // toQString() 把 C++ 标准的 std::string 转成 Qt 的 QString
    // 因为界面显示统一用 QString
    static QString toQString(const std::string& s);

    // equipmentName() 设备id -> 设备名
    // 比如传 "EQ-0001" 返回 "数字示波器"
    QString equipmentName(const std::string& id);

    // nextReservationId() 生成下一个预约编号
    // 比如当前最大是 RS-0003，就返回 RS-0004
    QString nextReservationId();

private:
    // 私有构造函数：禁止外部 new AppContext()
    // 因为是单例，只能通过 get() 获取实例
    AppContext() = default;

    // 成员变量：
    ReservationManager m_manager;  // 核心业务管理器（真正存数据的地方）
    User* m_currentUser = nullptr; // 当前登录用户指针（指向 m_manager 内部的用户）
    bool m_seeded = false;         // 种子数据是否已灌入（防止重复灌）
};

#endif // APPCONTEXT_H
