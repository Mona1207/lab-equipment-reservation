// ============================================================
// 文件说明：EquipmentPage.h —— 设备管理页头文件
// 设备管理页：用卡片网格展示所有设备，可以添加/编辑/删除设备
// ============================================================

#ifndef EQUIPMENTPAGE_H
#define EQUIPMENTPAGE_H

#include <QWidget>        // 基类

class QScrollArea;        // 前向声明：滚动区域（内容多时可以上下滚动）
class QGridLayout;        // 前向声明：网格布局（像表格一样排列卡片）

// ============================================================
// EquipmentPage 类：设备管理页
// ============================================================
class EquipmentPage : public QWidget
{
    Q_OBJECT

public:
    explicit EquipmentPage(QWidget *parent = nullptr);

public slots:
    // refreshTable() 刷新设备卡片网格
    // 外部数据变化时（比如审批通过了），调用这个刷新界面
    void refreshTable();

private slots:
    // 各种按钮点击的槽函数
    void onAddEquipment();      // 点“添加设备”
    void onEditEquipment();     // 点“编辑设备”
    void onDeleteEquipment();   // 点“删除设备”
    void onExportCsv();        // 点“导出CSV”
    void onRefresh();          // 点“刷新”

private:
    QScrollArea *m_scroll;        // 滚动区域
    QWidget     *m_gridContainer; // 网格容器（放卡片的地方）
    QGridLayout *m_grid;          // 网格布局（3列）
    QString      m_selectedId;    // 当前选中的设备id（没选中就是空字符串）
};

#endif // EQUIPMENTPAGE_H
