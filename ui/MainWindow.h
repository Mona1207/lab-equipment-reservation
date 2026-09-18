#ifndef MAINWINDOW_H      // 头文件保护宏开始
#define MAINWINDOW_H      // 定义头文件保护宏

#include <QMainWindow>     // 主窗口基类

class QListWidget;         // 前向声明：左侧导航列表
class QStackedWidget;      // 前向声明：右侧页面堆叠容器
class EquipmentPage;       // 前向声明：设备管理页
class DashboardPage;       // 前向声明：首页仪表盘
class StatsPage;           // 前向声明：统计页
class AdminPage;           // 前向声明：审批页
class MaintenancePage;     // 前向声明：保养管理页
class QTableWidget;        // 前向声明：我的预约表格
class QComboBox;           // 前向声明：状态筛选下拉框
class QLabel;              // 前向声明：用户信息标签
class QPushButton;         // 前向声明：退出按钮

// =============================================================
// MainWindow —— 登录后的主窗口（界面组 D）
// 左侧深色侧栏：顶部系统名 + 导航列表 + 底部用户信息+退出登录
// 右侧：顶栏（页面标题）+ 业务页面堆叠
// 页面索引：0首页 / 1设备 / 2预约 / 3保养 / 4统计 / 5账户 / 6审批
// =============================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:
    void logoutRequested();    // 退出登录信号（main.cpp 监听后返回登录窗）

private slots:
    void openReserveDialog();
    void refreshMyReservations();
    void onCancelReservation();
    void onFilterChanged(const QString& text);
    void switchPage(int index);

private:
    QListWidget     *m_nav;
    QStackedWidget  *m_pages;
    DashboardPage   *m_dashboard;
    EquipmentPage   *m_equipPage;
    QTableWidget    *m_myTable;
    QComboBox       *m_filterCombo;
    MaintenancePage *m_maintPage;
    StatsPage       *m_statsPage;
    AdminPage       *m_adminPage;

    QLabel          *m_userName;     // 底部用户名
    QLabel          *m_roleBadge;    // 角色标签
    QPushButton     *m_logoutBtn;    // 退出登录按钮
    QLabel          *m_pageTitle;    // 顶栏页面标题
    QLabel          *m_pageSubtitle; // 顶栏页面副标题

    // 底部状态栏标签
    QLabel          *m_statEquip;    // 设备总数
    QLabel          *m_statToday;    // 今日预约
    QLabel          *m_statPending;  // 待审批
    QLabel          *m_statMaint;    // 待保养
    QLabel          *m_statUser;     // 当前用户

    void updateStatusBar();          // 刷新底部状态栏
};

#endif // MAINWINDOW_H
