#ifndef RESERVEDIALOG_H   // 头文件保护宏开始
#define RESERVEDIALOG_H   // 定义头文件保护宏

#include <QDialog>        // 对话框基类

class QComboBox;          // 前向声明：设备下拉框
class QDateTimeEdit;      // 前向声明：时间选择控件
class QPushButton;        // 前向声明：按钮
class QListWidget;        // 前向声明：可用时段列表
class QListWidgetItem;    // 前向声明：列表条目

// =============================================================
// ReserveDialog —— 发起预约对话框（界面组 D，核心生死线界面）
// 设备下拉框 + 开始/结束时间选择 + 可用时段查询；确认时构造核心 Reservation 并调用
// ReservationManager::apply_reservation()，由核心统一完成：
//   学生->待审批 / 教师·管理员->自动通过 / 时间冲突抢占或拒绝。
// 核心返回 Rejected 时弹红色 critical 错误框且不关闭对话框。
// =============================================================
class ReserveDialog : public QDialog   // 预约对话框
{
    Q_OBJECT                            // 信号槽元对象宏
public:
    explicit ReserveDialog(QWidget *parent = nullptr);   // 构造

private slots:
    void onConfirm();                   // "确认预约"槽：校验并提交核心
    void onShowSlots();                 // "查看可用时段"槽：调核心扫描当天空闲窗口
    void onSlotSelected(QListWidgetItem *item);  // 点击可用时段条目→自动填充时间

private:
    QComboBox     *m_equipBox;          // 选择设备的下拉框
    QDateTimeEdit *m_startEdit;         // 开始时间选择
    QDateTimeEdit *m_endEdit;           // 结束时间选择
    QPushButton   *m_slotsBtn;          // "查看可用时段"按钮
    QListWidget   *m_slotsList;         // 可用时段列表（点击可填充）
};

#endif // RESERVEDIALOG_H  // 头文件保护结束
