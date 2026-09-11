#include "StatsPage.h"    // 对应头文件
#include "AppContext.h"   // 界面适配层
#include "DateTime.h"     // 核心时间

#include "Equipment.h"    // 核心设备实体

#include <QVBoxLayout>    // 垂直布局
#include <QHBoxLayout>    // 水平布局
#include <QPushButton>    // 按钮
#include <QLabel>         // 文本标签
#include <QPainter>       // 绘图（抗锯齿）
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

QT_USE_NAMESPACE
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namespace QtCharts;
#endif

// 构造统计页
StatsPage::StatsPage(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *refreshBtn = new QPushButton(QStringLiteral("刷新统计"), this);
    connect(refreshBtn, &QPushButton::clicked, this, &StatsPage::refresh);

    QHBoxLayout *top = new QHBoxLayout;
    top->addWidget(new QLabel(QStringLiteral("📊 统计分析"), this));
    top->addStretch();
    top->addWidget(refreshBtn);

    // 上方：饼图 + 使用率图（左右各半）
    m_pieView = new QChartView(this);
    m_pieView->setRenderHint(QPainter::Antialiasing);
    m_pieView->setMinimumHeight(280);

    m_usageView = new QChartView(this);
    m_usageView->setRenderHint(QPainter::Antialiasing);
    m_usageView->setMinimumHeight(280);

    QHBoxLayout *topCharts = new QHBoxLayout;
    topCharts->setSpacing(16);
    topCharts->addWidget(m_pieView, 1);
    topCharts->addWidget(m_usageView, 1);

    // 下方：预约次数柱状图（全宽）
    m_barView = new QChartView(this);
    m_barView->setRenderHint(QPainter::Antialiasing);
    m_barView->setMinimumHeight(300);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(16);
    lay->addLayout(top);
    lay->addLayout(topCharts);
    lay->addWidget(m_barView, 1);

    refresh();
}

// 统一刷新所有图表
void StatsPage::refresh()
{
    drawPieChart();
    drawUsageChart();
    drawBarChart();
}

// 设备状态饼图
void StatsPage::drawPieChart()
{
    ReservationManager& mgr = AppContext::get().manager();
    int available = 0, borrowed = 0, maintenance = 0;
    for (const auto& eqPtr : mgr.equipments()) {
        switch (eqPtr->status()) {
        case EquipmentStatus::Available:   ++available; break;
        case EquipmentStatus::Borrowed:    ++borrowed; break;
        case EquipmentStatus::Maintenance: ++maintenance; break;
        }
    }

    auto *series = new QPieSeries();
    if (available > 0)   series->append(QStringLiteral("可用"), available);
    if (borrowed > 0)    series->append(QStringLiteral("已借出"), borrowed);
    if (maintenance > 0) series->append(QStringLiteral("维护中"), maintenance);

    if (series->slices().isEmpty()) {
        series->append(QStringLiteral("无设备"), 1);
    }

    // 设置颜色
    const QList<QColor> colors = {
        QColor(34, 154, 22),   // 可用-绿
        QColor(230, 126, 0),   // 已借出-橙
        QColor(120, 132, 150), // 维护中-灰
        QColor(200, 200, 200)  // 无设备
    };
    for (int i = 0; i < series->slices().size() && i < colors.size(); ++i) {
        series->slices()[i]->setColor(colors[i]);
        series->slices()[i]->setLabelVisible(true);
        series->slices()[i]->setLabel(QStringLiteral("%1 %2%")
            .arg(series->slices()[i]->label())
            .arg(QString::number(series->slices()[i]->percentage() * 100, 'f', 0)));
    }

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QStringLiteral("设备状态分布"));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QChart *old = m_pieView->chart();
    m_pieView->setChart(chart);
    if (old) old->deleteLater();
}

// 设备使用率横向柱状图（使用次数 / 总使用次数）
void StatsPage::drawUsageChart()
{
    ReservationManager& mgr = AppContext::get().manager();
    const auto& equipments = mgr.equipments();

    int totalUsage = 0;
    for (const auto& eqPtr : equipments)
        totalUsage += eqPtr->usage_count();
    if (totalUsage == 0) totalUsage = 1; // 防除零

    auto *set = new QBarSet(QStringLiteral("使用率(%)"));
    QStringList categories;

    for (const auto& eqPtr : equipments) {
        categories << AppContext::toQString(eqPtr->name());
        const double pct = static_cast<double>(eqPtr->usage_count()) / totalUsage * 100.0;
        *set << pct;
    }
    set->setColor(QColor(72, 135, 255));

    auto *series = new QHorizontalBarSeries();
    series->append(set);

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QStringLiteral("设备使用率（按使用次数占比）"));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto *axisY = new QBarCategoryAxis();
    axisY->append(categories);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    auto *axisX = new QValueAxis();
    axisX->setTitleText(QStringLiteral("占比(%)"));
    axisX->setRange(0, 100);
    axisX->setLabelFormat(QStringLiteral("%.0f"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QChart *old = m_usageView->chart();
    m_usageView->setChart(chart);
    if (old) old->deleteLater();
}

// 预约次数柱状图（原有逻辑保留）
void StatsPage::drawBarChart()
{
    ReservationManager& mgr = AppContext::get().manager();

    const DateTime now = DateTime::now();
    const long long cur = now.to_minutes();
    const DateTime from = DateTime::from_minutes(cur - 365LL * 24 * 60);
    const DateTime to   = DateTime::from_minutes(cur + 365LL * 24 * 60);

    const auto ranking = mgr.ranking_by_count(from, to);
    auto *set = new QBarSet(QStringLiteral("预约次数"));
    QStringList categories;
    qreal maxValue = 1;

    for (const auto& kv : ranking) {
        categories << AppContext::get().equipmentName(kv.first);
        *set << static_cast<qreal>(kv.second);
        maxValue = qMax(maxValue, static_cast<qreal>(kv.second));
    }

    auto *series = new QBarSeries();
    series->append(set);
    set->setColor(QColor(72, 135, 255));

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QStringLiteral("设备被预约次数柱状图"));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto *axisY = new QValueAxis();
    axisY->setTitleText(QStringLiteral("预约次数"));
    axisY->setRange(0, maxValue + 1);
    axisY->setLabelFormat(QStringLiteral("%d"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChart *old = m_barView->chart();
    m_barView->setChart(chart);
    if (old) old->deleteLater();
}
