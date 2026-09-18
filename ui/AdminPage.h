// ============================================================
// 文件说明：AdminPage.h —— 审批管理页头文件
// 审批管理页：管理员可以审批学生的预约申请、确认归还设备
// ============================================================

#ifndef ADMINPAGE_H
#define ADMINPAGE_H

#include <QWidget>        // 基类

class QTableWidget;       // 前向声明：表格
class QPushButton;        // 前向声明：按钮

// ============================================================
// AdminPage 类：审批管理页
// ============================================================
class AdminPage : public QWidget
{
    Q_OBJECT

public:
    explicit AdminPage(QWidget *parent = nullptr);

public slots:
    void refreshTable();  // 刷新审批表格

signals:
    // dataChanged 信号：审批操作后数据变了
    // MainWindow 连接了这个信号，收到后会刷新所有页面
    void dataChanged();

private slots:
    void onApprove();       // 点“通过”
    void onReject();        // 点“拒绝”
    void onMarkReturned(); // 点“确认归还”
    void onRefresh();       // 点“刷新”

private:
    QTableWidget *m_table;      // 预约表格（待审批/已通过）
    QPushButton  *m_approveBtn; // 通过按钮
    QPushButton  *m_rejectBtn;  // 拒绝按钮
    QPushButton  *m_returnBtn;   // 确认归还按钮
};

#endif // ADMINPAGE_H
