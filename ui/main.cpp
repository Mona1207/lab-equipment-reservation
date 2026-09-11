#include <QApplication>   // Qt GUI 程序基础类，负责事件循环
#include <QDebug>         // 控制台调试输出

#include "AppContext.h"   // 界面适配层（持有核心、种子数据）
#include "LoginWindow.h"  // 登录窗口（界面组 D）
#include "MainWindow.h"   // 主窗口（界面组 D）
#include "Theme.h"        // 全局主题（字体 + QSS）

// =============================================================
// 实验室设备预约管理系统 —— 界面程序入口（界面组 D）
// 本入口只负责界面流程：灌入种子数据 -> 登录 -> 主界面 -> 退出可重登；
// 所有业务算法都在静态库 labres_core 中，本文件不含任何业务逻辑。
// 构建时用 -DBUILD_DEMO=OFF 关闭算法组的命令行 demo，由本文件提供 GUI main。
// =============================================================
int main(int argc, char *argv[])   // 程序入口
{
    QApplication app(argc, argv);  // 创建 Qt 应用对象，一个 GUI 程序只能有一个

    Theme::apply();                // 应用全局视觉主题（字体 + QSS 样式表）

    qDebug() << "LabEquipment UI started";   // 控制台启动标志

    AppContext::get().seed();      // 灌入演示用户/设备（正式版将由数据库组替换）

    LoginWindow login;             // 在栈上创建登录窗口

    // 登录成功：按当前登录用户新建主界面
    QObject::connect(&login, &LoginWindow::loggedIn, [&login] {
        auto *win = new MainWindow();             // 堆上新建主窗口（可换账号重复创建）
        win->setAttribute(Qt::WA_DeleteOnClose);  // 关闭即自动释放，防内存泄漏

        // 主界面点“退出登录”：清空登录态 -> 关主窗 -> 回登录窗，可换账号再进
        QObject::connect(win, &MainWindow::logoutRequested, [&login, win] {
            AppContext::get().logout();   // 清空当前登录用户
            win->close();                 // 关闭主窗口
            login.show();                 // 重新显示登录窗
        });

        login.hide();        // 隐藏登录窗
        win->show();         // 显示主界面
    });

    login.show();            // 启动首先显示登录窗
    return app.exec();       // 进入 Qt 事件循环；退出时返回退出码
}
