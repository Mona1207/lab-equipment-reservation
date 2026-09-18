// ============================================================
// 文件说明：AdminPage.cpp —— 审批管理页具体实现
// ============================================================

#include "AdminPage.h"
#include "AppContext.h"
#include "Theme.h"

#include "Reservation.h"

// 引入 Qt 控件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>

// ============================================================
// 构造函数：创建审批管理页
// ============================================================
AdminPage::AdminPage(QWidget *parent)
    : QWidget(parent)
{
    // ---- 顶部按钮行 ----
    m_approveBtn = new QPushButton(QStringLiteral("✔ 通过"), this);
    m_rejectBtn  = new QPushButton(QStringLiteral("✖ 拒绝"), this);
    m_returnBtn  = new QPushButton(QStringLiteral("↩ 确认归还"), this);
    QPushButton *refreshBtn = new QPushButton(QStringLiteral("🔄 刷新"), this);

    // 连接按钮信号
    connect(m_approveBtn, &QPushButton::clicked, this, &AdminPage::onApprove);
    connect(m_rejectBtn,  &QPushButton::clicked, this, &AdminPage::onReject);
    connect(m_returnBtn,  &QPushButton::clicked, this, &AdminPage::onMarkReturned);
    connect(refreshBtn,   &QPushButton::clicked, this, &AdminPage::onRefresh);

    // 按钮行水平布局
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(m_approveBtn);
    btnRow->addWidget(m_rejectBtn);
    btnRow->addWidget(m_returnBtn);
    btnRow->addWidget(refreshBtn);
    btnRow->addStretch();

    // ---- 预约表格（6列：预约号、设备、用户ID、开始、结束、状态）----
    m_table = new QTableWidget(0, 6, this);
    m_table->setHorizontalHeaderLabels(
        { QStringLiteral("预约号"), QStringLiteral("设备"), QStringLiteral("用户ID"),
          QStringLiteral("开始时间"), QStringLiteral("结束时间"), QStringLiteral("状态") });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 只读
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);  // 整行选中
    m_table->setAlternatingRowColors(true);  // 斑马纹
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // ---- 整体垂直布局 ----
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(16);
    lay->addLayout(btnRow);  // 顶部按钮
    lay->addWidget(m_table); // 表格（拉伸占满）

    refreshTable();  // 初始加载
}

// ============================================================
// refreshTable()：刷新审批表格
// 显示所有待审批和已通过的预约
// ============================================================
void AdminPage::refreshTable()
{
    // 先数一下有多少条待审批/已通过的
    const auto& all = AppContext::get().manager().reservations();
    int count = 0;
    for (const Reservation& r : all)
        if (r.status() == ReservationStatus::Pending ||
            r.status() == ReservationStatus::Approved)
            ++count;

    m_table->setRowCount(count);

    // 填充每一行
    int i = 0;
    for (const Reservation& r : all) {
        if (r.status() != ReservationStatus::Pending &&
            r.status() != ReservationStatus::Approved) continue;

        // 第0列：预约号
        m_table->setItem(i, 0, new QTableWidgetItem(
            AppContext::toQString(r.id())));
        // 第1列：设备名
        m_table->setItem(i, 1, new QTableWidgetItem(
            AppContext::get().equipmentName(r.equipment_id())));
        // 第2列：用户ID
        m_table->setItem(i, 2, new QTableWidgetItem(
            QString::number(r.user_id())));
        // 第3列：开始时间
        m_table->setItem(i, 3, new QTableWidgetItem(
            AppContext::toQString(r.start_time().to_string())));
        // 第4列：结束时间
        m_table->setItem(i, 4, new QTableWidgetItem(
            AppContext::toQString(r.end_time().to_string())));
        // 第5列：状态
        m_table->setItem(i, 5, Theme::makeStatusItem(
            AppContext::toQString(r.status_name())));
        ++i;
    }
}

// ============================================================
// 辅助函数：从表格选中行取出预约id
// ============================================================
static std::string selectedReservationId(QTableWidget *table)
{
    const int row = table->currentRow();  // 当前选中行
    if (row < 0) return "";
    return table->item(row, 0)->text().toStdString();  // 第0列是预约号
}

// ============================================================
// onApprove()：审批通过
// ============================================================
void AdminPage::onApprove()
{
    const std::string id = selectedReservationId(m_table);
    if (id.empty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条待审批的预约"));
        return;
    }

    // 调用核心函数审批通过
    if (!AppContext::get().manager().approve_reservation(id)) {
        QMessageBox::warning(this, QStringLiteral("操作失败"),
                             QStringLiteral("该预约当前状态不可审批。"));
        return;
    }

    QMessageBox::information(this, QStringLiteral("审批结果"),
                             QStringLiteral("预约已通过，设备状态已更新。"));
    refreshTable();
    emit dataChanged();  // 通知主窗口刷新其他页面
}

// ============================================================
// onReject()：审批拒绝
// ============================================================
void AdminPage::onReject()
{
    const std::string id = selectedReservationId(m_table);
    if (id.empty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条待审批的预约"));
        return;
    }

    // 确认拒绝
    const auto ans = QMessageBox::question(this, QStringLiteral("拒绝预约"),
        QStringLiteral("确定拒绝这条预约吗？"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ans != QMessageBox::Yes) return;

    // 调用核心函数拒绝
    if (!AppContext::get().manager().reject_reservation(id)) {
        QMessageBox::warning(this, QStringLiteral("操作失败"),
                             QStringLiteral("该预约当前状态不可拒绝。"));
        return;
    }

    refreshTable();
    emit dataChanged();
}

// ============================================================
// onMarkReturned()：确认归还（用户用完设备，管理员确认归还）
// ============================================================
void AdminPage::onMarkReturned()
{
    const std::string id = selectedReservationId(m_table);
    if (id.empty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条已通过的预约"));
        return;
    }

    // 调用核心函数确认归还
    if (!AppContext::get().manager().mark_returned(id)) {
        QMessageBox::warning(this, QStringLiteral("操作失败"),
                             QStringLiteral("只能对已通过且未归还的预约执行归还操作。"));
        return;
    }

    QMessageBox::information(this, QStringLiteral("归还确认"),
                             QStringLiteral("设备已归还，状态已更新为可用。"));
    refreshTable();
    emit dataChanged();
}

// 刷新按钮
void AdminPage::onRefresh()
{
    refreshTable();
}
