// ============================================================
// 文件说明：EquipmentPage.cpp —— 设备管理页具体实现
// 这个文件里定义了 3 个类：
//   1. EquipmentDialog：添加/编辑设备的表单对话框
//   2. EquipmentCard：单张设备卡片（网格里的一个小方块）
//   3. EquipmentPage：整个设备管理页面
// ============================================================

#include "EquipmentPage.h"
#include "AppContext.h"
#include "Theme.h"

#include "Equipment.h"

// 引入各种 Qt 控件和布局
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

// ============================================================
// 第 1 个类：EquipmentDialog —— 设备表单对话框
// 用途：添加新设备 或 编辑现有设备时弹出的窗口
// 双模式：同一个对话框，添加时编号可填，编辑时编号只读
// ============================================================
class EquipmentDialog : public QDialog
{
public:
    // 枚举：两种模式
    // enum（枚举）就是给几个固定的选项起名字
    // Add=添加新设备，Edit=编辑已有设备
    enum Mode { Add, Edit };

    // 构造函数：传模式（Add/Edit）和父窗口
    explicit EquipmentDialog(Mode mode, QWidget *parent = nullptr)
        : QDialog(parent), m_mode(mode)
    {
        setWindowTitle(mode == Add ? QStringLiteral("添加设备") : QStringLiteral("编辑设备"));
        setModal(true);  // 模态对话框：打开时不能操作后面的窗口

        // 设备编号输入框
        m_idEdit = new QLineEdit(this);
        m_idEdit->setPlaceholderText(QStringLiteral("如 EQ-0005"));

        // 编辑模式下，编号不能改（灰色显示）
        if (m_mode == Edit) {
            m_idEdit->setEnabled(false);
            m_idEdit->setStyleSheet(QStringLiteral("background: #eef1f5; color: #8a94a6;"));
        }

        // 设备名称、规格输入框
        m_nameEdit = new QLineEdit(this);
        m_specEdit = new QLineEdit(this);

        // 保养方式下拉框：按天数 / 按次数
        m_kindBox = new QComboBox(this);
        m_kindBox->addItems({ QStringLiteral("按天数周期"), QStringLiteral("按使用次数") });

        // 周期数值输入框（数字输入框）
        m_valueSpin = new QSpinBox(this);
        m_valueSpin->setRange(1, 9999);  // 范围 1~9999
        m_valueSpin->setValue(30);       // 默认值 30

        // 切换保养方式时，单位后缀跟着变（天/次）
        connect(m_kindBox, &QComboBox::currentTextChanged, this, [this](const QString& t) {
            m_valueSpin->setSuffix(t == QStringLiteral("按天数周期") ? QStringLiteral(" 天")
                                                                    : QStringLiteral(" 次"));
        });
        m_valueSpin->setSuffix(QStringLiteral(" 天"));  // 默认单位

        // 确定/取消按钮
        auto *box = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        box->button(QDialogButtonBox::Ok)->setText(mode == Add ? QStringLiteral("添加") : QStringLiteral("保存"));
        box->button(QDialogButtonBox::Ok)->setProperty("primary", true);  // 蓝色主按钮
        box->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));

        // 点确定 -> accept()（对话框关闭，返回 Accepted）
        // 点取消 -> reject()（对话框关闭，返回 Rejected）
        connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);

        // 表单布局：左边标签，右边输入框
        auto *lay = new QFormLayout(this);
        lay->addRow(QStringLiteral("设备编号："), m_idEdit);
        lay->addRow(QStringLiteral("设备名称："), m_nameEdit);
        lay->addRow(QStringLiteral("规格型号："), m_specEdit);
        lay->addRow(QStringLiteral("保养方式："), m_kindBox);
        lay->addRow(QStringLiteral("周期数值："), m_valueSpin);
        lay->addRow(box);  // 按钮放最后一行
    }

    // setEquipment()：编辑模式下，把现有设备的数据填到表单里
    void setEquipment(const Equipment* eq) {
        if (!eq) return;
        m_idEdit->setText(AppContext::toQString(eq->id()));
        m_nameEdit->setText(AppContext::toQString(eq->name()));
        m_specEdit->setText(AppContext::toQString(eq->spec()));
        m_kindBox->setCurrentIndex(eq->cycle_type() == CycleType::Days ? 0 : 1);
        m_valueSpin->setValue(eq->cycle_value());
    }

    // collect()：收集表单里的数据，校验是否合法
    // 返回 true 表示数据合法，false 表示有问题（会弹错误提示）
    bool collect(QString& id, QString& name, QString& spec,
                 CycleType& cycleType, int& cycleValue)
    {
        // trimmed() 去掉首尾空格
        id = m_idEdit->text().trimmed();
        name = m_nameEdit->text().trimmed();
        spec = m_specEdit->text().trimmed();

        // 校验：编号和名称不能为空
        if (id.isEmpty() || name.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("操作失败"),
                                 QStringLiteral("设备编号与名称为必填项。"));
            return false;
        }

        // 添加模式下，检查编号是否已存在
        if (m_mode == Add && AppContext::get().manager().find_equipment(id.toStdString())) {
            QMessageBox::warning(this, QStringLiteral("添加失败"),
                                 QStringLiteral("该设备编号已存在。"));
            return false;
        }

        // 把表单数据转成核心层的类型
        cycleType = (m_kindBox->currentText() == QStringLiteral("按天数周期"))
                        ? CycleType::Days : CycleType::Uses;
        cycleValue = m_valueSpin->value();
        return true;
    }

private:
    Mode m_mode;           // 当前模式（Add/Edit）
    QLineEdit *m_idEdit;   // 编号输入框
    QLineEdit *m_nameEdit; // 名称输入框
    QLineEdit *m_specEdit; // 规格输入框
    QComboBox *m_kindBox;  // 保养方式下拉框
    QSpinBox  *m_valueSpin; // 周期数值输入框
};

// ============================================================
// 第 2 个类：EquipmentCard —— 单张设备卡片
// 每个设备一张卡片，显示图标、名称、规格、状态、删除按钮
// 点击卡片选中，再次点击取消选中
// ============================================================
class EquipmentCard : public QFrame
{
    Q_OBJECT  // 因为有信号槽，必须加这个宏

public:
    QString equipmentId;   // 设备id
    bool    selected = false;  // 是否被选中

    // 构造函数：传一个设备数据对象
    EquipmentCard(const Equipment* eq, QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setProperty("glassCard", true);  // 卡片样式
        setFixedSize(320, 180);           // 卡片大小
        setCursor(Qt::PointingHandCursor); // 鼠标移上去变手型（提示可点击）
        equipmentId = AppContext::toQString(eq->id());

        // 设备状态文字和颜色
        const QString status = AppContext::toQString(eq->status_name());
        const QColor sc = Theme::statusColor(status);
        const QString hex = sc.name();  // 转成 #RRGGBB 格式

        // 根据设备名选一个图标块颜色（不同设备不同颜色，好看）
        QString iconColor = "#1565c0";  // 默认蓝色
        const QString nm = AppContext::toQString(eq->name());
        if (nm.contains(QStringLiteral("示波器")))      iconColor = "#1565c0";  // 蓝
        else if (nm.contains(QStringLiteral("信号")))    iconColor = "#2e7d32";  // 绿
        else if (nm.contains(QStringLiteral("万用")))   iconColor = "#f57f17";  // 黄
        else if (nm.contains(QStringLiteral("电源")))    iconColor = "#c62828";  // 红

        // 卡片内部垂直布局
        auto *mainLay = new QVBoxLayout(this);
        mainLay->setContentsMargins(18, 16, 18, 14);
        mainLay->setSpacing(8);

        // ---- 顶部一行：左边图标方块，右边状态标签 ----
        auto *topRow = new QHBoxLayout;
        topRow->setSpacing(10);

        // 图标方块（彩色背景，中间一个⚙符号）
        QLabel *iconBox = new QLabel(this);
        iconBox->setFixedSize(44, 44);
        iconBox->setAlignment(Qt::AlignCenter);
        iconBox->setText(QStringLiteral("⚙"));
        iconBox->setStyleSheet(QStringLiteral(
            "background: %1; border-radius: 4px; color: white; font-size: 20px;")
            .arg(iconColor));
        topRow->addWidget(iconBox);
        topRow->addStretch();  // 推到左边

        // 状态标签（● 可用 / ● 已借出...）
        QLabel *statusTag = new QLabel(QStringLiteral("● ") + status, this);
        statusTag->setStyleSheet(QStringLiteral(
            "color: %1; background: #f5f7fa; border: 1px solid #d0d7e0; border-radius: 3px;"
            "padding: 3px 12px; font-size: 12px; font-weight: bold;")
            .arg(hex));
        topRow->addWidget(statusTag);

        mainLay->addLayout(topRow);

        // ---- 设备名称（大粗体）----
        QLabel *nameLbl = new QLabel(nm, this);
        QFont nf = nameLbl->font();
        nf.setPointSize(13);
        nf.setBold(true);
        nameLbl->setFont(nf);
        nameLbl->setStyleSheet("color: #1f2937;");
        mainLay->addWidget(nameLbl);

        // ---- 规格型号（灰色小字）----
        QLabel *specLbl = new QLabel(AppContext::toQString(eq->spec()), this);
        specLbl->setStyleSheet("color: #6b7b8f; font-size: 12px;");
        mainLay->addWidget(specLbl);

        mainLay->addStretch();  // 中间弹簧

        // ---- 底部一行：保养周期 + 删除按钮 ----
        auto *bottomRow = new QHBoxLayout;
        bottomRow->setSpacing(8);

        // 保养周期文字
        const QString cycle = (eq->cycle_type() == CycleType::Days)
            ? QString(QStringLiteral("保养周期：%1 天")).arg(eq->cycle_value())
            : QString(QStringLiteral("保养周期：%1 次")).arg(eq->cycle_value());
        QLabel *cycleLbl = new QLabel(cycle, this);
        cycleLbl->setStyleSheet("color: #8a94a6; font-size: 11px;");
        bottomRow->addWidget(cycleLbl);
        bottomRow->addStretch();

        // 删除按钮（红色小按钮）
        QPushButton *delBtn = new QPushButton(QStringLiteral("删除"), this);
        delBtn->setFixedSize(56, 28);
        delBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: rgba(198,40,40,0.08); color: #c62828;"
            "border: 1px solid rgba(198,40,40,0.3); border-radius: 4px; font-size: 11px; padding: 0; }"
            "QPushButton:hover { background: #c62828; color: white; border-color: #c62828; }"));
        connect(delBtn, &QPushButton::clicked, this, &EquipmentCard::onDeleteClicked);
        bottomRow->addWidget(delBtn);

        mainLay->addLayout(bottomRow);
    }

    // setSelected() 设置选中状态（选中时边框变蓝）
    void setSelected(bool sel) {
        selected = sel;
        setStyleSheet(sel
            ? QStringLiteral("QFrame[glassCard=\"true\"] { background: #e3f0fd; border: 2px solid #1565c0; border-radius: 6px; }")
            : QStringLiteral("QFrame[glassCard=\"true\"] { background: #ffffff; border-radius: 6px; border: 1px solid #d0d7e0; }"));
    }

signals:
    void cardClicked(const QString& id);    // 卡片被点击
    void deleteRequested(const QString& id); // 点了删除按钮

protected:
    // mousePressEvent：鼠标按下事件（重写父类的虚函数）
    // 点卡片任意位置都算选中卡片
    void mousePressEvent(QMouseEvent *ev) override {
        emit cardClicked(equipmentId);  // 发射“卡片被点击”信号
        QFrame::mousePressEvent(ev);    // 调用父类的处理
    }

private slots:
    void onDeleteClicked() { emit deleteRequested(equipmentId); }
};

// 因为 EquipmentCard 是在 .cpp 里定义的，不是单独的 .h 文件
// 所以需要手动 include moc 文件（Qt 元对象编译器生成的）
#include "EquipmentPage.moc"

// ============================================================
// 第 3 个类：EquipmentPage —— 设备管理页
// ============================================================
EquipmentPage::EquipmentPage(QWidget *parent)
    : QWidget(parent)
{
    // ---- 顶部按钮行 ----
    QPushButton *addBtn = new QPushButton(QStringLiteral("＋ 添加设备"), this);
    addBtn->setProperty("primary", true);  // 蓝色主按钮
    QPushButton *editBtn = new QPushButton(QStringLiteral("✎ 编辑设备"), this);
    QPushButton *exportBtn = new QPushButton(QStringLiteral("📤 导出CSV"), this);
    QPushButton *refreshBtn = new QPushButton(QStringLiteral("🔄 刷新"), this);

    // 连接按钮点击信号
    connect(addBtn, &QPushButton::clicked, this, &EquipmentPage::onAddEquipment);
    connect(editBtn, &QPushButton::clicked, this, &EquipmentPage::onEditEquipment);
    connect(exportBtn, &QPushButton::clicked, this, &EquipmentPage::onExportCsv);
    connect(refreshBtn, &QPushButton::clicked, this, &EquipmentPage::onRefresh);

    // 按钮行水平布局
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(addBtn);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(exportBtn);
    btnRow->addWidget(refreshBtn);
    btnRow->addStretch();  // 推到左边

    // ---- 卡片网格（放在滚动区域里）----
    m_scroll = new QScrollArea(this);  // 滚动区域
    m_scroll->setWidgetResizable(true); // 内容自适应大小

    m_gridContainer = new QWidget;  // 网格容器
    m_gridContainer->setObjectName("cardGridContainer");
    m_grid = new QGridLayout(m_gridContainer);  // 网格布局
    m_grid->setSpacing(20);  // 卡片间距 20 像素
    m_grid->setContentsMargins(4, 4, 4, 4);
    m_scroll->setWidget(m_gridContainer);  // 把网格容器放进滚动区域

    // ---- 整体垂直布局 ----
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(16);
    lay->addLayout(btnRow);   // 顶部按钮
    lay->addWidget(m_scroll); // 卡片网格（拉伸占满）

    refreshTable();  // 初始加载设备列表
}

// ============================================================
// refreshTable() 函数：刷新设备卡片网格
// 先清空旧卡片，再从核心管理器读最新数据，重新生成卡片
// ============================================================
void EquipmentPage::refreshTable()
{
    // 第 1 步：清空旧卡片
    // takeAt(0) 每次取第 0 个，直到清空
    while (QLayoutItem *item = m_grid->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();  // 删除控件
        delete item;  // 删除布局项
    }

    // 第 2 步：从核心管理器读设备列表
    const auto& list = AppContext::get().manager().equipments();
    const int cols = 3;  // 每行 3 张卡片

    // 第 3 步：循环创建每张卡片
    for (int i = 0; i < static_cast<int>(list.size()); ++i) {
        const Equipment* e = list.at(i).get();  // 取第 i 个设备
        auto *card = new EquipmentCard(e, m_gridContainer);  // 创建卡片

        // 如果这张卡片的设备正好是当前选中的，就设置为选中状态
        const QString id = AppContext::toQString(e->id());
        if (id == m_selectedId) card->setSelected(true);

        // 点卡片：切换选中状态
        connect(card, &EquipmentCard::cardClicked, this, [this](const QString& cid) {
            // 再点一次取消选中
            m_selectedId = (m_selectedId == cid) ? QString() : cid;
            refreshTable();  // 刷新网格（重新绘制选中状态）
        });

        // 点卡片上的删除按钮
        connect(card, &EquipmentCard::deleteRequested, this, [this](const QString& cid) {
            m_selectedId = cid;  // 先设为当前选中
            onDeleteEquipment();  // 调用删除函数
        });

        // 把卡片加到网格里：第 i/cols 行，第 i%cols 列
        // i/cols 是行号（整数除法），i%cols 是列号（取余）
        m_grid->addWidget(card, i / cols, i % cols);
    }

    // 如果没有设备，显示提示文字
    if (list.empty()) {
        QLabel *empty = new QLabel(QStringLiteral("暂无设备，点击「添加设备」开始"), m_gridContainer);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color: #6b7b8f; font-size: 15px;");
        m_grid->addWidget(empty, 0, 0);
    }
}

// ============================================================
// onAddEquipment()：添加设备
// 弹出表单对话框，用户填完点确定，就加到核心管理器
// ============================================================
void EquipmentPage::onAddEquipment()
{
    EquipmentDialog dlg(EquipmentDialog::Add, this);

    // 循环：如果用户填的数据不合法，会重新弹出对话框
    while (dlg.exec() == QDialog::Accepted)
    {
        QString id, name, spec;
        CycleType cycleType;
        int cycleValue = 0;

        // collect() 校验数据，不合法就继续循环重新弹
        if (!dlg.collect(id, name, spec, cycleType, cycleValue))
            continue;

        // 数据合法，加到核心管理器
        AppContext::get().manager().add_equipment(
            id.toStdString(), name.toStdString(),
            spec.toStdString(), cycleType, cycleValue);

        m_selectedId = id;  // 新添加的设为选中
        refreshTable();    // 刷新网格
        return;            // 成功就退出函数
    }
}

// ============================================================
// onEditEquipment()：编辑选中的设备
// ============================================================
void EquipmentPage::onEditEquipment()
{
    if (m_selectedId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先点击一张设备卡片"));
        return;
    }

    // 找到选中的设备
    const Equipment* eq = AppContext::get().manager().find_equipment(m_selectedId.toStdString());
    if (!eq) {
        QMessageBox::warning(this, QStringLiteral("编辑失败"),
                             QStringLiteral("未找到该设备"));
        return;
    }

    // 弹出编辑对话框，把现有数据填进去
    EquipmentDialog dlg(EquipmentDialog::Edit, this);
    dlg.setEquipment(eq);

    while (dlg.exec() == QDialog::Accepted)
    {
        QString eid, name, spec;
        CycleType cycleType;
        int cycleValue = 0;
        if (!dlg.collect(eid, name, spec, cycleType, cycleValue))
            continue;

        // 更新设备信息
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

// ============================================================
// onDeleteEquipment()：删除选中的设备
// ============================================================
void EquipmentPage::onDeleteEquipment()
{
    if (m_selectedId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先点击一张设备卡片"));
        return;
    }

    const Equipment* eq = AppContext::get().manager().find_equipment(m_selectedId.toStdString());
    const QString name = eq ? AppContext::toQString(eq->name()) : m_selectedId;

    // 确认删除提示
    const auto answer = QMessageBox::question(this,
        QStringLiteral("删除设备"),
        QStringLiteral("确定删除设备「%1（%2）」吗？\n相关历史预约记录将保留。").arg(name, m_selectedId),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    // 从核心管理器删除
    AppContext::get().manager().remove_equipment(m_selectedId.toStdString());
    m_selectedId.clear();  // 清空选中
    refreshTable();        // 刷新网格
}

// 刷新按钮
void EquipmentPage::onRefresh()
{
    refreshTable();
}

// ============================================================
// onExportCsv()：导出设备列表为 CSV 文件
// CSV 是一种表格文件格式，Excel 可以直接打开
// ============================================================
void EquipmentPage::onExportCsv()
{
    const auto& list = AppContext::get().manager().equipments();
    if (list.empty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("当前没有设备可导出"));
        return;
    }

    // 默认文件名：设备列表_20260918_153000.csv
    const QString defaultName = QStringLiteral("设备列表_%1.csv")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));

    // 弹出保存文件对话框，让用户选保存位置
    const QString filePath = QFileDialog::getSaveFileName(this,
        QStringLiteral("导出设备列表"), defaultName,
        QStringLiteral("CSV 文件 (*.csv)"));
    if (filePath.isEmpty()) return;  // 用户取消了

    // 打开文件准备写
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("导出失败"),
                             QStringLiteral("无法打开文件进行写入"));
        return;
    }

    // 写 CSV 内容
    QTextStream out(&file);
    out << "\xEF\xBB\xBF";  // BOM 头，让 Excel 正确识别 UTF-8 中文
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
