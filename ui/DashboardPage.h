#ifndef DASHBOARDPAGE_H   // 头文件保护宏开始
#define DASHBOARDPAGE_H   // 定义头文件保护宏

#include <QWidget>        // 控件基类

class QLabel;             // 前向声明：标签
class QTableWidget;       // 前向声明：表格

// =============================================================
// DashboardPage —— 首页仪表盘（界面组 D，新增功能）
// 顶部 4 个 KPI 卡片：设备总数 / 今日预约 / 待审批 / 待保养
// 下方：最近预约记录表格 + 设备状态分布
// =============================================================
class DashboardPage : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardPage(QWidget *parent = nullptr);

public slots:
    void refresh();   // 刷新所有数据

private:
    QLabel       *m_equipCount;    // 设备总数
    QLabel       *m_todayResv;     // 今日预约数
    QLabel       *m_pendingCount;  // 待审批数
    QLabel       *m_maintCount;    // 待保养数
    QTableWidget *m_recentTable;   // 最近预约表格
    QLabel       *m_statusDist;    // 设备状态分布文字
};

#endif // DASHBOARDPAGE_H  // 头文件保护结束
