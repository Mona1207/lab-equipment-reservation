// ============================================================
// 文件说明：ReserveDialog.cpp —— 预约对话框具体实现
// ============================================================

#include "ReserveDialog.h"
#include "AppContext.h"

#include "Reservation.h"
#include "Equipment.h"
#include "User.h"

// 引入 Qt 控件
#include <QFormLayout>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

// ============================================================
// 构造函数：创建预约对话框
// ============================================================
ReserveDialog::ReserveDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("预约申请"));
    resize(420, 480);  // 对话框大小

    // ---- 设备下拉框 ----
    m_equipBox = new QComboBox(this);
    // 把所有设备加到下拉框里，显示格式：设备名（编号·状态）
    for (const auto& eqPtr : AppContext::get().manager().equipments())
    {
        const Equipment* e = eqPtr.get();
        m_equipBox->addItem(
            QStringLiteral("%1（%2·%3）")
                .arg(AppContext::toQString(e->name()),
                     AppContext::toQString(e->id()),
                     AppContext::toQString(e->status_name())),
            AppContext::toQString(e->id()));  // 隐藏数据是设备id
    }

    // ---- 开始/结束时间选择器 ----
    // 默认时间：今天 14:30 ~ 15:30
    const QDate today = QDate::currentDate();
    m_startEdit = new QDateTimeEdit(QDateTime(today, QTime(14, 30)), this);
    m_endEdit   = new QDateTimeEdit(QDateTime(today, QTime(15, 30)), this);

    // setCalendarPopup(true)：点日期会弹出日历选择面板
    m_startEdit->setCalendarPopup(true);
    m_endEdit->setCalendarPopup(true);

    // 显示格式：年-月-日 时:分
    m_startEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");

    // ---- “查看可用时段”按钮 ----
    m_slotsBtn = new QPushButton(QStringLiteral("🔍 查看当天可用时段"), this);
    m_slotsBtn->setCursor(Qt::PointingHandCursor);
    connect(m_slotsBtn, &QPushButton::clicked, this, &ReserveDialog::onShowSlots);

    // ---- 可用时段列表 ----
    m_slotsList = new QListWidget(this);
    m_slotsList->setAlternatingRowColors(true);  // 斑马纹
    m_slotsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_slotsList->setMaximumHeight(140);  // 最大高度
    m_slotsList->setStyleSheet(QStringLiteral(
        "QListWidget { background: #ffffff; border: 1px solid #d0d7e0; border-radius: 4px; }"
        "QListWidget::item { padding: 6px 10px; }"
        "QListWidget::item:selected { background: #e3f0fd; color: #1565c0; }"
        "QListWidget::item:hover { background: #f0f4f9; }"));

    // 点列表里的某个时段，自动填充开始/结束时间
    connect(m_slotsList, &QListWidget::itemClicked, this, &ReserveDialog::onSlotSelected);

    // 提示文字
    QLabel *slotsHint = new QLabel(QStringLiteral("💡 点击时段可自动填充开始/结束时间"), this);
    slotsHint->setStyleSheet(QStringLiteral("color: #6b7b8f; font-size: 11px;"));

    // ---- 确定/取消按钮 ----
    QDialogButtonBox *box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    box->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认预约"));
    box->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(box, &QDialogButtonBox::accepted, this, &ReserveDialog::onConfirm);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // ---- 表单布局 ----
    QFormLayout *form = new QFormLayout;
    form->addRow(QStringLiteral("选择设备："), m_equipBox);
    form->addRow(QStringLiteral("开始时间："), m_startEdit);
    form->addRow(QStringLiteral("结束时间："), m_endEdit);

    // ---- 整体垂直布局 ----
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(10);
    lay->addLayout(form);      // 表单
    lay->addWidget(m_slotsBtn); // 查看时段按钮
    lay->addWidget(slotsHint);  // 提示文字
    lay->addWidget(m_slotsList); // 时段列表
    lay->addStretch();          // 弹簧
    lay->addWidget(box);        // 确定/取消按钮
}

// ============================================================
// onShowSlots()：查看当天可用时段
// 调用核心管理器的 get_available_slots() 函数
// ============================================================
void ReserveDialog::onShowSlots()
{
    // 取选中的设备id
    const std::string equipId =
        m_equipBox->currentData().toString().toStdString();
    if (equipId.empty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择设备"));
        return;
    }

    // 取开始日期（只取年月日，时间部分归零）
    const QDate qdate = m_startEdit->dateTime().date();
    const DateTime queryDate(qdate.year(), qdate.month(), qdate.day(), 0, 0);

    // 计算预约时长（分钟）
    long long durMin = m_endEdit->dateTime().toSecsSinceEpoch() / 60
                      - m_startEdit->dateTime().toSecsSinceEpoch() / 60;
    if (durMin < 30) durMin = 30;  // 最短30分钟

    // 调用核心函数查询可用时段
    const auto availableSlots = AppContext::get().manager().get_available_slots(
        equipId, queryDate, static_cast<int>(durMin));

    // 清空列表，重新填充
    m_slotsList->clear();

    if (availableSlots.empty()) {
        m_slotsList->addItem(QStringLiteral("（当天没有足够长的可用时段）"));
        m_slotsList->item(0)->setForeground(QColor(150, 150, 150));
        m_slotsList->item(0)->setFlags(Qt::NoItemFlags);  // 不可选中
        return;
    }

    // 把每个可用时段加到列表里
    for (const auto& slot : availableSlots) {
        const DateTime& s = slot.first;   // 开始时间
        const DateTime& e = slot.second;  // 结束时间
        const QString text = QStringLiteral("🕐  %1  ~  %2   （%3 分钟）")
            .arg(AppContext::toQString(s.to_string()),
                 AppContext::toQString(e.to_string()),
                 QString::number(static_cast<int>(e - s)));

        QListWidgetItem *item = new QListWidgetItem(text);
        // 把开始和结束时间存在 item 的数据里，点击时取出来
        item->setData(Qt::UserRole, AppContext::toQString(s.to_string()));
        item->setData(Qt::UserRole + 1, AppContext::toQString(e.to_string()));
        m_slotsList->addItem(item);
    }
}

// ============================================================
// onSlotSelected()：点了列表里的某个时段，自动填充时间
// ============================================================
void ReserveDialog::onSlotSelected(QListWidgetItem *item)
{
    if (!item || item->flags() == Qt::NoItemFlags) return;

    // 从 item 的数据里取出开始和结束时间字符串
    const QString startStr = item->data(Qt::UserRole).toString();
    const QString endStr   = item->data(Qt::UserRole + 1).toString();

    // 解析成 QDateTime，填到输入框里
    m_startEdit->setDateTime(QDateTime::fromString(startStr, QStringLiteral("yyyy-MM-dd HH:mm")));
    m_endEdit->setDateTime(QDateTime::fromString(endStr, QStringLiteral("yyyy-MM-dd HH:mm")));
}

// ============================================================
// onConfirm()：确认预约
// 收集表单数据，调用核心 apply_reservation() 提交预约
// ============================================================
void ReserveDialog::onConfirm()
{
    // 取设备id
    const std::string equipId =
        m_equipBox->currentData().toString().toStdString();

    // 把 Qt 时间转成核心层时间
    const DateTime start = AppContext::toCoreTime(m_startEdit->dateTime());
    const DateTime end   = AppContext::toCoreTime(m_endEdit->dateTime());

    // 检查是否登录
    User* u = AppContext::get().currentUser();
    if (!u) {
        QMessageBox::critical(this, QStringLiteral("预约失败"),
                              QStringLiteral("登录状态已失效，请重新登录。"));
        return;
    }

    // 生成预约编号
    const std::string resId =
        AppContext::get().nextReservationId().toStdString();

    // 创建预约对象
    Reservation reservation(resId, equipId, u->getId(), start, end);

    // 调用核心函数提交预约
    const ApplyResult result =
        AppContext::get().manager().apply_reservation(reservation);

    // 预约被拒绝（时间冲突、设备维护中...）
    if (result.status == ReservationStatus::Rejected) {
        QMessageBox::critical(this, QStringLiteral("预约失败"),
                              AppContext::toQString(result.message));
        return;  // 不关闭对话框
    }

    // 预约成功（学生待审批，教师/管理员自动通过）
    QMessageBox::information(this, QStringLiteral("预约结果"),
                             AppContext::toQString(result.to_string()));
    accept();  // 关闭对话框
}
