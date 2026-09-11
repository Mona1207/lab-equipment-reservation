#include "LoginWindow.h"   // 对应头文件
#include "AppContext.h"    // 界面适配层（登录校验）

#include <QVBoxLayout>     // 垂直布局（卡片内）
#include <QFormLayout>     // 表单布局（标签+控件逐行排列）
#include <QComboBox>       // 下拉框
#include <QLineEdit>       // 单行输入框
#include <QPushButton>     // 按钮
#include <QMessageBox>     // 弹窗提示
#include <QLabel>          // 标题/副标题

// 构造登录界面（卡片式：窗口即深底，中央白色圆角卡片）
LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)      // 转交父窗口
{
    setObjectName(QStringLiteral("LoginWindow")); // 供 Theme 渐变背景匹配
    setWindowTitle(QStringLiteral("实验室设备预约管理系统 - 登录")); // 窗口标题
    setFixedSize(420, 380);                   // 固定大小，卡片居中

    // ---- 中央白色卡片 ----
    auto *card = new QWidget(this);           // 卡片容器
    card->setObjectName(QStringLiteral("LoginCard"));
    card->setFixedWidth(320);                 // 卡片宽度

    auto *title = new QLabel(QStringLiteral("🔬 实验室设备预约"), card); // 大标题
    title->setObjectName(QStringLiteral("LoginTitle"));
    auto *subtitle = new QLabel(QStringLiteral("请选择账号并输入密码登录"), card); // 副标题
    subtitle->setObjectName(QStringLiteral("LoginSubtitle"));

    m_roleBox = new QComboBox(card);          // 新建账号下拉框
    // addItem 第二参为隐藏数据：对应该账号在核心里的 int 用户id
    m_roleBox->addItem(QStringLiteral("张三（学生）"), 2001);     // 学生
    m_roleBox->addItem(QStringLiteral("王老师（教师）"), 3001);   // 教师
    m_roleBox->addItem(QStringLiteral("老王（管理员）"), 1);      // 管理员

    m_pwdEdit = new QLineEdit(card);          // 新建密码输入框
    m_pwdEdit->setEchoMode(QLineEdit::Password); // 密码以圆点显示
    m_pwdEdit->setText(QStringLiteral("123"));   // 演示方便，预填密码 123
    m_pwdEdit->setPlaceholderText(QStringLiteral("密码（演示：123）")); // 占位提示

    QPushButton *btn = new QPushButton(QStringLiteral("登  录"), card); // 登录按钮
    btn->setProperty("primary", true);        // 主题主按钮样式（蓝底白字）
    btn->setMinimumHeight(38);                // 加高更易点
    connect(btn, &QPushButton::clicked, this, &LoginWindow::onLogin); // 点击 -> onLogin
    connect(m_pwdEdit, &QLineEdit::returnPressed,   // 密码框回车也能登录
            this, &LoginWindow::onLogin);

    auto *form = new QFormLayout;             // 表单布局
    form->addRow(QStringLiteral("账号："), m_roleBox); // 第一行：账号
    form->addRow(QStringLiteral("密码："), m_pwdEdit); // 第二行：密码
    form->setContentsMargins(0, 0, 0, 0);     // 表单不吃额外边距

    auto *cardLay = new QVBoxLayout(card);    // 卡片垂直布局
    cardLay->setContentsMargins(28, 30, 28, 28); // 卡片内边距
    cardLay->setSpacing(14);                  // 控件间距
    cardLay->addWidget(title, 0, Qt::AlignHCenter);   // 标题居中
    cardLay->addWidget(subtitle, 0, Qt::AlignHCenter); // 副标题居中
    cardLay->addSpacing(8);                   // 标题区与表单拉开距离
    cardLay->addLayout(form);                 // 表单
    cardLay->addWidget(btn);                  // 登录按钮

    // ---- 外层：卡片在窗口中居中 ----
    auto *outer = new QVBoxLayout(this);      // 窗口级布局
    outer->addStretch();                      // 上弹簧
    outer->addWidget(card, 0, Qt::AlignHCenter); // 卡片水平居中
    outer->addStretch();                      // 下弹簧
}

// 登录按钮处理：把 id+密码交给核心校验
void LoginWindow::onLogin()
{
    const int userId = m_roleBox->currentData().toInt();           // 取选中账号的用户id
    if (!AppContext::get().login(userId, m_pwdEdit->text())) {     // 核心校验失败
        QMessageBox::warning(this, QStringLiteral("登录失败"),
                             QStringLiteral("账号或密码错误"));      // 弹警告框
        return;                            // 中止登录
    }
    emit loggedIn();                       // 校验通过：发登录成功信号
}
