// ============================================================
// 文件说明：MainWindow.cpp —— 主窗口具体实现
// 这个文件很大，负责搭建整个主界面的布局和逻辑
// 结构：左边侧边栏 + 右边（顶栏 + 内容页堆叠）
// ============================================================

#include "MainWindow.h"
#include "AppContext.h"
#include "Theme.h"

// 引入各个页面的头文件
#include "EquipmentPage.h"
#include "DashboardPage.h"
#include "StatsPage.h"
#include "AdminPage.h"
#include "MaintenancePage.h"
#include "ReserveDialog.h"

// 引入核心层的类
#include "Reservation.h"
#include "User.h"
#include "DateTime.h"
#include "MaintenanceTask.h"

// 引入 Qt 各种控件和布局类
#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QComboBox>
#include <QFrame>
#include <QStatusBar>

// ============================================================
// NavItem 结构体：导航项的数据结构
// 结构体（struct）就是把几个相关的变量打包在一起
// 比如一个导航项需要：图标、文字、副标题、对应页面索引
// ============================================================
struct NavItem {
    const char* icon;       // 图标（emoji）
    const char* text;       // 导航文字
    const char* subtitle;   // 页面副标题
    int pageIndex;          // 对应的页面索引（0=首页，1=设备...）
};

// 定义 7 个导航项的数组
static const NavItem kNavItems[] = {
    { "🏠", "仪表板",   "总览系统运行状态与待办事项", 0 },
    { "🖥", "设备管理", "维护设备台账与状态信息",     1 },
    { "📅", "我的预约", "查看和管理您的预约记录",     2 },
    { "🔧", "保养管理", "查看并执行设备保养任务",     3 },
    { "📊", "统计分析", "图表化分析设备使用情况",     4 },
    { "👤", "账户管理", "查看账户信息与登录凭据",       5 },
    { "✅", "审批管理", "处理预约审批与归还确认",       6 },
};

// ============================================================
// 构造函数：创建主窗口时自动执行
// 负责：搭建布局、创建控件、连接信号槽
// ============================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)  // 调用父类构造函数
{
    setWindowTitle(QStringLiteral("实验室设备预约管理系统"));  // 窗口标题
    resize(1280, 800);  // 窗口默认大小

    // ==================== 第 1 步：左侧侧边栏 ====================
    auto *sidebar = new QWidget(this);  // 侧栏容器
    sidebar->setObjectName(QStringLiteral("Sidebar"));  // QSS 选中
    sidebar->setFixedWidth(220);  // 侧栏固定宽度 220 像素

    // ---- 顶部品牌标识：蓝色方块 + 系统名 ----
    auto *brandBox = new QHBoxLayout;  // 水平布局
    brandBox->setContentsMargins(16, 0, 12, 0);
    brandBox->setSpacing(10);

    // 蓝色方块logo，里面写个“设”字
    QLabel *brandLogo = new QLabel(QStringLiteral("设"), sidebar);
    brandLogo->setFixedSize(36, 36);
    brandLogo->setAlignment(Qt::AlignCenter);  // 文字居中
    brandLogo->setStyleSheet(QStringLiteral(
        "background: #1565c0;"
        "border-radius: 4px; color: white; font-size: 16px; font-weight: bold;"));

    // 系统标题：两行文字
    QLabel *appTitle = new QLabel(QStringLiteral("实验室设备\n预约管理系统"), sidebar);
    appTitle->setObjectName(QStringLiteral("AppTitle"));
    appTitle->setStyleSheet(QStringLiteral("padding: 0; font-size: 14px; line-height: 1.3;"));

    brandBox->addWidget(brandLogo);    // 加logo
    brandBox->addWidget(appTitle);     // 加标题
    brandBox->addStretch();            // 弹簧，推到左边

    // ---- 导航列表 ----
    m_nav = new QListWidget(sidebar);  // 创建列表
    m_nav->setObjectName(QStringLiteral("NavList"));
    m_nav->setFixedWidth(220);

    // 循环把 7 个导航项加进去
    for (const auto& item : kNavItems) {
        m_nav->addItem(QStringLiteral("%1  %2").arg(
            QString::fromUtf8(item.icon), QString::fromUtf8(item.text)));
    }
    m_nav->setCurrentRow(0);  // 默认选中第0个（仪表板）

    // ---- 底部用户信息区 ----
    auto *footer = new QWidget(sidebar);
    footer->setObjectName(QStringLiteral("SidebarFooter"));

    // 当前登录用户
    User *u = AppContext::get().currentUser();
    m_userName = new QLabel(
        u ? AppContext::toQString(u->getName()) : QStringLiteral("未登录"), footer);
    m_userName->setObjectName(QStringLiteral("UserNameLabel"));

    // 角色标签（学生/教师/管理员）
    QString roleText = QString::fromStdString(u ? u->role_name() : std::string("用户"));
    m_roleBadge = new QLabel(roleText, footer);
    m_roleBadge->setObjectName(QStringLiteral("RoleBadge"));

    // 退出登录按钮
    m_logoutBtn = new QPushButton(QStringLiteral("🚪  退出登录"), footer);
    m_logoutBtn->setObjectName(QStringLiteral("LogoutButton"));

    // 点击退出按钮时弹确认框
    connect(m_logoutBtn, &QPushButton::clicked, this, [this]() {
        const auto ans = QMessageBox::question(this,
            QStringLiteral("退出登录"),
            QStringLiteral("确定要退出当前账户吗？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ans == QMessageBox::Yes) emit logoutRequested();  // 发射退出信号
    });

    // 底部区的垂直布局
    auto *footerLay = new QVBoxLayout(footer);
    footerLay->setContentsMargins(16, 12, 16, 12);
    footerLay->setSpacing(6);
    footerLay->addWidget(m_userName);      // 用户名
    footerLay->addWidget(m_roleBadge, 0, Qt::AlignLeft);  // 角色标签
    footerLay->addSpacing(6);
    footerLay->addWidget(m_logoutBtn);     // 退出按钮

    // 侧栏整体垂直布局
    auto *sideLay = new QVBoxLayout(sidebar);
    sideLay->setContentsMargins(0, 0, 0, 0);
    sideLay->setSpacing(0);
    sideLay->addLayout(brandBox);    // 顶部品牌区
    sideLay->addSpacing(8);
    sideLay->addWidget(m_nav, 1);    // 导航列表（拉伸占满中间）
    sideLay->addWidget(footer);      // 底部用户区

    // ==================== 第 2 步：右侧区域 ====================
    auto *rightArea = new QWidget(this);  // 右边总容器

    // ---- 顶栏（页面标题区） ----
    auto *topBar = new QWidget(rightArea);
    topBar->setObjectName(QStringLiteral("TopBar"));
    topBar->setFixedHeight(72);  // 顶栏高度 72 像素

    // 标题 + 副标题
    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(2);
    m_pageTitle = new QLabel(QStringLiteral("仪表板"), topBar);
    m_pageTitle->setObjectName(QStringLiteral("PageTitle"));
    m_pageSubtitle = new QLabel(QString::fromUtf8(kNavItems[0].subtitle), topBar);
    m_pageSubtitle->setObjectName(QStringLiteral("PageSubtitle"));
    titleBox->addWidget(m_pageTitle);      // 大标题
    titleBox->addWidget(m_pageSubtitle);   // 副标题

    // 顶栏水平布局
    auto *topLay = new QHBoxLayout(topBar);
    topLay->setContentsMargins(28, 0, 24, 0);
    topLay->addLayout(titleBox);
    topLay->addStretch();  // 推到左边

    // ---- 页面堆叠容器 ----
    // QStackedWidget 可以放多个页面，一次只显示一个
    // 切换页面就是 setCurrentIndex()
    m_pages = new QStackedWidget(rightArea);

    // 页面 0：仪表板
    m_dashboard = new DashboardPage(this);
    m_pages->addWidget(m_dashboard);

    // 页面 1：设备管理
    m_equipPage = new EquipmentPage(this);
    m_pages->addWidget(m_equipPage);

    // 页面 2：我的预约（在主窗口里直接写，不用单独文件）
    auto *resPage = new QWidget(this);

    // 发起预约按钮
    auto *addBtn = new QPushButton(QStringLiteral("＋ 发起预约"), resPage);
    addBtn->setProperty("primary", true);  // 蓝色主按钮
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::openReserveDialog);

    // 取消预约按钮
    auto *cancelBtn = new QPushButton(QStringLiteral("✖ 取消预约"), resPage);
    connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelReservation);

    // 状态筛选下拉框
    m_filterCombo = new QComboBox(resPage);
    m_filterCombo->addItems({
        QStringLiteral("全部状态"), QStringLiteral("待审批"),
        QStringLiteral("已通过"), QStringLiteral("已取消"),
        QStringLiteral("已归还"), QStringLiteral("已拒绝")
    });
    connect(m_filterCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::onFilterChanged);

    // 我的预约表格（4列：设备、开始时间、结束时间、状态）
    m_myTable = new QTableWidget(0, 4, resPage);
    m_myTable->setHorizontalHeaderLabels(
        { QStringLiteral("设备"), QStringLiteral("开始时间"),
          QStringLiteral("结束时间"), QStringLiteral("状态") });
    m_myTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁止编辑
    m_myTable->setSelectionBehavior(QAbstractItemView::SelectRows);  // 整行选中
    m_myTable->setAlternatingRowColors(true);  // 斑马纹
    m_myTable->verticalHeader()->setVisible(false);  // 隐藏行号
    m_myTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 列自适应宽度

    // 按钮行水平布局
    auto *btnRow = new QHBoxLayout;
    btnRow->addWidget(addBtn);
    btnRow->addWidget(cancelBtn);
    btnRow->addSpacing(20);
    btnRow->addWidget(new QLabel(QStringLiteral("筛选："), resPage));
    btnRow->addWidget(m_filterCombo);
    btnRow->addStretch();

    // 我的预约页垂直布局
    auto *resLay = new QVBoxLayout(resPage);
    resLay->setContentsMargins(24, 20, 24, 24);
    resLay->setSpacing(16);
    resLay->addLayout(btnRow);   // 按钮行
    resLay->addWidget(m_myTable); // 表格
    m_pages->addWidget(resPage);  // 加到页面堆叠

    // 页面 3：保养管理
    m_maintPage = new MaintenancePage(this);
    m_pages->addWidget(m_maintPage);

    // 页面 4：统计分析
    m_statsPage = new StatsPage(this);
    m_pages->addWidget(m_statsPage);

    // 页面 5：账户管理（简单信息展示页）
    {
        auto *acctPage = new QWidget(this);
        acctPage->setObjectName("MainContent");
        auto *acctLay = new QVBoxLayout(acctPage);
        acctLay->setContentsMargins(32, 28, 32, 28);
        acctLay->setSpacing(16);

        // 白色卡片，显示用户头像、名字、角色、ID
        auto *card = new QFrame(acctPage);
        card->setProperty("glassCard", true);
        card->setFixedWidth(420);
        card->setFixedHeight(280);
        auto *cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(28, 24, 28, 24);
        cardLay->setSpacing(12);

        User *u2 = AppContext::get().currentUser();

        // 蓝色圆形头像（里面是emoji）
        QLabel *avatar = new QLabel(card);
        avatar->setFixedSize(64, 64);
        avatar->setAlignment(Qt::AlignCenter);
        avatar->setText(QStringLiteral("👤"));
        avatar->setStyleSheet(QStringLiteral(
            "background: #1565c0;"
            "border-radius: 32px; font-size: 28px;"));
        cardLay->addWidget(avatar, 0, Qt::AlignCenter);

        // 用户名
        QLabel *nameL = new QLabel(u2 ? AppContext::toQString(u2->getName()) : QStringLiteral("未登录"), card);
        nameL->setAlignment(Qt::AlignCenter);
        QFont nf = nameL->font(); nf.setPointSize(16); nf.setBold(true);
        nameL->setFont(nf);
        cardLay->addWidget(nameL);

        // 角色
        QLabel *roleL = new QLabel(QString::fromStdString(u2 ? u2->role_name() : "用户"), card);
        roleL->setAlignment(Qt::AlignCenter);
        roleL->setStyleSheet("color: #6b7b8f;");
        cardLay->addWidget(roleL);

        cardLay->addSpacing(8);

        // 用户ID
        QLabel *idL = new QLabel(QStringLiteral("用户 ID：%1").arg(u2 ? u2->getId() : 0), card);
        idL->setAlignment(Qt::AlignCenter);
        idL->setStyleSheet("color: #5a6a7e; font-size: 13px;");
        cardLay->addWidget(idL);

        acctLay->addWidget(card, 0, Qt::AlignLeft);
        acctLay->addStretch();
        m_pages->addWidget(acctPage);
    }

    // 页面 6：审批管理
    m_adminPage = new AdminPage(this);
    m_pages->addWidget(m_adminPage);

    // 审批页数据变化时，刷新所有其他页面
    connect(m_adminPage, &AdminPage::dataChanged, this, [this]() {
        m_dashboard->refresh();
        m_equipPage->refreshTable();
        m_statsPage->refresh();
        m_maintPage->refreshTable();
        refreshMyReservations();
        updateStatusBar();
    });

    // 点侧边栏导航切换页面
    connect(m_nav, &QListWidget::currentRowChanged,
            this, &MainWindow::switchPage);

    // 右侧整体垂直布局
    auto *rightLay = new QVBoxLayout(rightArea);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(0);
    rightLay->addWidget(topBar);       // 顶栏
    rightLay->addWidget(m_pages, 1);  // 内容区（拉伸占满）

    // ==================== 第 3 步：底部状态栏 ====================
    statusBar()->setSizeGripEnabled(false);  // 隐藏右下角调整大小的小三角

    // 创建状态栏的各个标签
    m_statEquip   = new QLabel(this);
    m_statToday   = new QLabel(this);
    m_statPending = new QLabel(this);
    m_statMaint   = new QLabel(this);
    m_statUser    = new QLabel(this);

    // 加到状态栏
    statusBar()->addWidget(m_statEquip);
    statusBar()->addWidget(m_statToday);
    statusBar()->addWidget(m_statPending);
    statusBar()->addWidget(m_statMaint);
    statusBar()->addPermanentWidget(m_statUser);  // permanent 表示靠右对齐

    // ==================== 第 4 步：整体布局 ====================
    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("MainContent"));

    // 水平布局：左边侧栏，右边内容区
    auto *mainLay = new QHBoxLayout(central);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    mainLay->addWidget(sidebar);       // 左边侧栏
    mainLay->addWidget(rightArea, 1);   // 右边内容区（拉伸）

    setCentralWidget(central);  // 设置为中心部件

    // 初始刷新数据
    refreshMyReservations();
    updateStatusBar();
}

// ============================================================
// updateStatusBar() 函数：刷新底部状态栏的数据
// 统计：设备总数、今日预约数、待审批数、待保养数
// ============================================================
void MainWindow::updateStatusBar()
{
    ReservationManager& mgr = AppContext::get().manager();
    const DateTime now = DateTime::now();  // 当前时间

    // 设备总数
    const int equipTotal = static_cast<int>(mgr.equipments().size());

    // 统计今日预约、待审批、待保养
    int todayCount = 0, pendingCount = 0, maintCount = 0;
    for (const Reservation& r : mgr.reservations()) {
        if (r.start_time().same_day(now)) ++todayCount;  // 今天的预约
        if (r.status() == ReservationStatus::Pending) ++pendingCount;  // 待审批
    }
    mgr.scan_due_maintenance();  // 扫描生成到期保养任务
    for (const MaintenanceTask& t : mgr.maintenance_tasks())
        if (!t.executed()) ++maintCount;  // 未执行的保养任务

    // 设置状态栏文字
    m_statEquip->setText(QStringLiteral("🖥 设备总数：%1").arg(equipTotal));
    m_statToday->setText(QStringLiteral("📅 今日预约：%1").arg(todayCount));
    m_statPending->setText(QStringLiteral("⏳ 待审批：%1").arg(pendingCount));
    m_statMaint->setText(QStringLiteral("🔧 待保养：%1").arg(maintCount));

    // 当前用户
    User *u = AppContext::get().currentUser();
    if (u) {
        m_statUser->setText(QStringLiteral("当前账户：%1（%2）")
            .arg(AppContext::toQString(u->getName()),
                 QString::fromStdString(u->role_name())));
    } else {
        m_statUser->setText(QStringLiteral("未登录"));
    }
}

// ============================================================
// openReserveDialog() 函数：打开预约对话框
// ============================================================
void MainWindow::openReserveDialog()
{
    ReserveDialog dlg(this);  // 创建对话框（栈上对象）
    if (dlg.exec() == QDialog::Accepted) {  // exec() 是模态显示，用户点了确定
        // 预约成功，刷新各个页面
        refreshMyReservations();
        m_dashboard->refresh();
        m_statsPage->refresh();
        updateStatusBar();
    }
}

// ============================================================
// refreshMyReservations() 函数：刷新“我的预约”表格
// 从核心管理器读数据，筛选当前用户的预约，填到表格里
// ============================================================
void MainWindow::refreshMyReservations()
{
    User *u = AppContext::get().currentUser();
    if (!u) return;  // 没登录就返回

    // 当前筛选条件
    const QString filter = m_filterCombo ? m_filterCombo->currentText()
                                          : QStringLiteral("全部状态");

    const auto& all = AppContext::get().manager().reservations();

    // 先数一下有多少条符合条件的，设置表格行数
    int count = 0;
    for (const Reservation& r : all) {
        if (r.user_id() != u->getId()) continue;  // 不是当前用户的跳过
        if (filter != QStringLiteral("全部状态") &&
            AppContext::toQString(r.status_name()) != filter) continue;  // 状态不匹配跳过
        ++count;
    }
    m_myTable->setRowCount(count);  // 设置行数

    // 填充每一行的数据
    int i = 0;
    for (const Reservation& r : all) {
        if (r.user_id() != u->getId()) continue;
        if (filter != QStringLiteral("全部状态") &&
            AppContext::toQString(r.status_name()) != filter) continue;

        // 第0列：设备名
        m_myTable->setItem(i, 0, new QTableWidgetItem(
            AppContext::get().equipmentName(r.equipment_id())));
        // 第1列：开始时间
        m_myTable->setItem(i, 1, new QTableWidgetItem(
            AppContext::toQString(r.start_time().to_string())));
        // 第2列：结束时间
        m_myTable->setItem(i, 2, new QTableWidgetItem(
            AppContext::toQString(r.end_time().to_string())));
        // 第3列：状态（彩色单元格）
        m_myTable->setItem(i, 3, Theme::makeStatusItem(
            AppContext::toQString(r.status_name())));
        ++i;
    }
}

// ============================================================
// onCancelReservation() 函数：取消选中的预约
// ============================================================
void MainWindow::onCancelReservation()
{
    const int row = m_myTable->currentRow();  // 当前选中行
    if (row < 0) {  // 没选中任何行
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条预约记录"));
        return;
    }

    User *u = AppContext::get().currentUser();
    if (!u) return;

    // 从表格里拿到设备名和开始时间
    const QString equipName = m_myTable->item(row, 0)->text();
    const QString startStr  = m_myTable->item(row, 1)->text();

    // 从核心数据里找到对应的预约id
    std::string targetId;
    for (const Reservation& r : AppContext::get().manager().reservations()) {
        if (r.user_id() == u->getId() &&
            AppContext::get().equipmentName(r.equipment_id()) == equipName &&
            AppContext::toQString(r.start_time().to_string()) == startStr) {
            targetId = r.id();
            break;
        }
    }

    if (targetId.empty()) {
        QMessageBox::warning(this, QStringLiteral("取消失败"),
                             QStringLiteral("未找到对应的预约记录"));
        return;
    }

    // 确认取消
    const auto answer = QMessageBox::question(this,
        QStringLiteral("取消预约"),
        QStringLiteral("确定取消这条预约吗？取消后时段将释放。"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    // 调用核心取消预约
    if (!AppContext::get().manager().cancel_reservation(targetId)) {
        QMessageBox::warning(this, QStringLiteral("取消失败"),
                             QStringLiteral("只有待审批或已通过的预约才能取消。"));
        return;
    }

    QMessageBox::information(this, QStringLiteral("取消成功"),
                             QStringLiteral("预约已取消，时段已释放。"));

    // 刷新各个页面
    refreshMyReservations();
    m_dashboard->refresh();
    m_statsPage->refresh();
    updateStatusBar();
}

// 筛选条件变了，刷新表格
void MainWindow::onFilterChanged(const QString&)
{
    refreshMyReservations();
}

// ============================================================
// switchPage() 函数：切换页面
// index 是页面索引（0=首页，1=设备，2=我的预约...）
// ============================================================
void MainWindow::switchPage(int index)
{
    m_pages->setCurrentIndex(index);  // 切换到对应页面

    // 更新顶栏标题和副标题
    if (index >= 0 && index < 7) {
        m_pageTitle->setText(QString::fromUtf8(kNavItems[index].text));
        m_pageSubtitle->setText(QString::fromUtf8(kNavItems[index].subtitle));
    }

    // 切换到对应页面时，刷新那个页面的数据
    switch (index) {
    case 0: m_dashboard->refresh();      break;
    case 1: m_equipPage->refreshTable(); break;
    case 2: refreshMyReservations();     break;
    case 3: m_maintPage->refreshTable(); break;
    case 4: m_statsPage->refresh();      break;
    case 5: /* 账户管理页静态信息，不用刷新 */ break;
    case 6: m_adminPage->refreshTable(); break;
    }
    updateStatusBar();
}
