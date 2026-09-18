// ============================================================
// 文件说明：Theme.cpp —— 全局主题（“皮肤”）具体实现
// 这里写 Theme.h 里声明的每个函数的具体代码
// ============================================================

// 先引入自己的头文件
#include "Theme.h"

// 引入需要用到的 Qt 类
#include <QApplication>     // qApp 全局指针，指向 QApplication 对象
#include <QFont>            // QFont：字体类，设置字号、字体名
#include <QColor>           // QColor：颜色类
#include <QBrush>           // QBrush：画刷，用来设置单元格的填充色/文字色
#include <QTableWidgetItem> // 表格单元格类
#include <QHeaderView>      // 表头类

// 再次进入 Theme 命名空间
namespace Theme {

// ============================================================
// apply() 函数：应用全局主题
// 这个函数做两件事：
//   1. 设置全局字体（微软雅黑 10 号）
//   2. 设置全局 QSS 样式表（Qt 版的 CSS，控制所有控件的外观）
// ============================================================
void apply()
{
    // QFont 是字体对象
    // QStringLiteral 是 Qt 的宏，把中文字符串转成 Unicode，防止乱码
    QFont font(QStringLiteral("Microsoft YaHei UI"));
    font.setPointSize(10);   // 设置字号为 10 磅

    // qApp 是 Qt 提供的全局指针，指向当前的 QApplication 对象
    // setFont() 把这个字体设为整个程序的默认字体
    qApp->setFont(font);

    // setStyleSheet() 设置全局样式表
    // QSS（Qt Style Sheet）语法和 CSS 几乎一样
    // R"(...)" 是 C++11 的原始字符串，里面的换行、空格都会原样保留
    // 好处：写多行 CSS 不用每行加换行符
    qApp->setStyleSheet(QStringLiteral(R"(
        /* ===== 全局基调：浅灰蓝纯色背景 ===== */
        /* QMainWindow 是主窗口基类，设置它的背景色 */
        QMainWindow {
            background: #eef1f5;
        }
        /* #MainContent 是对象名（objectName），代码里 setObjectName("MainContent") 设置过 */
        /* CSS 里 #xxx 就是按 id 选元素，和网页 CSS 一样 */
        #MainContent {
            background: #eef1f5;
        }
        /* QDialog 是对话框基类 */
        QDialog {
            background: #f5f7fa;
        }
        /* QWidget 是所有控件的基类，设置默认文字颜色和字号 */
        QWidget { color: #1f2937; font-size: 14px; }

        /* ===== 侧边导航：深蓝实色 ===== */
        /* #Sidebar 是左侧导航栏的对象名 */
        #Sidebar {
            background: #1b2a41;  /* 深藏蓝色背景 */
            border: none;
            border-right: 1px solid #142033;  /* 右边一条深色分割线 */
        }
        /* #AppTitle 是顶部系统标题的对象名 */
        #AppTitle {
            font-size: 14px; font-weight: bold; color: #ffffff;
            padding: 22px 8px 16px 20px;  /* 内边距：上 右 下 左 */
        }
        /* #NavList 是导航列表的对象名 */
        #NavList {
            background: transparent; border: none; outline: none;
            font-size: 14px; color: #a8b5c7;
        }
        /* ::item 是列表里的每一项，类似 CSS 的 .item */
        #NavList::item {
            padding: 11px 16px; margin: 2px 8px; border-radius: 4px;
            background: transparent;
            border-left: 3px solid transparent;  /* 左边框透明，选中时变蓝 */
        }
        /* :hover 是鼠标悬停时的样式，和网页 CSS 一样 */
        #NavList::item:hover { background: rgba(255, 255, 255, 0.08); color: #ffffff; }
        /* :selected 是选中项的样式 */
        #NavList::item:selected {
            background: #1565c0;  /* 科技蓝背景 */
            color: #ffffff; font-weight: bold;
            border-left: 3px solid #42a5f5;  /* 左边亮蓝色竖条 */
        }
        /* #SidebarFooter 是侧栏底部用户信息区 */
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
        /* hover 时变红色，提示危险操作 */
        #LogoutButton:hover { background: #c62828; color: #ffffff; border-color: #c62828; }

        /* ===== 顶栏：白色实色 ===== */
        #TopBar {
            background: #ffffff;
            border-bottom: 1px solid #d0d7e0;
        }
        #PageTitle { font-size: 20px; font-weight: bold; color: #1f2937; }
        #TopBar QLabel { color: #6b7b8f; font-size: 13px; }

        /* ===== 按钮：工业风格小矩形 ===== */
        /* QPushButton 是所有按钮的默认样式 */
        QPushButton {
            background: #ffffff; color: #334155;
            border: 1px solid #c0c8d4;
            border-radius: 4px; padding: 8px 20px;
        }
        QPushButton:hover {
            border-color: #1565c0; color: #1565c0;
            background: #f0f6ff;
        }
        /* :pressed 是按钮被按下时的样式 */
        QPushButton:pressed { background: #d6e6f7; border-color: #0d47a1; color: #0d47a1; }
        /* :disabled 是按钮禁用时的样式 */
        QPushButton:disabled { color: #a0aab8; border-color: #dde2e9; background: #f5f7fa; }
        /* [primary="true"] 是按属性选元素 */
        /* 代码里 setProperty("primary", true) 设置过的按钮才是这个样式 */
        /* 主按钮：蓝底白字，突出最重要的操作 */
        QPushButton[primary="true"] {
            background: #1565c0;
            color: #ffffff; border: 1px solid #1565c0; font-weight: bold;
        }
        QPushButton[primary="true"]:hover {
            background: #0d47a1; border-color: #0d47a1; color: #ffffff;
        }
        QPushButton[primary="true"]:pressed { background: #0a3a85; border-color: #0a3a85; color: #ffffff; }

        /* ===== 输入控件 ===== */
        /* QLineEdit 是单行输入框，QComboBox 是下拉框 */
        /* QDateTimeEdit 是日期时间选择器，QSpinBox 是数字输入框 */
        QLineEdit, QComboBox, QDateTimeEdit, QSpinBox {
            background: #ffffff;
            border: 1px solid #c0c8d4;
            border-radius: 4px; padding: 7px 12px;
            selection-background-color: #1565c0;  /* 选中文字时的背景色 */
            selection-color: #ffffff;             /* 选中文字时的文字色 */
            color: #1f2937;
        }
        /* :focus 是控件获得焦点时（点进去了）的样式 */
        QLineEdit:focus, QComboBox:focus, QDateTimeEdit:focus, QSpinBox:focus {
            border: 1px solid #1565c0;
        }
        /* 禁用时变灰 */
        QLineEdit:disabled, QComboBox:disabled, QDateTimeEdit:disabled, QSpinBox:disabled {
            background: #eef1f5; color: #8a94a6;
        }
        /* QComboBox::drop-down 是下拉框右边那个小箭头按钮 */
        QComboBox::drop-down { border: none; width: 26px; border-left: 1px solid #d0d7e0; }
        /* 下拉框弹出的列表 */
        QComboBox QAbstractItemView {
            background: #ffffff; border: 1px solid #c0c8d4;
            border-radius: 0px; selection-background-color: #e3f0fd; selection-color: #1565c0;
            outline: none;
        }

        /* ===== 表格：白底直角工业风 ===== */
        QTableWidget {
            background: #ffffff;
            border: 1px solid #d0d7e0;
            border-radius: 4px; gridline-color: #e4e8ee;  /* 网格线颜色 */
            selection-background-color: #e3f0fd; selection-color: #1f2937;
            alternate-background-color: #f7f9fc;  /* 交替行颜色（斑马纹） */
        }
        QTableWidget::item { padding: 8px; border: none; }
        /* 表头 */
        QHeaderView::section {
            background: #e8edf3; color: #334155;
            font-weight: bold; border: none;
            border-right: 1px solid #d0d7e0;
            border-bottom: 2px solid #1565c0;  /* 表头底部蓝色粗线 */
            padding: 10px 6px;
        }

        /* ===== 卡片（设备卡片网格、KPI卡片等） ===== */
        /* [glassCard="true"] 是带卡片属性的控件 */
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
        /* 垂直滚动条 */
        QScrollBar:vertical {
            background: #e4e8ee; width: 10px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #a8b5c7; border-radius: 0px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #1565c0; }
        /* add-line/sub-line 是滚动条上下箭头，这里隐藏了（高度为0） */
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        /* 水平滚动条同理 */
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

// ============================================================
// statusColor() 函数：把中文状态转成对应的颜色
// 比如传“可用”返回绿色，传“已借出”返回橙色
// 参数 const QString& s：输入的状态字符串，const 表示不会修改它，& 表示引用传参（效率高）
// 返回值：QColor 颜色对象
// ============================================================
QColor statusColor(const QString& s)
{
    // if-else 链：依次判断，匹配到就返回
    if (s == QStringLiteral("可用") || s == QStringLiteral("已归还") || s == QStringLiteral("已执行"))
        return QColor(46, 125, 50);      // 深绿色，代表好状态
    if (s == QStringLiteral("已借出"))
        return QColor(230, 81, 0);       // 深橙色，代表借出中
    if (s == QStringLiteral("维护中"))
        return QColor(84, 110, 122);     // 蓝灰色，代表停机维护
    if (s == QStringLiteral("待审批") || s == QStringLiteral("未执行"))
        return QColor(245, 127, 23);     // 琥珀色，代表等待中
    if (s == QStringLiteral("已通过"))
        return QColor(21, 101, 192);     // 科技蓝，代表已批准
    if (s == QStringLiteral("已拒绝"))
        return QColor(198, 40, 40);      // 深红色，代表失败
    if (s == QStringLiteral("已取消"))
        return QColor(117, 117, 117);    // 灰色，代表取消了
    return QColor(31, 41, 55);           // 兜底：深灰色，其他情况都用这个
}

// ============================================================
// makeStatusItem() 函数：创建一个“● 状态”样式的表格单元格
// 参数 statusName：状态文字，比如“可用”
// 返回值：QTableWidgetItem* 指针，指向新建的单元格
// ============================================================
QTableWidgetItem* makeStatusItem(const QString& statusName)
{
    // new 创建一个表格单元格，内容是“● 可用”（前面加个圆点）
    auto *item = new QTableWidgetItem(QStringLiteral("● ") + statusName);

    // 调用上面的 statusColor() 获取这个状态对应的颜色
    const QColor c = statusColor(statusName);

    // setForeground 设置前景色（文字颜色）
    // QBrush 是画刷，用来填充颜色，这里用文字颜色
    item->setForeground(QBrush(c));

    // 获取当前字体，设为粗体，再设回去
    QFont f = item->font();
    f.setBold(true);
    item->setFont(f);

    // 设置文字居中对齐（水平居中 + 垂直居中）
    item->setTextAlignment(Qt::AlignCenter);

    // 设置单元格标志：去掉 ItemIsEditable（不可编辑）
    // flags() 返回当前所有标志，& ~Qt::ItemIsEditable 就是“把可编辑这个标志关掉”
    // 位运算：& 是按位与，~ 是按位取反
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    return item;  // 返回创建好的单元格指针
}

}   // namespace Theme 结束
