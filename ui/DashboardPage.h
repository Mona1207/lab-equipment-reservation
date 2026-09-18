// ============================================================
// 文件说明：DashboardPage.h —— 首页仪表盘头文件
// 仪表盘就是程序刚登录进去看到的“首页”
// 上面 4 个 KPI 卡片，下面是最近预约和设备状态
// ============================================================

#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>        // 所有控件的基类

class QLabel;             // 前向声明：标签类
class QTableWidget;       // 前向声明：表格类

// ============================================================
// DashboardPage 类：首页仪表盘页面
// ============================================================
class DashboardPage : public QWidget
{
    Q_OBJECT  // 信号槽宏

public:
    explicit DashboardPage(QWidget *parent = nullptr);

public slots:
    // refresh() 是槽函数：外部调用它就能刷新页面数据
    // 比如预约成功后，主窗口会调用 dashboard->refresh() 更新数字
    void refresh();

private:
    // 4 个 KPI 数字标签
    QLabel       *m_equipCount;    // 设备总数
    QLabel       *m_todayResv;     // 今日预约数
    QLabel       *m_pendingCount;  // 待审批数
    QLabel       *m_maintCount;    // 待保养数

    QTableWidget *m_recentTable;   // 最近预约表格
    QLabel       *m_statusDist;    // 设备状态分布（文字描述）
};

#endif // DASHBOARDPAGE_H
