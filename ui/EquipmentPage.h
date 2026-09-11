#ifndef EQUIPMENTPAGE_H   // 头文件保护宏开始
#define EQUIPMENTPAGE_H   // 定义头文件保护宏

#include <QWidget>        // 控件基类

class QTableWidget;       // 前向声明：表格控件

// =============================================================
// EquipmentPage —— 设备管理页（界面组 D）
// QTableWidget 绑定核心 ReservationManager::equipments()：
//   refreshTable() 遍历设备，逐行显示 字符串id / 名称 / 状态(可用/已借出/维护中)；
//   "添加设备"/"编辑设备"弹表单对话框，调核心 add_equipment/update_equipment；
//   "删除选中"调 remove_equipment。
// =============================================================
class EquipmentPage : public QWidget   // 设备管理页
{
    Q_OBJECT                            // 信号槽元对象宏
public:
    explicit EquipmentPage(QWidget *parent = nullptr);   // 构造

public slots:
    void refreshTable();          // 任何数据变动后都调它重刷表格

private slots:
    void onAddEquipment();        // 添加设备按钮
    void onEditEquipment();       // 编辑设备按钮
    void onDeleteEquipment();     // 删除选中设备
    void onExportCsv();           // 导出设备列表为 CSV

private:
    QTableWidget *m_table;        // 显示设备列表的表格
};

#endif // EQUIPMENTPAGE_H  // 头文件保护结束
