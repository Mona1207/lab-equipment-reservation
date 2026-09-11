#ifndef LOGINWINDOW_H   // 头文件保护宏开始
#define LOGINWINDOW_H   // 定义保护宏

#include <QWidget>      // 窗口基类

class QComboBox;        // 前向声明：账号下拉框
class QLineEdit;        // 前向声明：密码输入框
class User;             // 前向声明：用户类

// =============================================================
// LoginWindow —— 界面组(D)
// 下拉选择演示账号（携带核心用户id），输入密码后交给 AppContext 校验，
// 校验通过发出 loggedIn 信号，由 main 打开主窗口。
// 演示账号：张三(学生2001)/王老师(教师3001)/老王(管理员1)，密码均为 123
// =============================================================
class LoginWindow : public QWidget   // 登录窗口，继承普通控件窗口
{
    Q_OBJECT                  // Qt 元对象宏：信号槽必备
public:
    explicit LoginWindow(QWidget *parent = nullptr);   // 构造函数

signals:
    void loggedIn();          // 登录成功信号（当前用户已存进 AppContext，无需再带指针）

private slots:
    void onLogin();           // “登录”按钮点击槽函数

private:
    QComboBox *m_roleBox;     // 账号（角色）下拉框
    QLineEdit *m_pwdEdit;     // 密码输入框
};

#endif // LOGINWINDOW_H  // 头文件保护结束
