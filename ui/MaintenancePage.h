#ifndef MAINTENANCEPAGE_H   // 头文件保护宏开始
#define MAINTENANCEPAGE_H   // 定义头文件保护宏

#include <QWidget>        // 控件基类

class QTableWidget;       // 前向声明：表格控件
class QLabel;             // 前向声明：统计标签

// =============================================================
// MaintenancePage —— 保养任务管理页（界面组 D，新增功能）
// 表格列出全部保养任务（周期型/次数型）；选中后可"执行保养"。
// 刷新时调核心 scan_due_maintenance() 自动生成到期任务。
// 表格列：任务ID / 设备 / 类型 / 计划日期 / 状态（未执行/已执行）
// =============================================================
class MaintenancePage : public QWidget   // 保养管理页
{
    Q_OBJECT                            // 信号槽元对象宏
public:
    explicit MaintenancePage(QWidget *parent = nullptr);   // 构造

public slots:
    void refreshTable();          // 重新加载全部保养任务到表格（并扫描生成新到期任务）

signals:
    void dataChanged();           // 执行保养后通知其它页面同步刷新

private slots:
    void onExecute();             // "执行选中保养"按钮

private:
    QTableWidget *m_table;        // 显示保养任务的表格
    QLabel       *m_summary;      // 顶部统计摘要（未执行/已执行数量）
};

#endif // MAINTENANCEPAGE_H  // 头文件保护结束
