#include "MainWindow.h"
#include "AppContext.h"
#include "Theme.h"

#include "EquipmentPage.h"
#include "DashboardPage.h"
#include "StatsPage.h"
#include "AdminPage.h"
#include "MaintenancePage.h"
#include "ReserveDialog.h"

#include "Reservation.h"
#include "User.h"
#include "DateTime.h"
#include "MaintenanceTask.h"

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

// 导航项定义：图标 + 文字 + 页面副标题 + 页面索引
struct NavItem {
    const char* icon;
    const char* text;
    const char* subtitle;
    int pageIndex;
};

static const NavItem kNavItems[] = {
    { "🏠", "仪表板",   "总览系统运行状态与待办事项", 0 },
    { "🖥", "设备管理", "维护设备台账与状态信息",     1 },
    { "📅", "我的预约", "查看和管理您的预约记录",     2 },
    { "🔧", "保养管理", "查看并执行设备保养任务",     3 },
    { "📊", "统计分析", "图表化分析设备使用情况",     4 },
    { "👤", "账户管理", "查看账户信息与登录凭据",       5 },
    { "✅", "审批管理", "处理预约审批与归还确认",       6 },
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("实验室设备预约管理系统"));
    resize(1280, 800);

    // ==================== 左侧侧边栏 ====================
    auto *sidebar = new QWidget(this);
    sidebar->setObjectName(QStringLiteral("Sidebar"));
    sidebar->setFixedWidth(220);

    // 顶部品牌标识：深蓝方块 + 系统名
    auto *brandBox = new QHBoxLayout;
    brandBox->setContentsMargins(16, 0, 12, 0);
    brandBox->setSpacing(10);

    QLabel *brandLogo = new QLabel(QStringLiteral("设"), sidebar);
    brandLogo->setFixedSize(36, 36);
    brandLogo->setAlignment(Qt::AlignCenter);
    brandLogo->setStyleSheet(QStringLiteral(
        "background: #1565c0;"
        "border-radius: 4px; color: white; font-size: 16px; font-weight: bold;"));

    QLabel *appTitle = new QLabel(QStringLiteral("实验室设备\n预约管理系统"), sidebar);
    appTitle->setObjectName(QStringLiteral("AppTitle"));
    appTitle->setStyleSheet(QStringLiteral("padding: 0; font-size: 14px; line-height: 1.3;"));

    brandBox->addWidget(brandLogo);
    brandBox->addWidget(appTitle);
    brandBox->addStretch();

    // 导航列表
    m_nav = new QListWidget(sidebar);
    m_nav->setObjectName(QStringLiteral("NavList"));
    m_nav->setFixedWidth(220);
    for (const auto& item : kNavItems) {
        m_nav->addItem(QStringLiteral("%1  %2").arg(
            QString::fromUtf8(item.icon), QString::fromUtf8(item.text)));
    }
    m_nav->setCurrentRow(0);

    // 底部用户信息区
    auto *footer = new QWidget(sidebar);
    footer->setObjectName(QStringLiteral("SidebarFooter"));

    User *u = AppContext::get().currentUser();
    m_userName = new QLabel(
        u ? AppContext::toQString(u->getName()) : QStringLiteral("未登录"), footer);
    m_userName->setObjectName(QStringLiteral("UserNameLabel"));

    QString roleText = QString::fromStdString(u ? u->role_name() : std::string("用户"));
    m_roleBadge = new QLabel(roleText, footer);
    m_roleBadge->setObjectName(QStringLiteral("RoleBadge"));

    m_logoutBtn = new QPushButton(QStringLiteral("🚪  退出登录"), footer);
    m_logoutBtn->setObjectName(QStringLiteral("LogoutButton"));
    connect(m_logoutBtn, &QPushButton::clicked, this, [this]() {
        const auto ans = QMessageBox::question(this,
            QStringLiteral("退出登录"),
            QStringLiteral("确定要退出当前账户吗？"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ans == QMessageBox::Yes) emit logoutRequested();
    });

    auto *footerLay = new QVBoxLayout(footer);
    footerLay->setContentsMargins(16, 12, 16, 12);
    footerLay->setSpacing(6);
    footerLay->addWidget(m_userName);
    footerLay->addWidget(m_roleBadge, 0, Qt::AlignLeft);
    footerLay->addSpacing(6);
    footerLay->addWidget(m_logoutBtn);

    auto *sideLay = new QVBoxLayout(sidebar);
    sideLay->setContentsMargins(0, 0, 0, 0);
    sideLay->setSpacing(0);
    sideLay->addLayout(brandBox);
    sideLay->addSpacing(8);
    sideLay->addWidget(m_nav, 1);
    sideLay->addWidget(footer);

    // ==================== 右侧区域 ====================
    auto *rightArea = new QWidget(this);

    // 顶栏
    auto *topBar = new QWidget(rightArea);
    topBar->setObjectName(QStringLiteral("TopBar"));
    topBar->setFixedHeight(72);

    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(2);
    m_pageTitle = new QLabel(QStringLiteral("仪表板"), topBar);
    m_pageTitle->setObjectName(QStringLiteral("PageTitle"));
    m_pageSubtitle = new QLabel(QString::fromUtf8(kNavItems[0].subtitle), topBar);
    m_pageSubtitle->setObjectName(QStringLiteral("PageSubtitle"));
    titleBox->addWidget(m_pageTitle);
    titleBox->addWidget(m_pageSubtitle);

    auto *topLay = new QHBoxLayout(topBar);
    topLay->setContentsMargins(28, 0, 24, 0);
    topLay->addLayout(titleBox);
    topLay->addStretch();

    // 页面堆叠
    m_pages = new QStackedWidget(rightArea);

    // 页面0：仪表板
    m_dashboard = new DashboardPage(this);
    m_pages->addWidget(m_dashboard);

    // 页面1：设备管理
    m_equipPage = new EquipmentPage(this);
    m_pages->addWidget(m_equipPage);

    // 页面2：我的预约
    auto *resPage = new QWidget(this);
    auto *addBtn = new QPushButton(QStringLiteral("＋ 发起预约"), resPage);
    addBtn->setProperty("primary", true);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::openReserveDialog);

    auto *cancelBtn = new QPushButton(QStringLiteral("✖ 取消预约"), resPage);
    connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelReservation);

    m_filterCombo = new QComboBox(resPage);
    m_filterCombo->addItems({
        QStringLiteral("全部状态"), QStringLiteral("待审批"),
        QStringLiteral("已通过"), QStringLiteral("已取消"),
        QStringLiteral("已归还"), QStringLiteral("已拒绝")
    });
    connect(m_filterCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::onFilterChanged);

    m_myTable = new QTableWidget(0, 4, resPage);
    m_myTable->setHorizontalHeaderLabels(
        { QStringLiteral("设备"), QStringLiteral("开始时间"),
          QStringLiteral("结束时间"), QStringLiteral("状态") });
    m_myTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_myTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_myTable->setAlternatingRowColors(true);
    m_myTable->verticalHeader()->setVisible(false);
    m_myTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    auto *btnRow = new QHBoxLayout;
    btnRow->addWidget(addBtn);
    btnRow->addWidget(cancelBtn);
    btnRow->addSpacing(20);
    btnRow->addWidget(new QLabel(QStringLiteral("筛选："), resPage));
    btnRow->addWidget(m_filterCombo);
    btnRow->addStretch();

    auto *resLay = new QVBoxLayout(resPage);
    resLay->setContentsMargins(24, 20, 24, 24);
    resLay->setSpacing(16);
    resLay->addLayout(btnRow);
    resLay->addWidget(m_myTable);
    m_pages->addWidget(resPage);

    // 页面3：保养管理
    m_maintPage = new MaintenancePage(this);
    m_pages->addWidget(m_maintPage);

    // 页面4：统计
    m_statsPage = new StatsPage(this);
    m_pages->addWidget(m_statsPage);

    // 页面5：账户管理（简单信息展示页）
    {
        auto *acctPage = new QWidget(this);
        acctPage->setObjectName("MainContent");
        auto *acctLay = new QVBoxLayout(acctPage);
        acctLay->setContentsMargins(32, 28, 32, 28);
        acctLay->setSpacing(16);

        auto *card = new QFrame(acctPage);
        card->setProperty("glassCard", true);
        card->setFixedWidth(420);
        card->setFixedHeight(280);
        auto *cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(28, 24, 28, 24);
        cardLay->setSpacing(12);

        User *u2 = AppContext::get().currentUser();
        QLabel *avatar = new QLabel(card);
        avatar->setFixedSize(64, 64);
        avatar->setAlignment(Qt::AlignCenter);
        avatar->setText(QStringLiteral("👤"));
        avatar->setStyleSheet(QStringLiteral(
            "background: #1565c0;"
            "border-radius: 32px; font-size: 28px;"));
        cardLay->addWidget(avatar, 0, Qt::AlignCenter);

        QLabel *nameL = new QLabel(u2 ? AppContext::toQString(u2->getName()) : QStringLiteral("未登录"), card);
        nameL->setAlignment(Qt::AlignCenter);
        QFont nf = nameL->font(); nf.setPointSize(16); nf.setBold(true);
        nameL->setFont(nf);
        cardLay->addWidget(nameL);

        QLabel *roleL = new QLabel(QString::fromStdString(u2 ? u2->role_name() : "用户"), card);
        roleL->setAlignment(Qt::AlignCenter);
        roleL->setStyleSheet("color: #6b7b8f;");
        cardLay->addWidget(roleL);

        cardLay->addSpacing(8);
        QLabel *idL = new QLabel(QStringLiteral("用户 ID：%1").arg(u2 ? u2->getId() : 0), card);
        idL->setAlignment(Qt::AlignCenter);
        idL->setStyleSheet("color: #5a6a7e; font-size: 13px;");
        cardLay->addWidget(idL);

        acctLay->addWidget(card, 0, Qt::AlignLeft);
        acctLay->addStretch();
        m_pages->addWidget(acctPage);
    }

    // 页面6：审批
    m_adminPage = new AdminPage(this);
    m_pages->addWidget(m_adminPage);

    connect(m_adminPage, &AdminPage::dataChanged, this, [this]() {
        m_dashboard->refresh();
        m_equipPage->refreshTable();
        m_statsPage->refresh();
        m_maintPage->refreshTable();
        refreshMyReservations();
        updateStatusBar();
    });

    connect(m_nav, &QListWidget::currentRowChanged,
            this, &MainWindow::switchPage);

    // 右侧总布局
    auto *rightLay = new QVBoxLayout(rightArea);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(0);
    rightLay->addWidget(topBar);
    rightLay->addWidget(m_pages, 1);

    // 底部状态栏
    statusBar()->setSizeGripEnabled(false);
    m_statEquip   = new QLabel(this);
    m_statToday   = new QLabel(this);
    m_statPending = new QLabel(this);
    m_statMaint   = new QLabel(this);
    m_statUser    = new QLabel(this);
    statusBar()->addWidget(m_statEquip);
    statusBar()->addWidget(m_statToday);
    statusBar()->addWidget(m_statPending);
    statusBar()->addWidget(m_statMaint);
    statusBar()->addPermanentWidget(m_statUser);

    // 整体
    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("MainContent"));
    auto *mainLay = new QHBoxLayout(central);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);
    mainLay->addWidget(sidebar);
    mainLay->addWidget(rightArea, 1);
    setCentralWidget(central);

    refreshMyReservations();
    updateStatusBar();
}

// 刷新底部状态栏
void MainWindow::updateStatusBar()
{
    ReservationManager& mgr = AppContext::get().manager();
    const DateTime now = DateTime::now();

    const int equipTotal = static_cast<int>(mgr.equipments().size());
    int todayCount = 0, pendingCount = 0, maintCount = 0;
    for (const Reservation& r : mgr.reservations()) {
        if (r.start_time().same_day(now)) ++todayCount;
        if (r.status() == ReservationStatus::Pending) ++pendingCount;
    }
    mgr.scan_due_maintenance();
    for (const MaintenanceTask& t : mgr.maintenance_tasks())
        if (!t.executed()) ++maintCount;

    m_statEquip->setText(QStringLiteral("🖥 设备总数：%1").arg(equipTotal));
    m_statToday->setText(QStringLiteral("📅 今日预约：%1").arg(todayCount));
    m_statPending->setText(QStringLiteral("⏳ 待审批：%1").arg(pendingCount));
    m_statMaint->setText(QStringLiteral("🔧 待保养：%1").arg(maintCount));

    User *u = AppContext::get().currentUser();
    if (u) {
        m_statUser->setText(QStringLiteral("当前账户：%1（%2）")
            .arg(AppContext::toQString(u->getName()),
                 QString::fromStdString(u->role_name())));
    } else {
        m_statUser->setText(QStringLiteral("未登录"));
    }
}

void MainWindow::openReserveDialog()
{
    ReserveDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        refreshMyReservations();
        m_dashboard->refresh();
        m_statsPage->refresh();
        updateStatusBar();
    }
}

void MainWindow::refreshMyReservations()
{
    User *u = AppContext::get().currentUser();
    if (!u) return;

    const QString filter = m_filterCombo ? m_filterCombo->currentText()
                                          : QStringLiteral("全部状态");
    const auto& all = AppContext::get().manager().reservations();
    int count = 0;
    for (const Reservation& r : all) {
        if (r.user_id() != u->getId()) continue;
        if (filter != QStringLiteral("全部状态") &&
            AppContext::toQString(r.status_name()) != filter) continue;
        ++count;
    }

    m_myTable->setRowCount(count);
    int i = 0;
    for (const Reservation& r : all) {
        if (r.user_id() != u->getId()) continue;
        if (filter != QStringLiteral("全部状态") &&
            AppContext::toQString(r.status_name()) != filter) continue;
        m_myTable->setItem(i, 0, new QTableWidgetItem(
            AppContext::get().equipmentName(r.equipment_id())));
        m_myTable->setItem(i, 1, new QTableWidgetItem(
            AppContext::toQString(r.start_time().to_string())));
        m_myTable->setItem(i, 2, new QTableWidgetItem(
            AppContext::toQString(r.end_time().to_string())));
        m_myTable->setItem(i, 3, Theme::makeStatusItem(
            AppContext::toQString(r.status_name())));
        ++i;
    }
}

void MainWindow::onCancelReservation()
{
    const int row = m_myTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条预约记录"));
        return;
    }
    User *u = AppContext::get().currentUser();
    if (!u) return;

    const QString equipName = m_myTable->item(row, 0)->text();
    const QString startStr  = m_myTable->item(row, 1)->text();

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
    const auto answer = QMessageBox::question(this,
        QStringLiteral("取消预约"),
        QStringLiteral("确定取消这条预约吗？取消后时段将释放。"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    if (!AppContext::get().manager().cancel_reservation(targetId)) {
        QMessageBox::warning(this, QStringLiteral("取消失败"),
                             QStringLiteral("只有待审批或已通过的预约才能取消。"));
        return;
    }
    QMessageBox::information(this, QStringLiteral("取消成功"),
                             QStringLiteral("预约已取消，时段已释放。"));
    refreshMyReservations();
    m_dashboard->refresh();
    m_statsPage->refresh();
    updateStatusBar();
}

void MainWindow::onFilterChanged(const QString&)
{
    refreshMyReservations();
}

void MainWindow::switchPage(int index)
{
    m_pages->setCurrentIndex(index);
    // 更新顶栏标题 + 副标题
    if (index >= 0 && index < 7) {
        m_pageTitle->setText(QString::fromUtf8(kNavItems[index].text));
        m_pageSubtitle->setText(QString::fromUtf8(kNavItems[index].subtitle));
    }
    // 刷新对应页
    switch (index) {
    case 0: m_dashboard->refresh();      break;
    case 1: m_equipPage->refreshTable(); break;
    case 2: refreshMyReservations();     break;
    case 3: m_maintPage->refreshTable(); break;
    case 4: m_statsPage->refresh();      break;
    case 5: /* 账户管理页静态信息 */       break;
    case 6: m_adminPage->refreshTable(); break;
    }
    updateStatusBar();
}
