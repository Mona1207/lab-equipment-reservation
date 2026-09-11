#include "EquipmentPage.h"   // 对应头文件
#include "AppContext.h"      // 界面适配层（取核心管理器）
#include "Theme.h"           // 全局主题

#include "Equipment.h"       // 核心设备实体

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMouseEvent>

// =============================================================
// EquipmentDialog —— 通用设备表单对话框（添加/编辑双模式）
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
            m_idEdit->setEnabled(false);
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
        box->button(QDialogButtonBox::Ok)->setText(mode == Add ? QStringLiteral("添加") : QStringLiteral("保存"));
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

    void setEquipment(const Equipment* eq) {
        if (!eq) return;
        m_idEdit->setText(AppContext::toQString(eq->id()));
        m_nameEdit->setText(AppContext::toQString(eq->name()));
        m_specEdit->setText(AppContext::toQString(eq->spec()));
        m_kindBox->setCurrentIndex(eq->cycle_type() == CycleType::Days ? 0 : 1);
        m_valueSpin->setValue(eq->cycle_value());
    }

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

// =============================================================
// EquipmentCard —— 单张设备卡片（磨砂玻璃风格）
// 点击卡片选中，再次点击取消选中
// =============================================================
class EquipmentCard : public QFrame
{
    Q_OBJECT
public:
    QString equipmentId;
    bool    selected = false;

    EquipmentCard(const Equipment* eq, QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setProperty("glassCard", true);
        setFixedSize(320, 180);
        setCursor(Qt::PointingHandCursor);
        equipmentId = AppContext::toQString(eq->id());

        const QString status = AppContext::toQString(eq->status_name());
        const QColor sc = Theme::statusColor(status);
        const QString hex = sc.name();

        // 根据设备名选一个彩色图标块颜色
        QString iconColor = "#6c7bff";
        const QString nm = AppContext::toQString(eq->name());
        if (nm.contains(QStringLiteral("示波器")))      iconColor = "#6c7bff";
        else if (nm.contains(QStringLiteral("信号")))    iconColor = "#22b86a";
        else if (nm.contains(QStringLiteral("万用")))   iconColor = "#f5a623";
        else if (nm.contains(QStringLiteral("电源")))    iconColor = "#ef5350";

        auto *mainLay = new QVBoxLayout(this);
        mainLay->setContentsMargins(18, 16, 18, 14);
        mainLay->setSpacing(8);

        auto *topRow = new QHBoxLayout;
        topRow->setSpacing(10);

        QLabel *iconBox = new QLabel(this);
        iconBox->setFixedSize(44, 44);
        iconBox->setAlignment(Qt::AlignCenter);
        iconBox->setText(QStringLiteral("⚙"));
        iconBox->setStyleSheet(QStringLiteral(
            "background: %1; border-radius: 12px; color: white; font-size: 20px;")
            .arg(iconColor));
        topRow->addWidget(iconBox);
        topRow->addStretch();

        QLabel *statusTag = new QLabel(QStringLiteral("● ") + status, this);
        statusTag->setStyleSheet(QStringLiteral(
            "color: %1; background: rgba(255,255,255,0.6); border-radius: 9px;"
            "padding: 3px 12px; font-size: 12px; font-weight: bold;")
            .arg(hex));
        topRow->addWidget(statusTag);

        mainLay->addLayout(topRow);

        QLabel *nameLbl = new QLabel(nm, this);
        QFont nf = nameLbl->font();
        nf.setPointSize(13);
        nf.setBold(true);
        nameLbl->setFont(nf);
        nameLbl->setStyleSheet("color: #2b3445;");
        mainLay->addWidget(nameLbl);

        QLabel *specLbl = new QLabel(AppContext::toQString(eq->spec()), this);
        specLbl->setStyleSheet("color: #8a94a6; font-size: 12px;");
        mainLay->addWidget(specLbl);

        mainLay->addStretch();

        auto *bottomRow = new QHBoxLayout;
        bottomRow->setSpacing(8);

        const QString cycle = (eq->cycle_type() == CycleType::Days)
            ? QString(QStringLiteral("保养周期：%1 天")).arg(eq->cycle_value())
            : QString(QStringLiteral("保养周期：%1 次")).arg(eq->cycle_value());
        QLabel *cycleLbl = new QLabel(cycle, this);
        cycleLbl->setStyleSheet("color: #a0aabf; font-size: 11px;");
        bottomRow->addWidget(cycleLbl);
        bottomRow->addStretch();

        QPushButton *delBtn = new QPushButton(QStringLiteral("删除"), this);
        delBtn->setFixedSize(56, 28);
        delBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: rgba(239,83,80,0.1); color: #ef5350;"
            "border: 1px solid rgba(239,83,80,0.25); border-radius: 8px; font-size: 11px; padding: 0; }"
            "QPushButton:hover { background: #ef5350; color: white; }"));
        connect(delBtn, &QPushButton::clicked, this, &EquipmentCard::onDeleteClicked);
        bottomRow->addWidget(delBtn);

        mainLay->addLayout(bottomRow);
    }

    void setSelected(bool sel) {
        selected = sel;
        setStyleSheet(sel
            ? QStringLiteral("QFrame[glassCard=\"true\"] { background: rgba(238,240,255,0.95); border: 2px solid #6c7bff; border-radius: 18px; }")
            : QStringLiteral("QFrame[glassCard=\"true\"] { background: rgba(255,255,255,0.75); border-radius: 18px; border: 1px solid rgba(255,255,255,0.8); }"));
    }

signals:
    void cardClicked(const QString& id);
    void deleteRequested(const QString& id);

protected:
    void mousePressEvent(QMouseEvent *ev) override {
        emit cardClicked(equipmentId);
        QFrame::mousePressEvent(ev);
    }

private slots:
    void onDeleteClicked() { emit deleteRequested(equipmentId); }
};

#include "EquipmentPage.moc"

// =============================================================
// EquipmentPage —— 设备管理页（卡片网格）
// =============================================================
EquipmentPage::EquipmentPage(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *addBtn = new QPushButton(QStringLiteral("＋ 添加设备"), this);
    addBtn->setProperty("primary", true);
    QPushButton *editBtn = new QPushButton(QStringLiteral("✎ 编辑设备"), this);
    QPushButton *exportBtn = new QPushButton(QStringLiteral("📤 导出CSV"), this);
    QPushButton *refreshBtn = new QPushButton(QStringLiteral("🔄 刷新"), this);
    connect(addBtn, &QPushButton::clicked, this, &EquipmentPage::onAddEquipment);
    connect(editBtn, &QPushButton::clicked, this, &EquipmentPage::onEditEquipment);
    connect(exportBtn, &QPushButton::clicked, this, &EquipmentPage::onExportCsv);
    connect(refreshBtn, &QPushButton::clicked, this, &EquipmentPage::onRefresh);

    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(addBtn);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(exportBtn);
    btnRow->addWidget(refreshBtn);
    btnRow->addStretch();

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);

    m_gridContainer = new QWidget;
    m_gridContainer->setObjectName("cardGridContainer");
    m_grid = new QGridLayout(m_gridContainer);
    m_grid->setSpacing(20);
    m_grid->setContentsMargins(4, 4, 4, 4);
    m_scroll->setWidget(m_gridContainer);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(16);
    lay->addLayout(btnRow);
    lay->addWidget(m_scroll);

    refreshTable();
}

void EquipmentPage::refreshTable()
{
    while (QLayoutItem *item = m_grid->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }

    const auto& list = AppContext::get().manager().equipments();
    const int cols = 3;

    for (int i = 0; i < static_cast<int>(list.size()); ++i) {
        const Equipment* e = list.at(i).get();
        auto *card = new EquipmentCard(e, m_gridContainer);

        const QString id = AppContext::toQString(e->id());
        if (id == m_selectedId) card->setSelected(true);

        connect(card, &EquipmentCard::cardClicked, this, [this](const QString& cid) {
            m_selectedId = (m_selectedId == cid) ? QString() : cid;
            refreshTable();
        });
        connect(card, &EquipmentCard::deleteRequested, this, [this](const QString& cid) {
            m_selectedId = cid;
            onDeleteEquipment();
        });

        m_grid->addWidget(card, i / cols, i % cols);
    }

    if (list.empty()) {
        QLabel *empty = new QLabel(QStringLiteral("暂无设备，点击「添加设备」开始"), m_gridContainer);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color: #8a94a6; font-size: 15px;");
        m_grid->addWidget(empty, 0, 0);
    }
}

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

        m_selectedId = id;
        refreshTable();
        return;
    }
}

void EquipmentPage::onEditEquipment()
{
    if (m_selectedId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先点击一张设备卡片"));
        return;
    }

    const Equipment* eq = AppContext::get().manager().find_equipment(m_selectedId.toStdString());
    if (!eq) {
        QMessageBox::warning(this, QStringLiteral("编辑失败"),
                             QStringLiteral("未找到该设备"));
        return;
    }

    EquipmentDialog dlg(EquipmentDialog::Edit, this);
    dlg.setEquipment(eq);

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

void EquipmentPage::onDeleteEquipment()
{
    if (m_selectedId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先点击一张设备卡片"));
        return;
    }

    const Equipment* eq = AppContext::get().manager().find_equipment(m_selectedId.toStdString());
    const QString name = eq ? AppContext::toQString(eq->name()) : m_selectedId;

    const auto answer = QMessageBox::question(this,
        QStringLiteral("删除设备"),
        QStringLiteral("确定删除设备「%1（%2）」吗？\n相关历史预约记录将保留。").arg(name, m_selectedId),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    AppContext::get().manager().remove_equipment(m_selectedId.toStdString());
    m_selectedId.clear();
    refreshTable();
}

void EquipmentPage::onRefresh()
{
    refreshTable();
}

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
