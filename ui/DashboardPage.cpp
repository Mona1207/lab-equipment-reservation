#include "DashboardPage.h"   // 对应头文件
#include "AppContext.h"      // 界面适配层
#include "Theme.h"           // 全局主题

#include "Reservation.h"     // 核心预约实体
#include "Equipment.h"       // 核心设备实体
#include "MaintenanceTask.h" // 核心保养任务
#include "DateTime.h"        // 核心时间

#include <QVBoxLayout>       // 垂直布局
#include <QHBoxLayout>       // 水平布局
#include <QLabel>            // 标签
#include <QTableWidget>      // 表格
#include <QHeaderView>       // 表头
#include <QFrame>            // 卡片框架
#include <QDateTime>         // Qt 时间（判断今天）

// 创建一个 KPI 卡片：图标方块 + 大数字 + 标题（严肃工业风格）
static QFrame* createKpiCard(const QString& icon, const QString& title,
                              QLabel*& valueLabel, const QString& color)
{
    auto *card = new QFrame;
    card->setStyleSheet(QStringLiteral(
        "QFrame {"
        "  background: #ffffff;"
        "  border-radius: 6px;"
        "  border: 1px solid #d0d7e0;"
        "}"
        "QFrame:hover {"
        "  background: #ffffff;"
        "  border: 1px solid %1;"
        "}").arg(color));

    // 图标方块：实色圆角小方块
    auto *iconBox = new QFrame(card);
    iconBox->setFixedSize(46, 46);
    iconBox->setStyleSheet(QStringLiteral(
        "QFrame {"
        "  background: %1;"
        "  border-radius: 4px; border: none;"
        "}").arg(color));
    auto *iconLab = new QLabel(icon, iconBox);
    iconLab->setStyleSheet(QStringLiteral("font-size: 22px; background: transparent;"));
    auto *iconLay = new QHBoxLayout(iconBox);
    iconLay->setContentsMargins(0, 0, 0, 0);
    iconLay->addWidget(iconLab, 0, Qt::AlignCenter);

    valueLabel = new QLabel(QStringLiteral("0"), card);
    valueLabel->setStyleSheet(QStringLiteral(
        "font-size: 30px; font-weight: bold; color: %1; background: transparent;").arg(color));

    auto *titleLab = new QLabel(title, card);
    titleLab->setStyleSheet(QStringLiteral(
        "color: #5a6a7e; font-size: 13px; background: transparent;"));

    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(18, 16, 18, 16);
    lay->setSpacing(6);
    lay->addWidget(iconBox);
    lay->addWidget(valueLabel);
    lay->addWidget(titleLab);

    return card;
}

// 构造仪表盘
DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    // ---- KPI 卡片行 ----
    auto *kpiRow = new QHBoxLayout;
    kpiRow->setSpacing(16);

    kpiRow->addWidget(createKpiCard(QStringLiteral("🖥"), QStringLiteral("设备总数"),
                                     m_equipCount, QStringLiteral("#1565c0")));
    kpiRow->addWidget(createKpiCard(QStringLiteral("📅"), QStringLiteral("今日预约"),
                                     m_todayResv, QStringLiteral("#2e7d32")));
    kpiRow->addWidget(createKpiCard(QStringLiteral("⏳"), QStringLiteral("待审批"),
                                     m_pendingCount, QStringLiteral("#f57f17")));
    kpiRow->addWidget(createKpiCard(QStringLiteral("🔧"), QStringLiteral("待保养"),
                                     m_maintCount, QStringLiteral("#c62828")));

    // ---- 下方：最近预约 + 设备状态 ----
    auto *bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(16);

    // 最近预约
    auto *recentBox = new QFrame;
    recentBox->setStyleSheet(QStringLiteral(
        "QFrame { background: #ffffff; border-radius: 6px;"
        " border: 1px solid #d0d7e0; }"));
    auto *recentTitle = new QLabel(QStringLiteral("📋 最近预约记录"), recentBox);
    recentTitle->setStyleSheet(QStringLiteral(
        "font-size: 15px; font-weight: bold; color: #1f2937; padding: 4px 0; background: transparent;"));

    m_recentTable = new QTableWidget(0, 4, recentBox);
    m_recentTable->setHorizontalHeaderLabels(
        { QStringLiteral("设备"), QStringLiteral("用户ID"), QStringLiteral("时间"), QStringLiteral("状态") });
    m_recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentTable->setAlternatingRowColors(true);
    m_recentTable->verticalHeader()->setVisible(false);
    m_recentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_recentTable->setMaximumHeight(280);

    auto *recentLay = new QVBoxLayout(recentBox);
    recentLay->setContentsMargins(20, 16, 20, 16);
    recentLay->setSpacing(10);
    recentLay->addWidget(recentTitle);
    recentLay->addWidget(m_recentTable);

    // 设备状态分布
    auto *statusBox = new QFrame;
    statusBox->setStyleSheet(QStringLiteral(
        "QFrame { background: #ffffff; border-radius: 6px;"
        " border: 1px solid #d0d7e0; }"));
    auto *statusTitle = new QLabel(QStringLiteral("📊 设备状态分布"), statusBox);
    statusTitle->setStyleSheet(QStringLiteral(
        "font-size: 15px; font-weight: bold; color: #1f2937; padding: 4px 0; background: transparent;"));

    m_statusDist = new QLabel(statusBox);
    m_statusDist->setStyleSheet(QStringLiteral(
        "font-size: 14px; color: #1f2937; line-height: 2;"));
    m_statusDist->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_statusDist->setWordWrap(true);

    auto *statusLay = new QVBoxLayout(statusBox);
    statusLay->setContentsMargins(20, 16, 20, 16);
    statusLay->setSpacing(10);
    statusLay->addWidget(statusTitle);
    statusLay->addWidget(m_statusDist);
    statusLay->addStretch();

    bottomRow->addWidget(recentBox, 3);
    bottomRow->addWidget(statusBox, 2);

    // ---- 整体布局 ----
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(24, 24, 24, 24);
    mainLay->setSpacing(16);
    mainLay->addLayout(kpiRow);
    mainLay->addLayout(bottomRow, 1);

    refresh();
}

// 刷新仪表盘所有数据
void DashboardPage::refresh()
{
    ReservationManager& mgr = AppContext::get().manager();
    const DateTime now = DateTime::now();

    // ---- 设备总数 ----
    const int equipTotal = static_cast<int>(mgr.equipments().size());
    m_equipCount->setText(QString::number(equipTotal));

    // ---- 今日预约数 + 待审批数 ----
    int todayCount = 0;
    int pendingCount = 0;
    for (const Reservation& r : mgr.reservations()) {
        if (r.start_time().same_day(now)) ++todayCount;
        if (r.status() == ReservationStatus::Pending) ++pendingCount;
    }
    m_todayResv->setText(QString::number(todayCount));
    m_pendingCount->setText(QString::number(pendingCount));

    // ---- 待保养数（扫描生成到期任务，统计未执行的）----
    mgr.scan_due_maintenance();
    int maintCount = 0;
    for (const MaintenanceTask& t : mgr.maintenance_tasks()) {
        if (!t.executed()) ++maintCount;
    }
    m_maintCount->setText(QString::number(maintCount));

    // ---- 最近预约（取最后 8 条，倒序显示）----
    const auto& allResv = mgr.reservations();
    const int showCount = qMin(8, static_cast<int>(allResv.size()));
    m_recentTable->setRowCount(showCount);
    for (int i = 0; i < showCount; ++i) {
        const Reservation& r = allResv[allResv.size() - 1 - i]; // 倒序
        m_recentTable->setItem(i, 0, new QTableWidgetItem(
            AppContext::get().equipmentName(r.equipment_id())));
        m_recentTable->setItem(i, 1, new QTableWidgetItem(QString::number(r.user_id())));
        m_recentTable->setItem(i, 2, new QTableWidgetItem(
            AppContext::toQString(r.start_time().to_string()) + " ~ " +
            AppContext::toQString(r.end_time().to_string())));
        m_recentTable->setItem(i, 3, Theme::makeStatusItem(
            AppContext::toQString(r.status_name())));
    }

    // ---- 设备状态分布 ----
    int available = 0, borrowed = 0, maintenance = 0;
    for (const auto& eqPtr : mgr.equipments()) {
        switch (eqPtr->status()) {
        case EquipmentStatus::Available:   ++available; break;
        case EquipmentStatus::Borrowed:    ++borrowed; break;
        case EquipmentStatus::Maintenance: ++maintenance; break;
        }
    }
    m_statusDist->setText(QStringLiteral(
        "<div style='line-height:2.2;'>"
        "<span style='color:#2e7d32;'>● 可用：%1 台</span><br>"
        "<span style='color:#e65100;'>● 已借出：%2 台</span><br>"
        "<span style='color:#546e7a;'>● 维护中：%3 台</span><br><br>"
        "<span style='color:#5a6a7e; font-size:12px;'>共 %4 台设备</span>"
        "</div>").arg(available).arg(borrowed).arg(maintenance).arg(equipTotal));
}
