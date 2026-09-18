// ============================================================
// 文件说明：MaintenancePage.h —— 保养管理页头文件
// 保养管理页：查看设备保养任务，执行保养（标记已完成）
// ============================================================

#ifndef MAINTENANCEPAGE_H
#define MAINTENANCEPAGE_H

#include <QWidget>        // 基类

class QTableWidget;       // 前向声明：表格
class QPushButton;        // 前向声明：按钮

// ============================================================
// MaintenancePage 类：保养管理页
// ============================================================
class MaintenancePage : public QWidget
{
    Q_OBJECT

public:
    explicit MaintenancePage(QWidget *parent = nullptr);

public slots:
    void refreshTable();  // 刷新保养任务表格

private slots:
    void onExecute();      // 点“执行保养”
    void onRefresh();      // 点“刷新”

private:
    QTableWidget *m_table;      // 保养任务表格
    QPushButton  *m_executeBtn; // 执行保养按钮
};

#endif // MAINTENANCEPAGE_H
