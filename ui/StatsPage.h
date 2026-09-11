#ifndef STATSPAGE_H   // 头文件保护宏开始
#define STATSPAGE_H   // 定义头文件保护宏

#include <QWidget>    // 控件基类
#include <QtGlobal>   // 版本宏 QT_VERSION

// Qt5 的 Charts 类在 QtCharts 命名空间，Qt6 移到 Qt 命名空间
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace QtCharts { class QChartView; }
#else
QT_BEGIN_NAMESPACE
class QChartView;
QT_END_NAMESPACE
#endif

// =============================================================
// StatsPage —— 统计分析页（界面组 D，增强版）
// 三图布局：
//   左上：设备状态饼图（可用/已借出/维护中）
//   右上：设备使用率横向柱状图（使用次数占比）
//   下方：设备被预约次数柱状图（核心 ranking_by_count）
// =============================================================
class StatsPage : public QWidget
{
    Q_OBJECT
public:
    explicit StatsPage(QWidget *parent = nullptr);

public slots:
    void refresh();   // 刷新所有图表（对外统一接口）

private slots:
    void drawBarChart();      // 预约次数柱状图
    void drawPieChart();      // 设备状态饼图
    void drawUsageChart();    // 设备使用率横向柱状图

private:
    QChartView *m_barView;    // 预约次数柱状图
    QChartView *m_pieView;    // 设备状态饼图
    QChartView *m_usageView;  // 设备使用率图
};

#endif // STATSPAGE_H  // 头文件保护结束
