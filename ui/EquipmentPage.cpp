#include "EquipmentPage.h"   // 对应头文件
#include "AppContext.h"      // 界面适配层（取核心管理器）
#include "Theme.h"           // 全局主题（状态彩色单元格）

#include "Equipment.h"       // 核心设备实体（枚举/取值方法）

#include <QVBoxLayout>       // 垂直布局
#include <QHBoxLayout>       // 水平布局
#include <QTableWidget>      // 表格控件
#include <QHeaderView>       // 表头
#include <QPushButton>       // 按钮
#include <QDialog>           // 自定义添加设备对话框基类
#include <QDialogButtonBox>  // 标准 确定/取消 按钮盒
#include <QFormLayout>       // 添加设备表单布局
#include <QLineEdit>         // 输入框
#include <QComboBox>         // 保养方式下拉
#include <QSpinBox>          // 周期数值输入
#include <QMessageBox>       // 提示框
#include <QFileDialog>       // 文件保存对话框
#include <QFile>             // 文件操作
#include <QTextStream>       // 文本流（写CSV）
#include <QDateTime>         // 导出文件名用时间戳

// =============================================================
// EquipmentDialog —— 通用设备表单对话框（添加/编辑双模式）
// 编辑模式下编号不可改（设备编号是主键），其余字段预填现有值。
// =============================================================
class EquipmentDialog : public QDialog
{
public:
    enum Mode { Add, Edit };

    explicit EquipmentDialog(Mode mode, QWidget *parent = nullptr)
        : QDialog(parent), m_mode(mode)
    {
        setWindowTitle(mode == Add ? QStringLiteral("添加设备") : QStringLiteral("编辑设备"));
        setModal(true);

        m_idEdit = new QLineEdit(this);
        m_idEdit->setPlaceholderText(QStringLiteral("如 EQ-0005"));
        if (m_mode == Edit) {
            m_idEdit->setEnabled(false);   // 编辑模式下编号不可改
            m_idEdit->setStyleSheet(QStringLiteral("background: #f0f0f0; color: #888;"));
        }

        m_nameEdit = new QLineEdit(this);
        m_specEdit = new QLineEdit(this);

        m_kindBox = new QComboBox(this);
        m_kindBox->addItems({ QStringLiteral("按天数周期"), QStringLiteral("按使用次数") });

        m_valueSpin = new QSpinBox(this);
        m_valueSpin->setRange(1, 9999);
        m_valueSpin->setValue(30);

        connect(m_kindBox, &QComboBox::currentTextChanged, this, [this](const QString& t) {
            m_valueSpin->setSuffix(t == QStringLiteral("按天数周期") ? QStringLiteral(" 天")
                                                                    : QStringLiteral(" 次"));
        });
        m_valueSpin->setSuffix(QStringLiteral(" 天"));

        auto *box = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        box->button(QDialogButtonBox::Ok)->setText(m_mode == Add ? QStringLiteral("添加") : QStringLiteral("保存"));
        box->button(QDialogButtonBox::Ok)->setProperty("primary", true);
        box->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);

        auto *lay = new QFormLayout(this);
        lay->addRow(QStringLiteral("设备编号："), m_idEdit);
        lay->addRow(QStringLiteral("设备名称："), m_nameEdit);
        lay->addRow(QStringLiteral("规格型号："), m_specEdit);
        lay->addRow(QStringLiteral("保养方式："), m_kindBox);
        lay->addRow(QStringLiteral("周期数值："), m_valueSpin);
        lay->addRow(box);
    }

    // 编辑模式：预填现有设备数据
    void setEquipment(const Equipment* eq) {
        if (!eq) return;
        m_idEdit->setText(AppContext::toQString(eq->id()));
        m_nameEdit->setText(AppContext::toQString(eq->name()));
        m_specEdit->setText(AppContext::toQString(eq->spec()));
        m_kindBox->setCurrentIndex(eq->cycle_type() == CycleType::Days ? 0 : 1);
        m_valueSpin->setValue(eq->cycle_value());
    }

    // 取表单结果
    bool collect(QString& id, QString& name, QString& spec,
                 CycleType& cycleType, int& cycleValue)
    {
        id = m_idEdit->text().trimmed();
        name = m_nameEdit->text().trimmed();
        spec = m_specEdit->text().trimmed();

        if (id.isEmpty() || name.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("操作失败"),
                                 QStringLiteral("设备编号与名称为必填项。"));
            return false;
        }
        // 添加模式：检查编号重复；编辑模式：编号不可改，无需检查
        if (m_mode == Add && AppContext::get().manager().find_equipment(id.toStdString())) {
            QMessageBox::warning(this, QStringLiteral("添加失败"),
                                 QStringLiteral("该设备编号已存在。"));
            return false;
        }
        cycleType = (m_kindBox->currentText() == QStringLiteral("按天数周期"))
                        ? CycleType::Days : CycleType::Uses;
        cycleValue = m_valueSpin->value();
        return true;
    }

private:
    Mode m_mode;
    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QLineEdit *m_specEdit;
    QComboBox *m_kindBox;
    QSpinBox  *m_valueSpin;
};

// 构建设备管理页
EquipmentPage::EquipmentPage(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *addBtn = new QPushButton(QStringLiteral("＋ 添加设备"), this);
    addBtn->setProperty("primary", true);
    QPushButton *editBtn = new QPushButton(QStringLiteral("✎ 编辑设备"), this);
    QPushButton *delBtn = new QPushButton(QStringLiteral("删除选中"), this);
    QPushButton *exportBtn = new QPushButton(QStringLiteral("📤 导出CSV"), this);
    connect(addBtn, &QPushButton::clicked, this, &EquipmentPage::onAddEquipment);
    connect(editBtn, &QPushButton::clicked, this, &EquipmentPage::onEditEquipment);
    connect(delBtn, &QPushButton::clicked, this, &EquipmentPage::onDeleteEquipment);
    connect(exportBtn, &QPushButton::clicked, this, &EquipmentPage::onExportCsv);

    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(addBtn);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(delBtn);
    btnRow->addWidget(exportBtn);
    btnRow->addStretch();

    m_table = new QTableWidget(0, 3, this);
    m_table->setHorizontalHeaderLabels(
        { QStringLiteral("设备ID"), QStringLiteral("设备名称"), QStringLiteral("状态") });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(16);
    lay->addLayout(btnRow);
    lay->addWidget(m_table);

    refreshTable();
}

// 遍历核心 equipments() 容器，逐行刷新
void EquipmentPage::refreshTable()
{
    const auto& list = AppContext::get().manager().equipments();
    m_table->setRowCount(static_cast<int>(list.size()));

    for (int row = 0; row < static_cast<int>(list.size()); ++row)
    {
        const Equipment* e = list.at(row).get();
        m_table->setItem(row, 0, new QTableWidgetItem(
            AppContext::toQString(e->id())));
        m_table->setItem(row, 1, new QTableWidgetItem(
            AppContext::toQString(e->name())));
        m_table->setItem(row, 2, Theme::makeStatusItem(
            AppContext::toQString(e->status_name())));
    }
}

// 添加设备
void EquipmentPage::onAddEquipment()
{
    EquipmentDialog dlg(EquipmentDialog::Add, this);
    while (dlg.exec() == QDialog::Accepted)
    {
        QString id, name, spec;
        CycleType cycleType;
        int cycleValue = 0;
        if (!dlg.collect(id, name, spec, cycleType, cycleValue))
            continue;

        AppContext::get().manager().add_equipment(
            id.toStdString(), name.toStdString(),
            spec.toStdString(), cycleType, cycleValue);

        refreshTable();
        return;
    }
}

// 编辑设备
void EquipmentPage::onEditEquipment()
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一行设备"));
        return;
    }

    const QString id = m_table->item(row, 0)->text();
    const Equipment* eq = AppContext::get().manager().find_equipment(id.toStdString());
    if (!eq) {
        QMessageBox::warning(this, QStringLiteral("编辑失败"),
                             QStringLiteral("未找到该设备"));
        return;
    }

    EquipmentDialog dlg(EquipmentDialog::Edit, this);
    dlg.setEquipment(eq);   // 预填现有数据

    while (dlg.exec() == QDialog::Accepted)
    {
        QString eid, name, spec;
        CycleType cycleType;
        int cycleValue = 0;
        if (!dlg.collect(eid, name, spec, cycleType, cycleValue))
            continue;

        const bool ok = AppContext::get().manager().update_equipment(
            eid.toStdString(), name.toStdString(), spec.toStdString(),
            cycleType, cycleValue);
        if (!ok) {
            QMessageBox::warning(this, QStringLiteral("编辑失败"),
                                 QStringLiteral("更新设备信息失败"));
            return;
        }

        QMessageBox::information(this, QStringLiteral("编辑成功"),
                                 QStringLiteral("设备信息已更新。"));
        refreshTable();
        return;
    }
}

// 删除选中设备
void EquipmentPage::onDeleteEquipment()
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一行设备"));
        return;
    }
    const QString id = m_table->item(row, 0)->text();
    const QString name = m_table->item(row, 1)->text();
    const auto answer = QMessageBox::question(this,
        QStringLiteral("删除设备"),
        QStringLiteral("确定删除设备「%1（%2）」吗？\n相关历史预约记录将保留。").arg(name, id),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    AppContext::get().manager().remove_equipment(id.toStdString());
    refreshTable();
}

// 导出设备列表为 CSV 文件
void EquipmentPage::onExportCsv()
{
    const auto& list = AppContext::get().manager().equipments();
    if (list.empty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("当前没有设备可导出"));
        return;
    }

    const QString defaultName = QStringLiteral("设备列表_%1.csv")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString filePath = QFileDialog::getSaveFileName(this,
        QStringLiteral("导出设备列表"), defaultName,
        QStringLiteral("CSV 文件 (*.csv)"));
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("导出失败"),
                             QStringLiteral("无法打开文件进行写入"));
        return;
    }

    QTextStream out(&file);
    // 写 BOM 让 Excel 正确识别 UTF-8 中文
    out << "\xEF\xBB\xBF";
    out << "设备ID,设备名称,规格型号,状态,保养方式,周期值,已使用次数\n";

    for (const auto& eqPtr : list) {
        const Equipment* e = eqPtr.get();
        const QString cycleType = (e->cycle_type() == CycleType::Days)
                                      ? QStringLiteral("按天数") : QStringLiteral("按次数");
        out << AppContext::toQString(e->id()) << ","
            << AppContext::toQString(e->name()) << ","
            << AppContext::toQString(e->spec()) << ","
            << AppContext::toQString(e->status_name()) << ","
            << cycleType << ","
            << e->cycle_value() << ","
            << e->usage_count() << "\n";
    }

    file.close();
    QMessageBox::information(this, QStringLiteral("导出成功"),
                             QStringLiteral("已导出 %1 台设备到：\n%2").arg(list.size()).arg(filePath));
}
