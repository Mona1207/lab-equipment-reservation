#ifndef THEME_H    // 头文件保护宏开始
#define THEME_H    // 定义保护宏

#include <QString>  // Qt 字符串
class QColor;       // 前向声明颜色
class QTableWidgetItem;   // 前向声明表格单元

// =============================================================
// Theme —— 界面组(D)全局视觉主题
// 集中放：① 全局 QSS 样式表；② 状态中文文案 -> 颜色 的映射；
// ③ 生成带彩色圆点的状态单元格。只依赖 Qt，不碰核心库。
// =============================================================
namespace Theme {

void apply();   // 应用全局字体与样式表（在 QApplication 构造后调用一次）

// 状态颜色：可用=绿 已借出=橙 维护中=灰 待审批=黄 已通过=蓝 已归还=绿 已拒绝=红
QColor statusColor(const QString& statusName);

// 生成"● 状态"样式的单元格：圆点与文字同色、加粗、居中
QTableWidgetItem* makeStatusItem(const QString& statusName);

}   // namespace Theme

#endif // THEME_H  // 头文件保护结束
