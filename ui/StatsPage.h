// ============================================================
// 文件说明：StatsPage.h —— 统计分析页头文件
// 统计分析页：用图表展示设备使用率、预约趋势、状态分布
// ============================================================

#ifndef STATSPAGE_H
#define STATSPAGE_H

#include <QWidget>        // 基类

class QLabel;             // 前向声明：文字统计标签
class QChartView;         // 前向声明：Qt Charts 的图表视图
class QChart;             // 前向声明：图表对象
class QPieSeries;         // 前向声明：饼图数据系列
class QBarSeries;         // 前向声明：柱状图数据系列

// ============================================================
// StatsPage 类：统计分析页
// 3 个图表：
//   1. 设备使用率柱状图
//   2. 近7天预约趋势折线图
//   3. 设备状态分布饼图
// ============================================================
class StatsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StatsPage(QWidget *parent = nullptr);

public slots:
    void refresh();  // 刷新所有图表

private:
    QChartView *m_barView;     // 柱状图视图（设备使用率）
    QChart     *m_barChart;    // 柱状图对象
    QBarSeries *m_barSeries;  // 柱状图数据

    QChartView *m_lineView;    // 折线图视图（预约趋势）
    QChart     *m_lineChart;   // 折线图对象

    QChartView *m_pieView;     // 饼图视图（状态分布）
    QChart     *m_pieChart;    // 饼图对象
    QPieSeries *m_pieSeries;   // 饼图数据

    QLabel *m_summary;         // 文字统计汇总（总预约数等）
};

#endif // STATSPAGE_H
