// ============================================================
// 文件说明：LoginWindow.h —— 登录窗口头文件
// 登录窗口：程序刚打开时你看到的那个窗口，选账号、输密码、点登录
// ============================================================

#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

// QWidget 是所有 Qt 窗口/控件的基类
// 就像“动物”是猫和狗的基类一样，QWidget 是按钮、输入框、窗口的爹
#include <QWidget>

// 前向声明：告诉编译器这些类存在，但不引入完整定义
class QComboBox;        // QComboBox：下拉选择框
class QLineEdit;        // QLineEdit：单行文本输入框
class User;             // User：用户类（核心层定义的）

// ============================================================
// LoginWindow 类：登录窗口
// public QWidget 表示“继承自 QWidget”，也就是 LoginWindow 是一种 QWidget
// 继承：子类自动拥有父类的所有功能，然后再加自己的功能
// ============================================================
class LoginWindow : public QWidget
{
    // Q_OBJECT 是 Qt 的元对象系统宏
    // 只要这个类要用“信号和槽”，就必须加这个宏
    // 它会让 Qt 的 moc（元对象编译器）自动生成一些代码
    Q_OBJECT

public:  // public：公开的，外部可以调用
    // 构造函数：创建对象时自动调用
    // QWidget *parent = nullptr：父窗口指针，默认空（没有父窗口，就是独立窗口）
    explicit LoginWindow(QWidget *parent = nullptr);

signals:  // signals：信号（Qt 特有）
    // loggedIn() 是一个信号
    // 登录成功时，这个信号会“发射”（emit）出去
    // 外部用 connect() 连接这个信号，就能在登录成功时做事情
    void loggedIn();

private slots:  // private slots：私有槽函数（只能内部用）
    // onLogin() 是“槽函数”：登录按钮被点击时，自动调用这个函数
    // 槽函数就是“信号触发后要执行的函数”
    void onLogin();

private:  // private：私有，外部不能直接访问
    // 成员变量：下拉框和密码输入框
    // 它们是指针，因为 Qt 的控件都是用 new 创建的（堆上对象）
    QComboBox *m_roleBox;     // 账号下拉框
    QLineEdit *m_pwdEdit;     // 密码输入框
};

#endif // LOGINWINDOW_H
