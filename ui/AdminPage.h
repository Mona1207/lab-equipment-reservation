#ifndef ADMINPAGE_H   // 头文件保护宏开始
#define ADMINPAGE_H   // 定义头文件保护宏

#include <QWidget>    // 控件基类

class QTableWidget;   // 前向声明：表格控件

// =============================================================
// AdminPage —— 管理员审批页（界面组 D，端到端闭环）
// 表格列出全部预约；选中记录后：
//   "审批通过" -> 核心 batch_approve（仅待审批可通过）
//   "确认归还" -> 核心 complete_reservation（仅已通过可归还，并累计使用次数）
//   "导出CSV"  -> 导出全部预约记录
// 处理后刷新表格并发出 dataChanged，联动设备页/统计页/我的预约/保养页。
// =============================================================
class AdminPage : public QWidget   // 管理员审批页
{
    Q_OBJECT                        // 信号槽元对象宏
public:
    explicit AdminPage(QWidget *parent = nullptr);   // 构造

public slots:
    void refreshTable();            // 重新加载全部预约到表格

signals:
    void dataChanged();             // 处理后通知其它页面同步刷新

private slots:
    void onApprove();               // "审批通过"按钮
    void onReturn();                // "确认归还"按钮
    void onExportCsv();             // 导出预约记录为 CSV

private:
    QTableWidget *m_table;          // 显示全部预约的表格
};

#endif // ADMINPAGE_H  // 头文件保护结束
