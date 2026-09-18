// ============================================================
// 文件说明：StatsPage.cpp —— 统计分析页具体实现
// ============================================================

#include "StatsPage.h"
#include "AppContext.h"

#include "Reservation.h"
#include "Equipment.h"
#include "DateTime.h"

// 引入 Qt Charts 模块（图表功能）
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegend>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

// 引入 Qt 布局
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QDateTime>

// 使用 QtCharts 命名空间
// 这样不用每次写 QtCharts::QBarSeries，直接写 QBarSeries 就行
QT_CHARTS_USE_NAMESPACE

// ============================================================
// 构造函数：创建统计分析页
// ============================================================
StatsPage::StatsPage(QWidget *parent)
    : QWidget(parent)
{
    // ---- 第 1 步：上面一排两个图表（左：柱状图，右：饼图）----

    // 左：设备使用率柱状图
    m_barChart = new QChart();
    m_barChart->setTitle(QStringLiteral("各设备使用次数（次）"));
    m_barChart->legend()->setVisible(false);  // 隐藏图例

    m_barSeries = new QBarSeries();  // 柱状图数据系列
    m_barChart->addSeries(m_barSeries);

    // X 轴：设备名称
    QBarCategoryAxis *barAxisX = new QBarCategoryAxis();
    m_barChart->addAxis(barAxisX, Qt::AlignBottom);
    m_barSeries->attachAxis(barAxisX);

    // Y 轴：次数
    QValueAxis *barAxisY = new QValueAxis();
    barAxisY->setRange(0, 10);  // 范围 0~10
    barAxisY->setLabelFormat("%d");
    m_barChart->addAxis(barAxisY, Qt::AlignLeft);
    m_barSeries->attachAxis(barAxisY);

    m_barView = new QChartView(m_barChart);
    m_barView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿（让线条平滑）
    m_barView->setMinimumHeight(260);

    // 右：设备状态分布饼图
    m_pieChart = new QChart();
    m_pieChart->setTitle(QStringLiteral("设备状态分布"));

    m_pieSeries = new QPieSeries();  // 饼图数据系列
    m_pieChart->addSeries(m_pieSeries);
    m_pieChart->legend()->setAlignment(Qt::AlignRight);  // 图例靠右

    m_pieView = new QChartView(m_pieChart);
    m_pieView->setRenderHint(QPainter::Antialiasing);
    m_pieView->setMinimumHeight(260);

    // 上面一行水平布局：左柱状图 + 右饼图
    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(16);
    topRow->addWidget(m_barView, 3);  // 左占3份
    topRow->addWidget(m_pieView, 2);  // 右占2份

    // ---- 第 2 步：中间折线图（近7天预约趋势）----
    m_lineChart = new QChart();
    m_lineChart->setTitle(QStringLiteral("近 7 天预约趋势（条）"));
    m_lineChart->legend()->setVisible(false);

    QLineSeries *lineSeries = new QLineSeries();  // 折线数据
    lineSeries->setColor(QColor("#1565c0"));     // 蓝色
    lineSeries->setPointsVisible(true);           // 显示数据点圆点

    m_lineChart->addSeries(lineSeries);

    // X 轴：日期
    QValueAxis *lineAxisX = new QValueAxis();
    lineAxisX->setRange(0, 6);  // 0~6 对应 7 天
    lineAxisX->setLabelFormat("Day %d");
    m_lineChart->addAxis(lineAxisX, Qt::AlignBottom);
    lineSeries->attachAxis(lineAxisX);

    // Y 轴：预约条数
    QValueAxis *lineAxisY = new QValueAxis();
    lineAxisY->setRange(0, 10);
    lineAxisY->setLabelFormat("%d");
    m_lineChart->addAxis(lineAxisY, Qt::AlignLeft);
    lineSeries->attachAxis(lineAxisY);

    m_lineView = new QChartView(m_lineChart);
    m_lineView->setRenderHint(QPainter::Antialiasing);
    m_lineView->setMinimumHeight(220);

    // ---- 第 3 步：底部文字统计汇总 ----
    m_summary = new QLabel(this);
    m_summary->setStyleSheet(QStringLiteral(
        "background: #ffffff; border: 1px solid #d0d7e0; border-radius: 6px;"
        "padding: 16px 20px; font-size: 13px; color: #1f2937; line-height: 1.8;"));
    m_summary->setWordWrap(true);

    // ---- 第 4 步：整体垂直布局 ----
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(24, 20, 24, 20);
    mainLay->setSpacing(16);
    mainLay->addLayout(topRow);     // 上面：柱状图+饼图
    mainLay->addWidget(m_lineView); // 中间：折线图
    mainLay->addWidget(m_summary);  // 底部：文字统计

    refresh();  // 初始加载数据
}

// ============================================================
// refresh()：刷新所有图表数据
// ============================================================
void StatsPage::refresh()
{
    ReservationManager& mgr = AppContext::get().manager();

    // ---- 1. 柱状图：每台设备的使用次数 ----
    // 先清空旧数据
    m_barSeries->clear();

    // X 轴类别列表
    QStringList categories;

    int maxUses = 0;  // 最大使用次数（用来设置Y轴范围）

    for (const auto& eqPtr : mgr.equipments()) {
        const Equipment* e = eqPtr.get();
        const int uses = e->usage_count();
        maxUses = qMax(maxUses, uses);  // 更新最大值

        // 新建一个柱子（一个 QBarSet 就是一根柱子）
        QBarSet *set = new QBarSet(AppContext::toQString(e->name()));
        *set << uses;  // 设置柱子高度

        // 给柱子上色
        set->setColor(QColor("#1565c0"));

        m_barSeries->append(set);  // 把柱子加到系列里
        categories << AppContext::toQString(e->name());  // 设备名加到X轴
    }

    // 更新 X 轴类别
    QBarCategoryAxis *barAxisX =
        qobject_cast<QBarCategoryAxis*>(m_barChart->axes(Qt::Horizontal).first());
    if (barAxisX) barAxisX->clear();
    for (const QString& cat : categories) barAxisX->append(cat);

    // 更新 Y 轴范围
    QValueAxis *barAxisY =
        qobject_cast<QValueAxis*>(m_barChart->axes(Qt::Vertical).first());
    if (barAxisY) barAxisY->setRange(0, qMax(5, maxUses + 2));

    // ---- 2. 饼图：设备状态分布 ----
    m_pieSeries->clear();

    int available = 0, borrowed = 0, maintenance = 0;
    for (const auto& eqPtr : mgr.equipments()) {
        switch (eqPtr->status()) {
        case EquipmentStatus::Available:   ++available; break;
        case EquipmentStatus::Borrowed:    ++borrowed; break;
        case EquipmentStatus::Maintenance: ++maintenance; break;
        }
    }

    // 把三种状态加到饼图里
    if (available > 0) {
        QPieSlice *s = m_pieSeries->append(
            QStringLiteral("可用 %1 台").arg(available), available);
        s->setColor(QColor("#2e7d32"));  // 绿色
    }
    if (borrowed > 0) {
        QPieSlice *s = m_pieSeries->append(
            QStringLiteral("已借出 %1 台").arg(borrowed), borrowed);
        s->setColor(QColor("#e65100"));  // 橙色
    }
    if (maintenance > 0) {
        QPieSlice *s = m_pieSeries->append(
            QStringLiteral("维护中 %1 台").arg(maintenance), maintenance);
        s->setColor(QColor("#546e7a"));  // 灰色
    }

    // ---- 3. 折线图：近7天预约趋势 ----
    QLineSeries *lineSeries =
        qobject_cast<QLineSeries*>(m_lineChart->series().first());
    if (lineSeries) lineSeries->clear();

    // 统计最近7天每天的预约条数
    const DateTime now = DateTime::now();
    QVector<int> dailyCounts(7, 0);  // 7个0，对应7天

    for (const Reservation& r : mgr.reservations()) {
        const int diffDays = r.start_time().days_until(now);  // 距离今天多少天
        // 只统计最近7天内的
        if (diffDays <= 0 && diffDays > -7) {
            const int idx = -diffDays;  // 0=今天，1=昨天...
            dailyCounts[idx]++;
        }
    }

    // 把数据加到折线图
    int maxDaily = 0;
    for (int i = 6; i >= 0; --i) {  // 从6天前到今天
        const int idx = 6 - i;     // 横轴坐标
        lineSeries->append(idx, dailyCounts[i]);
        maxDaily = qMax(maxDaily, dailyCounts[i]);
    }

    // 更新 Y 轴范围
    QValueAxis *lineAxisY =
        qobject_cast<QValueAxis*>(m_lineChart->axes(Qt::Vertical).first());
    if (lineAxisY) lineAxisY->setRange(0, qMax(3, maxDaily + 2));

    // ---- 4. 底部文字统计 ----
    const auto& allResv = mgr.reservations();
    int totalResv = static_cast<int>(allResv.size());
    int approvedCount = 0, pendingCount = 0, rejectedCount = 0, cancelledCount = 0;
    for (const Reservation& r : allResv) {
        switch (r.status()) {
        case ReservationStatus::Approved:  ++approvedCount;  break;
        case ReservationStatus::Pending:   ++pendingCount;   break;
        case ReservationStatus::Rejected:  ++rejectedCount;  break;
        case ReservationStatus::Cancelled: ++cancelledCount; break;
        default: break;
        }
    }

    // 设备总数
    const int equipTotal = static_cast<int>(mgr.equipments().size());
    // 活跃设备（已借出的）
    int activeEquip = 0;
    for (const auto& eqPtr : mgr.equipments())
        if (eqPtr->status() == EquipmentStatus::Borrowed) ++activeEquip;

    // 设置底部文字
    m_summary->setText(QStringLiteral(
        "📊 系统统计汇总：\n"
        "&nbsp;&nbsp;• 设备总数：%1 台（在用 %2 台）\n"
        "&nbsp;&nbsp;• 预约总数：%3 条（已通过 %4，待审批 %5，已拒绝 %6，已取消 %7）"
        ).arg(equipTotal).arg(activeEquip)
         .arg(totalResv).arg(approvedCount).arg(pendingCount)
         .arg(rejectedCount).arg(cancelledCount));
}
