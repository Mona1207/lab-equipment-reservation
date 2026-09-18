#ifndef EQUIPMENTPAGE_H   // 头文件保护宏开始
#define EQUIPMENTPAGE_H   // 定义头文件保护宏

#include <QWidget>        // 控件基类

class QScrollArea;        // 前向声明：滚动区域
class QGridLayout;        // 前向声明：网格布局

// =============================================================
// EquipmentPage —— 设备管理页（卡片网格布局）
// 主区域为圆角卡片网格，每张卡片：彩色图标 + 设备名 + 规格 + 状态标签 + 删除按钮
// 顶部按钮：添加设备 / 编辑设备 / 导出CSV / 刷新
// =============================================================
class EquipmentPage : public QWidget
{
    Q_OBJECT
public:
    explicit EquipmentPage(QWidget *parent = nullptr);

public slots:
    void refreshTable();          // 重刷卡片网格

private slots:
    void onAddEquipment();
    void onEditEquipment();
    void onDeleteEquipment();
    void onExportCsv();
    void onRefresh();

private:
    QScrollArea *m_scroll;        // 卡片滚动区域
    QWidget     *m_gridContainer; // 网格容器
    QGridLayout *m_grid;          // 卡片网格布局
    QString      m_selectedId;    // 当前选中的设备编号
};

#endif // EQUIPMENTPAGE_H
