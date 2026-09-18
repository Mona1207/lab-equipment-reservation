// ============================================================
// 文件说明：Theme.h —— 全局主题（“皮肤”）头文件
// 这个文件只声明“有什么功能”，具体实现写在 Theme.cpp 里
// 头文件（.h）就像菜单，源文件（.cpp）就像厨房做菜
// ============================================================

// 头文件保护宏：防止同一个头文件被重复包含多次
// 就像门牌号：ifndef 意思是“如果没定义过 THEME_H”
// define 就是“定义一下，标记我已经来过了”
// endif 是“保护结束”
#ifndef THEME_H
#define THEME_H

// 引入 QString 类（Qt 的字符串类型）
// QString 比 C++ 标准的 std::string 更好用，支持中文、Unicode
#include <QString>

// 前向声明：告诉编译器“有这个类存在”，但不引入它的完整定义
// 好处：减少编译时间，就像你知道张三这个人，但不需要把他全家都请来
class QColor;           // QColor：颜色类，比如红、绿、蓝
class QTableWidgetItem; // QTableWidgetItem：表格里的一个单元格

// namespace（命名空间）：把相关的东西打包在一起，防止名字冲突
// 比如你有两个叫“add”的函数，Theme::add 和 Math::add 就不会打架
namespace Theme {

// 函数声明：告诉外界“我有这个功能”，但具体怎么做看 .cpp
void apply();   // 应用全局字体和样式表，在程序启动时调用一次

// 给一个中文状态（比如“可用”、“已借出”），返回对应的颜色
// 比如传“可用”返回绿色，传“已借出”返回橙色
QColor statusColor(const QString& statusName);

// 创建一个带彩色圆点的表格单元格
// 比如返回的单元格内容是“● 可用”，圆点是绿色的，字也是绿色的
QTableWidgetItem* makeStatusItem(const QString& statusName);

}   // namespace Theme 结束

#endif // THEME_H  // 头文件保护结束
