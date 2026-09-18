// ============================================================
// 文件说明：MainWindow.h —— 主窗口头文件
// 登录成功后进入的主界面：左边侧边栏导航 + 右边内容区
// ============================================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>     // QMainWindow 是带菜单栏/工具栏/状态栏的主窗口基类

// 前向声明各种控件和页面类
class QListWidget;         // 左侧导航列表
class QStackedWidget;      // 右侧页面堆叠容器（放多个页面，切换显示哪个）
class EquipmentPage;       // 设备管理页
class DashboardPage;       // 首页仪表盘
class StatsPage;           // 统计分析页
class AdminPage;           // 审批管理页
class MaintenancePage;     // 保养管理页
class QTableWidget;        // 表格控件（我的预约页用）
class QComboBox;           // 下拉框（状态筛选用）
class QLabel;              // 标签（显示文字）
class QPushButton;         // 按钮

// ============================================================
// MainWindow 类：主窗口
// 继承自 QMainWindow，所以自带菜单栏、工具栏、状态栏
// ============================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT  // 信号槽必须的宏

public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:
    // logoutRequested 信号：用户点了“退出登录”
    // main.cpp 里连接了这个信号，会返回登录窗口
    void logoutRequested();

private slots:  // 槽函数：按钮点击时自动调用
    void openReserveDialog();      // 打开预约对话框
    void refreshMyReservations();  // 刷新“我的预约”表格
    void onCancelReservation();   // 取消选中的预约
    void onFilterChanged(const QString& text);  // 筛选条件变了
    void switchPage(int index);   // 切换页面（点侧边栏导航时）

private:
    // 左侧导航
    QListWidget     *m_nav;        // 导航列表
    QStackedWidget  *m_pages;      // 页面堆叠容器

    // 各个页面
    DashboardPage   *m_dashboard;   // 首页仪表盘
    EquipmentPage   *m_equipPage;   // 设备管理页
    QTableWidget    *m_myTable;     // 我的预约表格
    QComboBox       *m_filterCombo; // 状态筛选下拉框
    MaintenancePage *m_maintPage;   // 保养管理页
    StatsPage       *m_statsPage;   // 统计分析页
    AdminPage       *m_adminPage;   // 审批管理页

    // 侧栏底部用户信息
    QLabel          *m_userName;     // 用户名
    QLabel          *m_roleBadge;    // 角色标签
    QPushButton     *m_logoutBtn;    // 退出登录按钮

    // 顶栏
    QLabel          *m_pageTitle;    // 页面大标题
    QLabel          *m_pageSubtitle; // 页面副标题

    // 底部状态栏
    QLabel          *m_statEquip;    // 设备总数
    QLabel          *m_statToday;    // 今日预约
    QLabel          *m_statPending;  // 待审批
    QLabel          *m_statMaint;    // 待保养
    QLabel          *m_statUser;     // 当前用户

    void updateStatusBar();  // 刷新底部状态栏
};

#endif // MAINWINDOW_H
