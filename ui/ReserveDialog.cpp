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
#include <QMessageBox>       // 弹窗（红色错误框用 critical）
#include <QDate>             // 日期
#include <QTime>             // 时间
#include <QListWidget>       // 可用时段列表
#include <QListWidgetItem>   // 列表条目
#include <QLabel>            // 标签
#include <QVBoxLayout>       // 垂直布局
#include <QHBoxLayout>       // 水平布局

// 构造预约对话框
ReserveDialog::ReserveDialog(QWidget *parent)
    : QDialog(parent)       // 转交父窗口
{
    setWindowTitle(QStringLiteral("预约申请"));  // 对话框标题
    resize(420, 480);                            // 初始大小（加了可用时段列表后加高）

    m_equipBox = new QComboBox(this);            // 新建设备下拉框
    for (const auto& eqPtr : AppContext::get().manager().equipments()) // 遍历全部设备
    {
        const Equipment* e = eqPtr.get();        // 取设备裸指针
        // 显示"名称（编号·状态）"，隐藏数据存字符串设备id
        m_equipBox->addItem(
            QStringLiteral("%1（%2·%3）")
                .arg(AppContext::toQString(e->name()),
                     AppContext::toQString(e->id()),
                     AppContext::toQString(e->status_name())),
            AppContext::toQString(e->id()));     // data 存设备id字符串
    }

    const QDate today = QDate::currentDate();    // 今天
    m_startEdit = new QDateTimeEdit(QDateTime(today, QTime(14, 30)), this); // 默认14:30
    m_endEdit   = new QDateTimeEdit(QDateTime(today, QTime(15, 30)), this); // 默认15:30
    m_startEdit->setCalendarPopup(true);         // 点击弹出日历
    m_endEdit->setCalendarPopup(true);           // 点击弹出日历
    m_startEdit->setDisplayFormat("yyyy-MM-dd HH:mm"); // 显示格式
    m_endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");   // 显示格式

    // ---- 可用时段查询区 ----
    m_slotsBtn = new QPushButton(QStringLiteral("🔍 查看当天可用时段"), this);
    m_slotsBtn->setCursor(Qt::PointingHandCursor);
    connect(m_slotsBtn, &QPushButton::clicked, this, &ReserveDialog::onShowSlots);

    m_slotsList = new QListWidget(this);
    m_slotsList->setAlternatingRowColors(true);
    m_slotsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_slotsList->setMaximumHeight(140);
    m_slotsList->setStyleSheet(QStringLiteral(
        "QListWidget { background: #ffffff; border: 1px solid #d4dbe7; border-radius: 6px; }"
        "QListWidget::item { padding: 6px 10px; }"
        "QListWidget::item:selected { background: #eef3ff; color: #2b62d9; }"
        "QListWidget::item:hover { background: #f4f6fa; }"));
    connect(m_slotsList, &QListWidget::itemClicked, this, &ReserveDialog::onSlotSelected);

    QLabel *slotsHint = new QLabel(QStringLiteral("💡 点击时段可自动填充开始/结束时间"), this);
    slotsHint->setStyleSheet(QStringLiteral("color: #8a94a6; font-size: 11px;"));

    QDialogButtonBox *box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this); // 确定+取消按钮盒
    box->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认预约"));    // 确定改中文
    box->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));    // 取消改中文
    connect(box, &QDialogButtonBox::accepted, this, &ReserveDialog::onConfirm); // 确定->onConfirm
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);         // 取消->关闭

    QFormLayout *form = new QFormLayout;    // 表单布局
    form->addRow(QStringLiteral("选择设备："), m_equipBox);  // 设备行
    form->addRow(QStringLiteral("开始时间："), m_startEdit); // 开始行
    form->addRow(QStringLiteral("结束时间："), m_endEdit);   // 结束行

    QVBoxLayout *lay = new QVBoxLayout(this);    // 整体垂直布局
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(10);
    lay->addLayout(form);                          // 表单
    lay->addWidget(m_slotsBtn);                    // 查看可用时段按钮
    lay->addWidget(slotsHint);                     // 提示文字
    lay->addWidget(m_slotsList);                   // 可用时段列表
    lay->addStretch();
    lay->addWidget(box);                           // 按钮行
}

// "查看可用时段"：调核心 get_available_slots 扫描当天空闲窗口
void ReserveDialog::onShowSlots()
{
    const std::string equipId =
        m_equipBox->currentData().toString().toStdString(); // 取选中设备id
    if (equipId.empty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请先选择设备"));
        return;
    }

    // 以开始时间选择的日期为查询日期
    const QDate qdate = m_startEdit->dateTime().date();
    const DateTime queryDate(qdate.year(), qdate.month(), qdate.day(), 0, 0);

    // 最小时段长度 = 当前选择的预约时长（至少30分钟）
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
        // 把开始/结束时间存在 item 的 data 里，点击时取出填充
        item->setData(Qt::UserRole, AppContext::toQString(s.to_string()));
        item->setData(Qt::UserRole + 1, AppContext::toQString(e.to_string()));
        m_slotsList->addItem(item);
    }
}

// 点击可用时段条目 → 自动填充开始/结束时间
void ReserveDialog::onSlotSelected(QListWidgetItem *item)
{
    if (!item || item->flags() == Qt::NoItemFlags) return;
    const QString startStr = item->data(Qt::UserRole).toString();
    const QString endStr   = item->data(Qt::UserRole + 1).toString();
    // 核心 DateTime::to_string 格式是 "YYYY-MM-DD HH:MM"
    m_startEdit->setDateTime(QDateTime::fromString(startStr, QStringLiteral("yyyy-MM-dd HH:mm")));
    m_endEdit->setDateTime(QDateTime::fromString(endStr, QStringLiteral("yyyy-MM-dd HH:mm")));
}

// 点"确认预约"：构造核心预约并交给 apply_reservation 统一决策
void ReserveDialog::onConfirm()
{
    const std::string equipId =
        m_equipBox->currentData().toString().toStdString(); // 取选中设备id（字符串）
    const DateTime start = AppContext::toCoreTime(m_startEdit->dateTime()); // 转核心开始时间
    const DateTime end   = AppContext::toCoreTime(m_endEdit->dateTime());   // 转核心结束时间

    User* u = AppContext::get().currentUser();   // 当前登录用户
    if (!u) {                                    // 未登录兜底（正常流程不会发生）
        QMessageBox::critical(this, QStringLiteral("预约失败"),
                              QStringLiteral("登录状态已失效，请重新登录。")); // 红框
        return;
    }

    // 界面层生成预约编号，构造一条初始为"待审批"的核心预约
    const std::string resId =
        AppContext::get().nextReservationId().toStdString(); // 新预约编号
    Reservation reservation(resId, equipId, u->getId(), start, end); // 构造预约对象

    // 生死线：把预约交给核心（冲突检测/角色自动审批/抢占全部由核心完成）
    const ApplyResult result =
        AppContext::get().manager().apply_reservation(reservation); // 提交并取结果

    if (result.status == ReservationStatus::Rejected) {   // 核心判定拒绝
        QMessageBox::critical(this, QStringLiteral("预约失败"),
                              AppContext::toQString(result.message)); // 红框显示核心原因
        return;                                // 关键：不关闭对话框，便于当场改时间再试
    }

    // 未被拒绝即成功（学生=待审批，教师/管理员=已通过，可能伴随抢占）
    QMessageBox::information(this, QStringLiteral("预约结果"),
                             AppContext::toQString(result.to_string())); // 展示核心完整结论
    accept();                                // 以 Accepted 关闭，主窗口据此刷新
}
