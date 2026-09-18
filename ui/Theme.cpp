#include "Theme.h"     // 对应头文件
#include <QApplication> // 应用对象（设全局字体/样式表）
#include <QFont>        // 字体
#include <QColor>       // 颜色
#include <QBrush>       // 画刷（单元格文字色）
#include <QTableWidgetItem> // 表格单元格
#include <QHeaderView>  // 表头

namespace Theme {

// 应用全局主题：雅黑字体 + 全局 QSS
// 设计风格：严肃实验室管理系统 —— 深蓝侧栏 + 浅灰蓝工作区 + 白色卡片 + 科技蓝强调色
void apply()
{
    QFont font(QStringLiteral("Microsoft YaHei UI"));
    font.setPointSize(10);
    qApp->setFont(font);

    qApp->setStyleSheet(QStringLiteral(R"(
        /* ===== 全局基调：浅灰蓝纯色背景 ===== */
        QMainWindow {
            background: #eef1f5;
        }
        #MainContent {
            background: #eef1f5;
        }
        QDialog {
            background: #f5f7fa;
        }
        QWidget { color: #1f2937; font-size: 14px; }

        /* ===== 侧边导航：深蓝实色 ===== */
        #Sidebar {
            background: #1b2a41;
            border: none;
            border-right: 1px solid #142033;
        }
        #AppTitle {
            font-size: 14px; font-weight: bold; color: #ffffff;
            padding: 22px 8px 16px 20px;
        }
        #NavList {
            background: transparent; border: none; outline: none;
            font-size: 14px; color: #a8b5c7;
        }
        #NavList::item {
            padding: 11px 16px; margin: 2px 8px; border-radius: 4px;
            background: transparent;
            border-left: 3px solid transparent;
        }
        #NavList::item:hover { background: rgba(255, 255, 255, 0.08); color: #ffffff; }
        #NavList::item:selected {
            background: #1565c0;
            color: #ffffff; font-weight: bold;
            border-left: 3px solid #42a5f5;
        }
        #SidebarFooter {
            background: #16233a;
            border-top: 1px solid #142033;
        }
        #SidebarFooter QLabel { color: #8a99ad; font-size: 12px; }
        #UserNameLabel { color: #ffffff !important; font-size: 14px; font-weight: bold; }
        #RoleBadge {
            background: #1565c0;
            color: #ffffff; border-radius: 3px;
            padding: 3px 12px; font-size: 11px; font-weight: bold;
        }
        #LogoutButton {
            background: transparent;
            color: #a8b5c7; border: 1px solid #3a4a63;
            border-radius: 4px; padding: 8px 0; font-size: 13px;
        }
        #LogoutButton:hover { background: #c62828; color: #ffffff; border-color: #c62828; }

        /* ===== 顶栏：白色实色 ===== */
        #TopBar {
            background: #ffffff;
            border-bottom: 1px solid #d0d7e0;
        }
        #PageTitle { font-size: 20px; font-weight: bold; color: #1f2937; }
        #TopBar QLabel { color: #6b7b8f; font-size: 13px; }

        /* ===== 按钮：工业风格小矩形 ===== */
        QPushButton {
            background: #ffffff; color: #334155;
            border: 1px solid #c0c8d4;
            border-radius: 4px; padding: 8px 20px;
        }
        QPushButton:hover {
            border-color: #1565c0; color: #1565c0;
            background: #f0f6ff;
        }
        QPushButton:pressed { background: #d6e6f7; border-color: #0d47a1; color: #0d47a1; }
        QPushButton:disabled { color: #a0aab8; border-color: #dde2e9; background: #f5f7fa; }
        QPushButton[primary="true"] {
            background: #1565c0;
            color: #ffffff; border: 1px solid #1565c0; font-weight: bold;
        }
        QPushButton[primary="true"]:hover {
            background: #0d47a1; border-color: #0d47a1; color: #ffffff;
        }
        QPushButton[primary="true"]:pressed { background: #0a3a85; border-color: #0a3a85; color: #ffffff; }

        /* ===== 输入控件 ===== */
        QLineEdit, QComboBox, QDateTimeEdit, QSpinBox {
            background: #ffffff;
            border: 1px solid #c0c8d4;
            border-radius: 4px; padding: 7px 12px;
            selection-background-color: #1565c0;
            selection-color: #ffffff;
            color: #1f2937;
        }
        QLineEdit:focus, QComboBox:focus, QDateTimeEdit:focus, QSpinBox:focus {
            border: 1px solid #1565c0;
        }
        QLineEdit:disabled, QComboBox:disabled, QDateTimeEdit:disabled, QSpinBox:disabled {
            background: #eef1f5; color: #8a94a6;
        }
        QComboBox::drop-down { border: none; width: 26px; border-left: 1px solid #d0d7e0; }
        QComboBox QAbstractItemView {
            background: #ffffff; border: 1px solid #c0c8d4;
            border-radius: 0px; selection-background-color: #e3f0fd; selection-color: #1565c0;
            outline: none;
        }

        /* ===== 表格：白底直角工业风 ===== */
        QTableWidget {
            background: #ffffff;
            border: 1px solid #d0d7e0;
            border-radius: 4px; gridline-color: #e4e8ee;
            selection-background-color: #e3f0fd; selection-color: #1f2937;
            alternate-background-color: #f7f9fc;
        }
        QTableWidget::item { padding: 8px; border: none; }
        QHeaderView::section {
            background: #e8edf3; color: #334155;
            font-weight: bold; border: none;
            border-right: 1px solid #d0d7e0;
            border-bottom: 2px solid #1565c0;
            padding: 10px 6px;
        }

        /* ===== 卡片（设备卡片网格、KPI卡片等） ===== */
        QFrame[glassCard="true"] {
            background: #ffffff;
            border-radius: 6px;
            border: 1px solid #d0d7e0;
        }
        QFrame[glassCard="true"]:hover {
            background: #ffffff;
            border: 1px solid #1565c0;
        }

        /* ===== 菜单与消息框 ===== */
        QMenu { background: #ffffff; border: 1px solid #c0c8d4; border-radius: 4px; padding: 4px; }
        QMenu::item { padding: 8px 24px; border-radius: 2px; }
        QMenu::item:selected { background: #e3f0fd; color: #1565c0; }
        QMessageBox { background: #ffffff; }
        QMessageBox QLabel { color: #1f2937; }

        /* ===== 登录页：深蓝纯色背景 ===== */
        #LoginWindow {
            background: #1b2a41;
        }
        #LoginCard {
            background: #ffffff;
            border-radius: 6px;
            border: 1px solid #2a3f5f;
        }
        #LoginTitle { font-size: 20px; font-weight: bold; color: #1f2937; }
        #LoginSubtitle { color: #6b7b8f; font-size: 13px; }

        /* ===== 统计页图表 ===== */
        QChartView {
            background: #ffffff;
            border: 1px solid #d0d7e0;
            border-radius: 4px;
        }

        /* ===== 状态栏 ===== */
        QStatusBar {
            background: #ffffff;
            border-top: 1px solid #d0d7e0;
            color: #5a6a7e; font-size: 12px;
        }
        QStatusBar::item { border: none; }

        /* ===== 滚动区域 ===== */
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical {
            background: #e4e8ee; width: 10px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #a8b5c7; border-radius: 0px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #1565c0; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal {
            background: #e4e8ee; height: 10px; margin: 0;
        }
        QScrollBar::handle:horizontal {
            background: #a8b5c7; border-radius: 0px; min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover { background: #1565c0; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

        /* ===== 列表控件 ===== */
        QListWidget {
            background: #ffffff;
            border: 1px solid #d0d7e0;
            border-radius: 4px;
            outline: none;
        }
        QListWidget::item { padding: 6px 10px; }
        QListWidget::item:selected { background: #e3f0fd; color: #1565c0; }
        QListWidget::item:hover { background: #f0f4f9; }
    )"));
}

// 状态中文 -> 颜色（沉稳工业配色）
QColor statusColor(const QString& s)
{
    if (s == QStringLiteral("可用") || s == QStringLiteral("已归还") || s == QStringLiteral("已执行"))
        return QColor(46, 125, 50);      // 深绿
    if (s == QStringLiteral("已借出"))
        return QColor(230, 81, 0);       // 深橙
    if (s == QStringLiteral("维护中"))
        return QColor(84, 110, 122);     // 蓝灰
    if (s == QStringLiteral("待审批") || s == QStringLiteral("未执行"))
        return QColor(245, 127, 23);     // 琥珀
    if (s == QStringLiteral("已通过"))
        return QColor(21, 101, 192);     // 科技蓝
    if (s == QStringLiteral("已拒绝"))
        return QColor(198, 40, 40);      // 深红
    if (s == QStringLiteral("已取消"))
        return QColor(117, 117, 117);    // 灰
    return QColor(31, 41, 55);           // 兜底深灰
}

// "● 状态" 彩色单元格
QTableWidgetItem* makeStatusItem(const QString& statusName)
{
    auto *item = new QTableWidgetItem(QStringLiteral("● ") + statusName);
    const QColor c = statusColor(statusName);
    item->setForeground(QBrush(c));
    QFont f = item->font();
    f.setBold(true);
    item->setFont(f);
    item->setTextAlignment(Qt::AlignCenter);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 状态列禁止编辑
    return item;
}

}   // namespace Theme
