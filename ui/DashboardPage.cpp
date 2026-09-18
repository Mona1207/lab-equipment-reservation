// ============================================================
// 文件说明：DashboardPage.cpp —— 首页仪表盘具体实现
// ============================================================

#include "DashboardPage.h"
#include "AppContext.h"
#include "Theme.h"

// 引入核心层的类
#include "Reservation.h"
#include "Equipment.h"
#include "MaintenanceTask.h"
#include "DateTime.h"

// 引入 Qt 布局和控件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>
#include <QDateTime>

// ============================================================
// createKpiCard() 函数：创建一个 KPI 卡片
// KPI = Key Performance Indicator（关键指标）
// 每个卡片：左边彩色图标方块，右边大数字 + 标题
// 比如：蓝色方块 🖥  +  数字 4  +  文字“设备总数”
// ============================================================
static QFrame* createKpiCard(const QString& icon, const QString& title,
                              QLabel*& valueLabel, const QString& color)
{
    auto *card = new QFrame;  // QFrame 是一个带边框的容器控件

    // 设置卡片样式：白底、圆角、边框，hover 时边框变蓝
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

    // 图标方块：彩色背景 + emoji
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
    iconLay->addWidget(iconLab, 0, Qt::AlignCenter);  // emoji 居中

    // 大数字标签（比如“4”）
    valueLabel = new QLabel(QStringLiteral("0"), card);
    valueLabel->setStyleSheet(QStringLiteral(
        "font-size: 30px; font-weight: bold; color: %1; background: transparent;").arg(color));

    // 标题文字（比如“设备总数”）
    auto *titleLab = new QLabel(title, card);
    titleLab->setStyleSheet(QStringLiteral(
        "color: #5a6a7e; font-size: 13px; background: transparent;"));

    // 卡片内部垂直布局：图标 -> 大数字 -> 标题
    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(18, 16, 18, 16);
    lay->setSpacing(6);
    lay->addWidget(iconBox);
    lay->addWidget(valueLabel);
    lay->addWidget(titleLab);

    return card;
}

// ============================================================
// 构造函数：创建仪表盘页面
// ============================================================
DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
{
    // ---- 第 1 步：上面一行 4 个 KPI 卡片 ----
    auto *kpiRow = new QHBoxLayout;  // 水平布局，4个卡片并排
    kpiRow->setSpacing(16);  // 卡片间距 16 像素

    // 4 个卡片：设备总数（蓝）、今日预约（绿）、待审批（黄）、待保养（红）
    kpiRow->addWidget(createKpiCard(QStringLiteral("🖥"), QStringLiteral("设备总数"),
                                     m_equipCount, QStringLiteral("#1565c0")));
    kpiRow->addWidget(createKpiCard(QStringLiteral("📅"), QStringLiteral("今日预约"),
                                     m_todayResv, QStringLiteral("#2e7d32")));
    kpiRow->addWidget(createKpiCard(QStringLiteral("⏳"), QStringLiteral("待审批"),
                                     m_pendingCount, QStringLiteral("#f57f17")));
    kpiRow->addWidget(createKpiCard(QStringLiteral("🔧"), QStringLiteral("待保养"),
                                     m_maintCount, QStringLiteral("#c62828")));

    // ---- 第 2 步：下面一行，左边最近预约，右边设备状态 ----
    auto *bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(16);

    // 左边：最近预约表格（占 3 份宽度）
    auto *recentBox = new QFrame;
    recentBox->setStyleSheet(QStringLiteral(
        "QFrame { background: #ffffff; border-radius: 6px;"
        " border: 1px solid #d0d7e0; }"));

    auto *recentTitle = new QLabel(QStringLiteral("📋 最近预约记录"), recentBox);
    recentTitle->setStyleSheet(QStringLiteral(
        "font-size: 15px; font-weight: bold; color: #1f2937; padding: 4px 0; background: transparent;"));

    // 4 列表格：设备、用户ID、时间、状态
    m_recentTable = new QTableWidget(0, 4, recentBox);
    m_recentTable->setHorizontalHeaderLabels(
        { QStringLiteral("设备"), QStringLiteral("用户ID"), QStringLiteral("时间"), QStringLiteral("状态") });
    m_recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 只读
    m_recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentTable->setAlternatingRowColors(true);  // 斑马纹
    m_recentTable->verticalHeader()->setVisible(false);  // 隐藏行号
    m_recentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_recentTable->setMaximumHeight(280);  // 最大高度 280

    auto *recentLay = new QVBoxLayout(recentBox);
    recentLay->setContentsMargins(20, 16, 20, 16);
    recentLay->setSpacing(10);
    recentLay->addWidget(recentTitle);
    recentLay->addWidget(m_recentTable);

    // 右边：设备状态分布（占 2 份宽度）
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
    m_statusDist->setWordWrap(true);  // 自动换行

    auto *statusLay = new QVBoxLayout(statusBox);
    statusLay->setContentsMargins(20, 16, 20, 16);
    statusLay->setSpacing(10);
    statusLay->addWidget(statusTitle);
    statusLay->addWidget(m_statusDist);
    statusLay->addStretch();

    // 加入底部行：左边占3份，右边占2份
    bottomRow->addWidget(recentBox, 3);
    bottomRow->addWidget(statusBox, 2);

    // ---- 第 3 步：整体垂直布局 ----
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(24, 24, 24, 24);
    mainLay->setSpacing(16);
    mainLay->addLayout(kpiRow);      // 上面 KPI 行
    mainLay->addLayout(bottomRow, 1); // 下面内容区（拉伸占满）

    refresh();  // 初始刷新数据
}

// ============================================================
// refresh() 函数：刷新仪表盘所有数据
// 从核心管理器读最新数据，更新到界面上
// ============================================================
void DashboardPage::refresh()
{
    ReservationManager& mgr = AppContext::get().manager();
    const DateTime now = DateTime::now();  // 当前时间

    // ---- 设备总数 ----
    const int equipTotal = static_cast<int>(mgr.equipments().size());
    m_equipCount->setText(QString::number(equipTotal));

    // ---- 今日预约数 + 待审批数 ----
    int todayCount = 0;
    int pendingCount = 0;
    for (const Reservation& r : mgr.reservations()) {
        if (r.start_time().same_day(now)) ++todayCount;  // 今天开始的预约
        if (r.status() == ReservationStatus::Pending) ++pendingCount;  // 待审批状态
    }
    m_todayResv->setText(QString::number(todayCount));
    m_pendingCount->setText(QString::number(pendingCount));

    // ---- 待保养数 ----
    mgr.scan_due_maintenance();  // 扫描生成到期保养任务
    int maintCount = 0;
    for (const MaintenanceTask& t : mgr.maintenance_tasks()) {
        if (!t.executed()) ++maintCount;  // 未执行的
    }
    m_maintCount->setText(QString::number(maintCount));

    // ---- 最近预约（取最后 8 条，倒序显示）----
    const auto& allResv = mgr.reservations();
    const int showCount = qMin(8, static_cast<int>(allResv.size()));  // 最多显示8条
    m_recentTable->setRowCount(showCount);

    for (int i = 0; i < showCount; ++i) {
        const Reservation& r = allResv[allResv.size() - 1 - i]; // 倒序：最新的在最上面
        m_recentTable->setItem(i, 0, new QTableWidgetItem(
            AppContext::get().equipmentName(r.equipment_id())));
        m_recentTable->setItem(i, 1, new QTableWidgetItem(QString::number(r.user_id())));
        m_recentTable->setItem(i, 2, new QTableWidgetItem(
            AppContext::toQString(r.start_time().to_string()) + " ~ " +
            AppContext::toQString(r.end_time().to_string())));
        m_recentTable->setItem(i, 3, Theme::makeStatusItem(
            AppContext::toQString(r.status_name())));
    }

    // ---- 设备状态分布（彩色文字）----
    int available = 0, borrowed = 0, maintenance = 0;
    for (const auto& eqPtr : mgr.equipments()) {
        switch (eqPtr->status()) {
        case EquipmentStatus::Available:   ++available; break;   // 可用
        case EquipmentStatus::Borrowed:    ++borrowed; break;   // 已借出
        case EquipmentStatus::Maintenance: ++maintenance; break; // 维护中
        }
    }

    // 用 HTML 格式显示，带颜色
    m_statusDist->setText(QStringLiteral(
        "<div style='line-height:2.2;'>"
        "<span style='color:#2e7d32;'>● 可用：%1 台</span><br>"
        "<span style='color:#e65100;'>● 已借出：%2 台</span><br>"
        "<span style='color:#546e7a;'>● 维护中：%3 台</span><br><br>"
        "<span style='color:#5a6a7e; font-size:12px;'>共 %4 台设备</span>"
        "</div>").arg(available).arg(borrowed).arg(maintenance).arg(equipTotal));
}
