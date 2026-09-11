#ifndef APPCONTEXT_H   // 头文件保护宏开始
#define APPCONTEXT_H   // 定义保护宏

#include <QString>              // Qt 字符串
#include <QDateTime>            // Qt 日期时间（仅界面层使用）
#include "ReservationManager.h" // 算法组真实核心：统一业务管理器

class User;   // 前向声明用户类，减少头文件耦合

// =============================================================
// AppContext —— 界面组(D)的"适配层/门面"
// 作用：界面只和这一个类打交道，由它内部持有算法组的 ReservationManager，
//      并负责：① 演示种子数据；② 当前登录用户；③ Qt类型<->核心类型转换。
// 设计原则：不重写任何算法，所有业务判断都转发给 labres_core。
// =============================================================
class AppContext   // 界面全局上下文（单例）
{
public:
    static AppContext& get();                 // 取单例实例（全局唯一）

    void seed();                              // 灌入演示用的用户/设备种子数据
    ReservationManager& manager();            // 返回核心管理器（界面由此读写数据）

    User* currentUser() const;                // 返回当前登录用户指针（未登录为 nullptr）
    bool login(int userId, const QString& password); // 按 用户id+密码 登录，成功返回 true
    void logout();                            // 退出登录（仅清空当前用户指针，不删数据）

    static DateTime toCoreTime(const QDateTime& qdt); // QDateTime -> 核心 DateTime
    static QString toQString(const std::string& s);   // std::string -> QString
    QString equipmentName(const std::string& id);       // 设备id -> 设备名（找不到回退显示id）
    QString nextReservationId();              // 生成一个不重复的预约编号 "RS-0001"

private:
    AppContext() = default;                   // 私有构造：单例，禁止外部 new
    ReservationManager m_manager;             // 持有唯一的核心业务管理器
    User* m_currentUser = nullptr;            // 当前登录用户（指向 manager 内部，稳定不释放）
    bool m_seeded = false;                    // 种子数据是否已灌入，防止重复
};

#endif // APPCONTEXT_H  // 头文件保护结束
