#include "ReserveDialog.h"   // 对应头文件
#include "AppContext.h"      // 界面适配层

#include "Reservation.h"     // 核心预约实体
#include "Equipment.h"       // 核心设备实体
#include "User.h"            // 核心用户（取当前登录id）

#include <QFormLayout>       // 表单布局
#include <QComboBox>         // 下拉框
#include <QDateTimeEdit>     // 日期时间选择控件
#include <QPushButton>       // 按钮
#include <QDialogButtonBox>  // 标准 确定/取消 按钮盒
#include <QMessageBox>       // 弹窗
#include <QDate>             // 日期
#include <QTime>             // 时间
#include <QListWidget>       // 可用时段列表
#include <QListWidgetItem>   // 列表条目
#include <QLabel>            // 标签
#include <QVBoxLayout>       // 垂直布局
#include <QHBoxLayout>       // 水平布局

ReserveDialog::ReserveDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("预约申请"));
    resize(420, 480);

    m_equipBox = new QComboBox(this);
    for (const auto& eqPtr : AppContext::get().manager().equipments())
    {
        const Equipment* e = eqPtr.get();
        m_equipBox->addItem(
            QStringLiteral("%1（%2·%3）")
                .arg(AppContext::toQString(e->name()),
                     AppContext::toQString(e->id()),
                     AppContext::toQString(e->status_name())),
            AppContext::toQString(e->id()));
    }

    const QDate today = QDate::currentDate();
    m_startEdit = new QDateTimeEdit(QDateTime(today, QTime(14, 30)), this);
    m_endEdit   = new QDateTimeEdit(QDateTime(today, QTime(15, 30)), this);
    m_startEdit->setCalendarPopup(true);
    m_endEdit->setCalendarPopup(true);
    m_startEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");

    m_slotsBtn = new QPushButton(QStringLiteral("🔍 查看当天可用时段"), this);
    m_slotsBtn->setCursor(Qt::PointingHandCursor);
    connect(m_slotsBtn, &QPushButton::clicked, this, &ReserveDialog::onShowSlots);

    m_slotsList = new QListWidget(this);
    m_slotsList->setAlternatingRowColors(true);
    m_slotsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_slotsList->setMaximumHeight(140);
    m_slotsList->setStyleSheet(QStringLiteral(
        "QListWidget { background: #ffffff; border: 1px solid #d0d7e0; border-radius: 4px; }"
        "QListWidget::item { padding: 6px 10px; }"
        "QListWidget::item:selected { background: #e3f0fd; color: #1565c0; }"
        "QListWidget::item:hover { background: #f0f4f9; }"));
    connect(m_slotsList, &QListWidget::itemClicked, this, &ReserveDialog::onSlotSelected);

    QLabel *slotsHint = new QLabel(QStringLiteral("💡 点击时段可自动填充开始/结束时间"), this);
    slotsHint->setStyleSheet(QStringLiteral("color: #6b7b8f; font-size: 11px;"));

    QDialogButtonBox *box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    box->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认预约"));
    box->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(box, &QDialogButtonBox::accepted, this, &ReserveDialog::onConfirm);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QFormLayout *form = new QFormLayout;
    form->addRow(QStringLiteral("选择设备："), m_equipBox);
    form->addRow(QStringLiteral("开始时间："), m_startEdit);
    form->addRow(QStringLiteral("结束时间："), m_endEdit);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(10);
    lay->addLayout(form);
    lay->addWidget(m_slotsBtn);
    lay->addWidget(slotsHint);
    lay->addWidget(m_slotsList);
    lay->addStretch();
    lay->addWidget(box);
}

void ReserveDialog::onShowSlots()
{
    const std::string equipId =
        m_equipBox->currentData().toString().toStdString();
    if (equipId.empty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择设备"));
        return;
    }

    const QDate qdate = m_startEdit->dateTime().date();
    const DateTime queryDate(qdate.year(), qdate.month(), qdate.day(), 0, 0);

    long long durMin = m_endEdit->dateTime().toSecsSinceEpoch() / 60
                      - m_startEdit->dateTime().toSecsSinceEpoch() / 60;
    if (durMin < 30) durMin = 30;

    const auto availableSlots = AppContext::get().manager().get_available_slots(
        equipId, queryDate, static_cast<int>(durMin));

    m_slotsList->clear();
    if (availableSlots.empty()) {
        m_slotsList->addItem(QStringLiteral("（当天没有足够长的可用时段）"));
        m_slotsList->item(0)->setForeground(QColor(150, 150, 150));
        m_slotsList->item(0)->setFlags(Qt::NoItemFlags);
        return;
    }

    for (const auto& slot : availableSlots) {
        const DateTime& s = slot.first;
        const DateTime& e = slot.second;
        const QString text = QStringLiteral("🕐  %1  ~  %2   （%3 分钟）")
            .arg(AppContext::toQString(s.to_string()),
                 AppContext::toQString(e.to_string()),
                 QString::number(static_cast<int>(e - s)));
        QListWidgetItem *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, AppContext::toQString(s.to_string()));
        item->setData(Qt::UserRole + 1, AppContext::toQString(e.to_string()));
        m_slotsList->addItem(item);
    }
}

void ReserveDialog::onSlotSelected(QListWidgetItem *item)
{
    if (!item || item->flags() == Qt::NoItemFlags) return;
    const QString startStr = item->data(Qt::UserRole).toString();
    const QString endStr   = item->data(Qt::UserRole + 1).toString();
    m_startEdit->setDateTime(QDateTime::fromString(startStr, QStringLiteral("yyyy-MM-dd HH:mm")));
    m_endEdit->setDateTime(QDateTime::fromString(endStr, QStringLiteral("yyyy-MM-dd HH:mm")));
}

void ReserveDialog::onConfirm()
{
    const std::string equipId =
        m_equipBox->currentData().toString().toStdString();
    const DateTime start = AppContext::toCoreTime(m_startEdit->dateTime());
    const DateTime end   = AppContext::toCoreTime(m_endEdit->dateTime());

    User* u = AppContext::get().currentUser();
    if (!u) {
        QMessageBox::critical(this, QStringLiteral("预约失败"),
                              QStringLiteral("登录状态已失效，请重新登录。"));
        return;
    }

    const std::string resId =
        AppContext::get().nextReservationId().toStdString();
    Reservation reservation(resId, equipId, u->getId(), start, end);

    const ApplyResult result =
        AppContext::get().manager().apply_reservation(reservation);

    if (result.status == ReservationStatus::Rejected) {
        QMessageBox::critical(this, QStringLiteral("预约失败"),
                              AppContext::toQString(result.message));
        return;
    }

    QMessageBox::information(this, QStringLiteral("预约结果"),
                             AppContext::toQString(result.to_string()));
    accept();
}
