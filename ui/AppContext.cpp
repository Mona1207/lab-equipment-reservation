// ============================================================
// 文件说明：AppContext.cpp —— 全局上下文具体实现
// ============================================================

#include "AppContext.h"

// 引入核心层的用户类和设备类
#include "Student.h"      // 学生类
#include "Teacher.h"      // 教师类
#include "Admin.h"        // 管理员类
#include "Equipment.h"    // 设备类

#include <memory>         // std::make_unique（智能指针）
#include <cstdio>         // snprintf（格式化字符串）
#include <algorithm>      // std::max（取最大值）

// ============================================================
// get() 函数：获取单例实例
// 函数内静态变量：第一次调用时创建，之后每次调用都返回同一个
// 这是 C++ 单例模式的标准写法（Meyers' Singleton）
// ============================================================
AppContext& AppContext::get()
{
    static AppContext instance;   // 唯一实例，程序结束时自动销毁
    return instance;              // 返回它的引用
}

// ============================================================
// manager() 函数：返回核心业务管理器引用
// 界面各页面通过它来操作数据
// ============================================================
ReservationManager& AppContext::manager()
{
    return m_manager;
}

// ============================================================
// seed() 函数：灌入演示种子数据
// 正式版这里会从数据库读取，演示版我们手动造几个用户和设备
// ============================================================
void AppContext::seed()
{
    // 如果已经灌过了，直接返回（防止重复）
    if (m_seeded) return;
    m_seeded = true;  // 标记为已灌

    // add_user() 往核心管理器里加用户
    // std::make_unique 创建智能指针（自动管理内存，不用手动 delete）
    // 三个演示账号，密码都是 123
    m_manager.add_user(std::make_unique<Student>(2001, "张三", "123"));    // 学生
    m_manager.add_user(std::make_unique<Teacher>(3001, "王老师", "123"));  // 教师
    m_manager.add_user(std::make_unique<Admin>(1, "老王", "123"));         // 管理员

    // add_equipment() 往核心管理器里加设备
    // 参数：设备id、设备名、规格、保养类型、周期值
    // CycleType::Days 是按天数保养，CycleType::Uses 是按使用次数保养
    m_manager.add_equipment("EQ-0001", "数字示波器", "100MHz 四通道",
                            CycleType::Days, 30);   // 每30天保养一次
    m_manager.add_equipment("EQ-0002", "信号发生器", "双通道 25MHz",
                            CycleType::Uses, 20);   // 每用20次保养一次
    m_manager.add_equipment("EQ-0003", "台式万用表", "六位半",
                            CycleType::Days, 60);   // 每60天保养一次
    m_manager.add_equipment("EQ-0004", "直流稳压电源", "0-30V/3A",
                            CycleType::Uses, 15);   // 每用15次保养一次
}

// ============================================================
// currentUser() 函数：返回当前登录用户指针
// ============================================================
User* AppContext::currentUser() const
{
    return m_currentUser;
}

// ============================================================
// login() 函数：登录校验
// 参数：用户id、密码
// 返回：true 成功，false 失败
// ============================================================
bool AppContext::login(int userId, const QString& password)
{
    // find_user() 在核心管理器里按 id 找用户
    User* u = m_manager.find_user(userId);
    if (!u) return false;  // 没找到用户，失败

    // getPassword() 拿用户的密码（std::string）
    // password.toStdString() 把 QString 转成 std::string
    // 两个密码不一样，就登录失败
    if (u->getPassword() != password.toStdString())
        return false;

    // 登录成功：记录当前用户
    m_currentUser = u;
    return true;
}

// ============================================================
// logout() 函数：退出登录
// 只是清空当前用户指针，数据还在（可以换账号再进）
// ============================================================
void AppContext::logout()
{
    m_currentUser = nullptr;  // 置空
}

// ============================================================
// toCoreTime() 函数：Qt 时间 -> 核心层时间
// 逐字段转换（年、月、日、时、分），不用字符串解析，更准确
// ============================================================
DateTime AppContext::toCoreTime(const QDateTime& qdt)
{
    const QDate d = qdt.date();   // 取日期部分
    const QTime t = qdt.time();   // 取时间部分
    // 构造核心层的 DateTime 对象
    return DateTime(d.year(), d.month(), d.day(), t.hour(), t.minute());
}

// ============================================================
// toQString() 函数：std::string -> QString
// 界面显示统一用 QString，所以需要转换
// ============================================================
QString AppContext::toQString(const std::string& s)
{
    return QString::fromStdString(s);
}

// ============================================================
// equipmentName() 函数：设备id -> 设备名
// 比如传 "EQ-0001" 返回 "数字示波器"
// 找不到就返回 id 本身（保证界面不留空）
// ============================================================
QString AppContext::equipmentName(const std::string& id)
{
    const Equipment* e = m_manager.find_equipment(id);
    // 三元运算符：e 不为空就返回设备名，否则返回 id
    return e ? QString::fromStdString(e->name())
             : QString::fromStdString(id);
}

// ============================================================
// nextReservationId() 函数：生成下一个预约编号
// 格式是 RS-0001、RS-0002、RS-0003...
// 做法：遍历所有预约，找到最大的编号，加 1
// ============================================================
QString AppContext::nextReservationId()
{
    int maxNo = 0;  // 记录当前最大序号

    // reservations() 返回所有预约的列表
    // for 循环遍历每一条预约
    for (const Reservation& r : m_manager.reservations())
    {
        const std::string& rid = r.id();  // 取预约id字符串

        // rfind("RS-", 0) == 0 表示字符串以 "RS-" 开头
        // size() > 3 表示长度大于3（至少有 RS-xxx）
        if (rid.rfind("RS-", 0) == 0 && rid.size() > 3)
        {
            // substr(3) 取第3个字符之后的子串（就是数字部分）
            // stoi() 把字符串转成整数
            const int no = std::stoi(rid.substr(3));

            // 更新最大值
            maxNo = std::max(maxNo, no);
        }
    }

    // snprintf 格式化字符串：RS-%04d 表示 RS- 后面跟4位数字，不够补0
    // 比如 maxNo+1=5，就变成 RS-0005
    char buf[16];  // 字符数组，存结果
    std::snprintf(buf, sizeof(buf), "RS-%04d", maxNo + 1);

    // fromLatin1 把 char* 转成 QString
    return QString::fromLatin1(buf);
}
