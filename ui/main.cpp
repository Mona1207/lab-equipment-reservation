// ============================================================
// 文件说明：程序入口文件 main.cpp
// 这是整个程序的“起点”，操作系统最先执行这里的 main() 函数
// 你可以把它理解为：你打开微信，最先运行的就是微信的 main()
// ============================================================

// #include 是 C++ 的“引入头文件”语法
// 就像你做饭前要先买菜：用别人已经写好的工具，不用自己从头造轮子
// 下面三行就是引入 Qt 框架提供的现成工具
#include <QApplication>   // QApplication 是 Qt 的“大管家”，负责管理整个程序的运行
                          // 任何 Qt 图形程序都必须有且只有一个 QApplication 对象
#include <QDebug>         // qDebug() 是用来往控制台打印调试信息的，类似 Python 的 print

// 下面三行是引入我们自己写的头文件
// " 引号表示“我自己写的文件”，<> 尖括号表示“别人写的库文件”
#include "AppContext.h"   // AppContext：整个程序的“全局大管家”，管理用户登录、数据等
#include "LoginWindow.h"  // LoginWindow：登录窗口，就是程序刚打开时你看到的那个窗口
#include "MainWindow.h"   // MainWindow：登录成功后进入的主界面
#include "Theme.h"        // Theme：全局主题，就是程序的“皮肤/样式表”

// ============================================================
// main() 函数：C++ 程序的唯一入口
// 操作系统运行程序时，第一个调用的就是这个函数
// 参数 argc 是命令行参数个数，argv 是参数字符串数组（我们这个 GUI 程序用不上）
// ============================================================
int main(int argc, char *argv[])
{
    // 第 1 步：创建 QApplication 对象
    // 它负责：事件循环、字体、样式、剪贴板、鼠标键盘事件分发
    // 一个 GUI 程序有且只能有一个 QApplication，否则会报错
    QApplication app(argc, argv);

    // 第 2 步：应用全局视觉主题
    // Theme::apply() 会设置整个程序的字体、颜色、按钮样式等
    // 就像你给手机换主题，一键换所有图标颜色
    Theme::apply();

    // 第 3 步：往控制台打印一行字，告诉我们程序启动了
    // qDebug() 类似 printf/print，输出到 Debug 控制台
    qDebug() << "LabEquipment UI started";

    // 第 4 步：灌入演示数据
    // 正式版这里会从数据库读取数据；演示版我们手动造几个用户和设备
    AppContext::get().seed();

    // 第 5 步：创建登录窗口
    // LoginWindow login; 这里 login 是“栈上对象”，程序结束时自动销毁
    // 栈上对象：局部变量，函数结束自动释放内存，不需要你手动 delete
    LoginWindow login;

    // 第 6 步：连接信号和槽（Qt 最核心的机制）
    // 信号（signal）：某个事情发生了，比如“登录成功了”
    // 槽（slot）：事情发生后要执行的函数，比如“打开主窗口”
    // connect() 就是把“信号”和“槽”绑在一起：A 发生了，就执行 B
    //
    // 这里的意思：当 login 窗口发出 loggedIn 信号（登录成功）时，执行后面的 lambda
    QObject::connect(&login, &LoginWindow::loggedIn, [&login] {
        // lambda 是 C++11 的匿名函数，[] 里写捕获的外部变量
        // [&login] 表示按引用捕获 login，这样 lambda 内部能访问它

        // 创建主窗口（堆上对象，用 new 创建）
        // 堆上对象：需要你手动管理内存，这里设了 DeleteOnClose，关闭自动释放
        auto *win = new MainWindow();

        // setAttribute 给窗口设置一个属性
        // WA_DeleteOnClose：窗口关闭时自动 delete，防止内存泄漏
        win->setAttribute(Qt::WA_DeleteOnClose);

        // 连接主窗口的 logoutRequested 信号（用户点了退出登录）
        QObject::connect(win, &MainWindow::logoutRequested, [&login, win] {
            AppContext::get().logout();   // 清空当前登录用户
            win->close();                 // 关闭主窗口
            login.show();                 // 重新显示登录窗口，可以换账号再进
        });

        login.hide();        // 隐藏登录窗口（不是销毁，只是看不见）
        win->show();         // 显示主窗口
    });

    // 第 7 步：显示登录窗口
    login.show();

    // 第 8 步：进入 Qt 事件循环
    // app.exec() 会阻塞在这里，一直等待用户操作（点击按钮、输入文字等）
    // 直到用户关闭所有窗口，exec() 才返回，程序结束
    // 你可以把它理解为：while(用户没退出) { 处理用户操作; }
    return app.exec();
}
