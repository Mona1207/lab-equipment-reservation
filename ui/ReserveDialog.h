// ============================================================
// 文件说明：ReserveDialog.h —— 预约对话框头文件
// 用户点“发起预约”时弹出的窗口：选设备、选时间、查看可用时段、确认预约
// ============================================================

#ifndef RESERVEDIALOG_H
#define RESERVEDIALOG_H

#include <QDialog>        // 对话框基类

class QComboBox;          // 前向声明：设备下拉框
class QDateTimeEdit;      // 前向声明：时间选择控件
class QPushButton;        // 前向声明：按钮
class QListWidget;        // 前向声明：可用时段列表
class QListWidgetItem;    // 前向声明：列表条目

// ============================================================
// ReserveDialog 类：预约申请对话框
// 功能：
//   1. 选设备（下拉框）
//   2. 选开始/结束时间
//   3. 查看当天可用时段
//   4. 确认提交预约
// ============================================================
class ReserveDialog : public QDialog
{
    Q_OBJECT  // 信号槽宏

public:
    explicit ReserveDialog(QWidget *parent = nullptr);

private slots:
    void onConfirm();                   // 点“确认预约”
    void onShowSlots();                 // 点“查看可用时段”
    void onSlotSelected(QListWidgetItem *item);  // 点了某个可用时段

private:
    QComboBox     *m_equipBox;          // 设备下拉框
    QDateTimeEdit *m_startEdit;         // 开始时间选择
    QDateTimeEdit *m_endEdit;           // 结束时间选择
    QPushButton   *m_slotsBtn;          // “查看可用时段”按钮
    QListWidget   *m_slotsList;         // 可用时段列表
};

#endif // RESERVEDIALOG_H
