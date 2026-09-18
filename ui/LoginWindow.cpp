// ============================================================
// 文件说明：LoginWindow.cpp —— 登录窗口具体实现
// ============================================================

#include "LoginWindow.h"   // 先引入自己的头文件
#include "AppContext.h"    // 引入全局上下文（登录校验用）

// 引入需要的 Qt 控件类
#include <QVBoxLayout>     // QVBoxLayout：垂直布局（从上到下排列）
#include <QFormLayout>     // QFormLayout：表单布局（左边标签，右边输入框）
#include <QComboBox>       // 下拉框类
#include <QLineEdit>       // 单行输入框类
#include <QPushButton>     // 按钮类
#include <QMessageBox>     // 弹窗提示类（比如“登录失败”提示框）
#include <QLabel>          // 标签类（显示文字）

// ============================================================
// 构造函数：创建登录窗口时自动执行
// LoginWindow::LoginWindow 表示“这是 LoginWindow 类的构造函数”
// QWidget(parent) 是“调用父类构造函数”，把 parent 传给 QWidget
// ============================================================
LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)      // 初始化列表：先调用父类构造函数
{
    // setObjectName() 设置对象名，QSS 样式表里可以用 #LoginWindow 来选中它
    setObjectName(QStringLiteral("LoginWindow"));

    // setWindowTitle() 设置窗口标题栏的文字
    setWindowTitle(QStringLiteral("实验室设备预约管理系统 - 登录"));

    // setFixedSize() 设置窗口固定大小（宽420，高380），用户不能拖拽改变大小
    setFixedSize(420, 380);

    // ---- 第 1 步：创建中央白色卡片 ----
    // 登录窗口是深色背景，中间放一个白色的卡片，卡片里放登录表单
    auto *card = new QWidget(this);   // 创建卡片，parent 是 this（登录窗口自己）
    card->setObjectName(QStringLiteral("LoginCard"));  // 供 QSS 选中
    card->setFixedWidth(320);          // 卡片固定宽度 320 像素

    // 创建标题标签：大标题“🔬 实验室设备预约”
    auto *title = new QLabel(QStringLiteral("🔬 实验室设备预约"), card);
    title->setObjectName(QStringLiteral("LoginTitle"));

    // 创建副标题标签：“请选择账号并输入密码登录”
    auto *subtitle = new QLabel(QStringLiteral("请选择账号并输入密码登录"), card);
    subtitle->setObjectName(QStringLiteral("LoginSubtitle"));

    // ---- 第 2 步：创建账号下拉框 ----
    m_roleBox = new QComboBox(card);

    // addItem() 往下拉框里加选项
    // 第一个参数是显示文字，第二个参数是“隐藏数据”（用户id）
    // 用户看到的是“张三（学生）”，但我们代码里能拿到对应的 id 2001
    m_roleBox->addItem(QStringLiteral("张三（学生）"), 2001);     // 学生账号
    m_roleBox->addItem(QStringLiteral("王老师（教师）"), 3001);   // 教师账号
    m_roleBox->addItem(QStringLiteral("老王（管理员）"), 1);      // 管理员账号

    // ---- 第 3 步：创建密码输入框 ----
    m_pwdEdit = new QLineEdit(card);

    // setEchoMode(Password) 让输入的内容显示为圆点（●●●），不显示明文
    m_pwdEdit->setEchoMode(QLineEdit::Password);

    // 演示方便，预填密码 123，用户不用每次都输
    m_pwdEdit->setText(QStringLiteral("123"));

    // setPlaceholderText() 是输入框为空时显示的灰色提示文字
    m_pwdEdit->setPlaceholderText(QStringLiteral("密码（演示：123）"));

    // ---- 第 4 步：创建登录按钮 ----
    QPushButton *btn = new QPushButton(QStringLiteral("登  录"), card);

    // setProperty 设置属性，QSS 里 [primary="true"] 会选中它，显示为蓝色主按钮
    btn->setProperty("primary", true);

    // setMinimumHeight 设置最小高度 38 像素，按钮大一点好点
    btn->setMinimumHeight(38);

    // 信号槽连接：按钮被点击时，调用 onLogin() 函数
    // clicked 是 QPushButton 的信号，onLogin 是我们的槽函数
    connect(btn, &QPushButton::clicked, this, &LoginWindow::onLogin);

    // 密码框按回车键也能登录
    // returnPressed 信号：用户在输入框里按了回车
    connect(m_pwdEdit, &QLineEdit::returnPressed,
            this, &LoginWindow::onLogin);

    // ---- 第 5 步：表单布局（左边标签，右边控件） ----
    auto *form = new QFormLayout;

    // addRow(标签, 控件) 加一行：左边是“账号：”，右边是下拉框
    form->addRow(QStringLiteral("账号："), m_roleBox);
    form->addRow(QStringLiteral("密码："), m_pwdEdit);

    // 去掉表单的默认边距，让布局更紧凑
    form->setContentsMargins(0, 0, 0, 0);

    // ---- 第 6 步：卡片内部垂直布局 ----
    auto *cardLay = new QVBoxLayout(card);  // 垂直布局，从上到下排列
    cardLay->setContentsMargins(28, 30, 28, 28);  // 卡片内边距
    cardLay->setSpacing(14);  // 控件之间的间距

    // addWidget 把控件加到布局里，第二个参数是拉伸系数，第三个是对齐方式
    cardLay->addWidget(title, 0, Qt::AlignHCenter);   // 标题居中
    cardLay->addWidget(subtitle, 0, Qt::AlignHCenter); // 副标题居中
    cardLay->addSpacing(8);   // 加 8 像素的空距
    cardLay->addLayout(form); // 加表单
    cardLay->addWidget(btn);  // 加登录按钮

    // ---- 第 7 步：外层布局，让卡片在窗口中水平居中 ----
    auto *outer = new QVBoxLayout(this);  // 窗口级垂直布局
    outer->addStretch();   // 上弹簧：占满剩余空间，把卡片往下推
    outer->addWidget(card, 0, Qt::AlignHCenter);  // 卡片水平居中
    outer->addStretch();   // 下弹簧：把卡片往上推
    // 上下都有弹簧，卡片就卡在中间了
}

// ============================================================
// onLogin() 函数：登录按钮被点击时执行
// 功能：校验账号密码，成功就发 loggedIn 信号，失败就弹错误框
// ============================================================
void LoginWindow::onLogin()
{
    // currentData() 取当前选中项的隐藏数据（就是我们 addItem 时传的第二个参数）
    // toInt() 把它转成整数（用户id）
    const int userId = m_roleBox->currentData().toInt();

    // 调用 AppContext 的 login 方法校验密码
    // AppContext::get() 是单例，全局只有一个
    // login(用户id, 密码) 返回 true 表示登录成功，false 表示失败
    if (!AppContext::get().login(userId, m_pwdEdit->text())) {
        // 登录失败：弹一个警告框
        QMessageBox::warning(this,
            QStringLiteral("登录失败"),
            QStringLiteral("账号或密码错误"));
        return;  // 直接返回，不执行后面的
    }

    // 登录成功：发射 loggedIn 信号
    // emit 是 Qt 的关键字，表示“发出这个信号”
    // main.cpp 里 connect 了这个信号，会自动打开主窗口
    emit loggedIn();
}
