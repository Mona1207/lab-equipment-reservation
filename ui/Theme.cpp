#include "Theme.h"     // 对应头文件
#include <QApplication> // 应用对象（设全局字体/样式表）
#include <QFont>        // 字体
#include <QColor>       // 颜色
#include <QBrush>       // 画刷（单元格文字色）
#include <QTableWidgetItem> // 表格单元格
#include <QHeaderView>  // 表头

namespace Theme {

// 应用全局主题：雅黑字体 + 全局 QSS
// 设计风格：软玻璃拟态（glassmorphism）+ 粉橙蓝柔和渐变光晕
void apply()
{
    QFont font(QStringLiteral("Microsoft YaHei UI"));
    font.setPointSize(10);
    qApp->setFont(font);

    qApp->setStyleSheet(QStringLiteral(R"(
        /* ===== 全局基调：粉→橙→蓝 柔和渐变光晕背景 ===== */
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ffd6e0, stop:0.3 #ffc4d6, stop:0.55 #ffb8a0,
                stop:0.8 #c4d8ff, stop:1 #d6e4ff);
        }
        #MainContent {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ffd6e0, stop:0.3 #ffc4d6, stop:0.55 #ffb8a0,
                stop:0.8 #c4d8ff, stop:1 #d6e4ff);
        }
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #fff0f5, stop:1 #eef2ff);
        }
        QWidget { color: #2b3445; font-size: 14px; }

        /* ===== 侧边导航：浅色半透明磨砂 ===== */
        #Sidebar {
            background: rgba(255, 255, 255, 0.55);
            border: none;
            border-right: 1px solid rgba(255, 255, 255, 0.6);
        }
        #AppTitle {
            font-size: 18px; font-weight: bold; color: #2b3445;
            padding: 22px 8px 16px 20px;
            letter-spacing: 1px;
        }
        #NavList {
            background: transparent; border: none; outline: none;
            font-size: 14px; color: #5a6b85;
        }
        #NavList::item {
            padding: 11px 16px; margin: 4px 12px; border-radius: 12px;
            background: transparent;
        }
        #NavList::item:hover { background: rgba(255, 255, 255, 0.6); color: #2b5fd9; }
        #NavList::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6c7bff, stop:1 #a86cf5);
            color: #ffffff; font-weight: bold;
        }
        #SidebarFooter {
            background: rgba(255, 255, 255, 0.4);
            border-top: 1px solid rgba(255, 255, 255, 0.5);
        }
        #SidebarFooter QLabel { color: #5a6b85; font-size: 12px; }
        #UserNameLabel { color: #2b3445 !important; font-size: 14px; font-weight: bold; }
        #RoleBadge {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6c7bff, stop:1 #a86cf5);
            color: #ffffff; border-radius: 10px;
            padding: 3px 12px; font-size: 11px; font-weight: bold;
        }
        #LogoutButton {
            background: rgba(255, 255, 255, 0.7);
            color: #5a6b85; border: 1px solid rgba(180, 190, 210, 0.4);
            border-radius: 10px; padding: 8px 0; font-size: 13px;
        }
        #LogoutButton:hover { background: #ff6b7a; color: #ffffff; border-color: #ff6b7a; }

        /* ===== 顶栏：半透明白 ===== */
        #TopBar {
            background: rgba(255, 255, 255, 0.4);
            border-bottom: 1px solid rgba(255, 255, 255, 0.5);
        }
        #PageTitle { font-size: 22px; font-weight: bold; color: #2b3445; }
        #TopBar QLabel { color: #6b7688; font-size: 13px; }

        /* ===== 按钮：大圆角胶囊 ===== */
        QPushButton {
            background: rgba(255, 255, 255, 0.85); color: #3a4a63;
            border: 1px solid rgba(180, 190, 210, 0.35);
            border-radius: 12px; padding: 9px 22px;
        }
        QPushButton:hover {
            border-color: #6c7bff; color: #6c7bff;
            background: rgba(108, 123, 255, 0.08);
        }
        QPushButton:pressed { background: rgba(108, 123, 255, 0.15); }
        QPushButton:disabled { color: #b6bfce; border-color: #e3e8f0; background: rgba(255,255,255,0.5); }
        QPushButton[primary="true"] {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6c7bff, stop:1 #a86cf5);
            color: #ffffff; border: none; font-weight: bold;
        }
        QPushButton[primary="true"]:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #5a6bef, stop:1 #945ce5);
        }
        QPushButton[primary="true"]:pressed { background: #4a58d9; }

        /* ===== 输入控件 ===== */
        QLineEdit, QComboBox, QDateTimeEdit, QSpinBox {
            background: rgba(255, 255, 255, 0.9);
            border: 1px solid rgba(180, 190, 210, 0.35);
            border-radius: 10px; padding: 8px 14px;
            selection-background-color: #6c7bff;
        }
        QLineEdit:focus, QComboBox:focus, QDateTimeEdit:focus, QSpinBox:focus {
            border-color: #6c7bff;
        }
        QComboBox::drop-down { border: none; width: 26px; }
        QComboBox QAbstractItemView {
            background: #ffffff; border: 1px solid #dbe3f0;
            border-radius: 10px; selection-background-color: #eef0ff; selection-color: #2b3445;
        }

        /* ===== 表格：白底圆角 ===== */
        QTableWidget {
            background: rgba(255, 255, 255, 0.92);
            border: 1px solid rgba(255, 255, 255, 0.7);
            border-radius: 16px; gridline-color: #eef1f6;
            selection-background-color: #eef0ff; selection-color: #2b3445;
            alternate-background-color: rgba(248, 250, 253, 0.8);
        }
        QTableWidget::item { padding: 8px; border: none; }
        QHeaderView::section {
            background: rgba(250, 250, 255, 0.9); color: #6b7688;
            font-weight: bold; border: none;
            border-bottom: 2px solid rgba(180, 190, 210, 0.15);
            padding: 10px 6px;
        }

        /* ===== 玻璃卡片（设备卡片网格、KPI卡片等） ===== */
        QFrame[glassCard="true"] {
            background: rgba(255, 255, 255, 0.75);
            border-radius: 18px;
            border: 1px solid rgba(255, 255, 255, 0.8);
        }
        QFrame[glassCard="true"]:hover {
            background: rgba(255, 255, 255, 0.92);
            border: 1px solid rgba(108, 123, 255, 0.35);
        }

        /* ===== 菜单与消息框 ===== */
        QMenu { background: #ffffff; border: 1px solid #e3e8f0; border-radius: 12px; padding: 6px; }
        QMenu::item { padding: 8px 24px; border-radius: 8px; }
        QMenu::item:selected { background: #eef0ff; color: #6c7bff; }
        QMessageBox { background: #ffffff; }
        QMessageBox QLabel { color: #2b3445; }

        /* ===== 登录页 ===== */
        #LoginWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ffd6e0, stop:0.4 #ffc4d6, stop:0.7 #ffb8a0,
                stop:1 #c4d8ff);
        }
        #LoginCard {
            background: rgba(255, 255, 255, 0.88);
            border-radius: 20px;
            border: 1px solid rgba(255, 255, 255, 0.6);
        }
        #LoginTitle { font-size: 22px; font-weight: bold; color: #2b3445; }
        #LoginSubtitle { color: #8a94a6; font-size: 13px; }

        /* ===== 统计页图表 ===== */
        QChartView {
            background: rgba(255, 255, 255, 0.85);
            border: 1px solid rgba(255, 255, 255, 0.7);
            border-radius: 16px;
        }

        /* ===== 状态栏 ===== */
        QStatusBar {
            background: rgba(255, 255, 255, 0.45);
            border-top: 1px solid rgba(255, 255, 255, 0.5);
            color: #5a6b85; font-size: 12px;
        }
        QStatusBar::item { border: none; }

        /* ===== 滚动区域 ===== */
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical {
            background: transparent; width: 8px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(180, 190, 210, 0.5); border-radius: 4px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: rgba(108, 123, 255, 0.5); }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )"));
}

// 状态中文 -> 颜色
QColor statusColor(const QString& s)
{
    if (s == QStringLiteral("可用") || s == QStringLiteral("已归还"))
        return QColor(34, 154, 22);      // 绿
    if (s == QStringLiteral("已借出"))
        return QColor(230, 126, 0);      // 橙
    if (s == QStringLiteral("维护中"))
        return QColor(120, 132, 150);    // 灰蓝
    if (s == QStringLiteral("待审批"))
        return QColor(202, 144, 0);      // 琥珀
    if (s == QStringLiteral("已通过"))
        return QColor(108, 123, 255);    // 主紫蓝
    if (s == QStringLiteral("已拒绝"))
        return QColor(220, 76, 92);      // 红
    if (s == QStringLiteral("已取消"))
        return QColor(150, 150, 150);    // 灰
    return QColor(43, 52, 69);           // 兜底深灰
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
