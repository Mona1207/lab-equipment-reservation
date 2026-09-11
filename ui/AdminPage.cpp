#include "AdminPage.h"    // 对应头文件
#include "AppContext.h"   // 界面适配层
#include "Theme.h"        // 全局主题（状态彩色单元格）

#include "Reservation.h" // 核心预约实体（状态枚举）

#include <QVBoxLayout>    // 垂直布局
#include <QHBoxLayout>    // 水平布局
#include <QTableWidget>   // 表格
#include <QHeaderView>    // 表头
#include <QPushButton>    // 按钮
#include <QMessageBox>    // 弹窗
#include <QLabel>         // 文本标签
#include <QBrush>         // 画刷（单元格底色）
#include <QColor>         // 颜色
#include <QFileDialog>    // 文件保存对话框
#include <QFile>          // 文件操作
#include <QTextStream>    // 文本流（写CSV）
#include <QDateTime>      // 导出文件名用时间戳

static const int kResIdRole = Qt::UserRole + 1;   // 自定义角色：单元格里藏预约id字符串

// 构造管理员审批页
AdminPage::AdminPage(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *okBtn  = new QPushButton(QStringLiteral("审批通过"), this);
    QPushButton *retBtn = new QPushButton(QStringLiteral("确认归还"), this);
    QPushButton *expBtn = new QPushButton(QStringLiteral("📤 导出CSV"), this);
    QPushButton *refBtn = new QPushButton(QStringLiteral("刷新列表"), this);
    connect(okBtn,  &QPushButton::clicked, this, &AdminPage::onApprove);
    connect(retBtn, &QPushButton::clicked, this, &AdminPage::onReturn);
    connect(expBtn, &QPushButton::clicked, this, &AdminPage::onExportCsv);
    connect(refBtn, &QPushButton::clicked, this, &AdminPage::refreshTable);

    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(new QLabel(QStringLiteral("预约记录："), this));
    btnRow->addStretch();
    btnRow->addWidget(okBtn);
    btnRow->addWidget(retBtn);
    btnRow->addWidget(expBtn);
    btnRow->addWidget(refBtn);

    m_table = new QTableWidget(0, 6, this);
    m_table->setHorizontalHeaderLabels(
        { QStringLiteral("预约ID"), QStringLiteral("设备"), QStringLiteral("用户ID"),
          QStringLiteral("开始时间"), QStringLiteral("结束时间"), QStringLiteral("状态") });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(16);
    lay->addLayout(btnRow);
    lay->addWidget(m_table);

    refreshTable();
}

// 刷新审批表格：遍历核心 reservations() 双端队列
void AdminPage::refreshTable()
{
    const auto& list = AppContext::get().manager().reservations();
    m_table->setRowCount(static_cast<int>(list.size()));

    int row = 0;
    for (const Reservation& r : list)
    {
        const QString equipName = AppContext::get().equipmentName(r.equipment_id());
        const QString start = AppContext::toQString(r.start_time().to_string());
        const QString end   = AppContext::toQString(r.end_time().to_string());

        auto *idItem = new QTableWidgetItem(AppContext::toQString(r.id()));
        idItem->setData(kResIdRole, AppContext::toQString(r.id()));
        m_table->setItem(row, 0, idItem);
        m_table->setItem(row, 1, new QTableWidgetItem(equipName));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::number(r.user_id())));
        m_table->setItem(row, 3, new QTableWidgetItem(start));
        m_table->setItem(row, 4, new QTableWidgetItem(end));

        auto *statusItem = Theme::makeStatusItem(AppContext::toQString(r.status_name()));
        statusItem->setData(kResIdRole, AppContext::toQString(r.id()));
        m_table->setItem(row, 5, statusItem);

        if (r.status() == ReservationStatus::Pending)
        {
            for (int c = 0; c < 6; ++c) {
                auto *item = m_table->item(row, c);
                if (item) item->setBackground(QBrush(QColor(255, 248, 220)));
            }
        }
        ++row;
    }
}

// 文件内辅助：取当前选中行隐藏的预约id
static QString selectedResId(QTableWidget *table)
{
    const int row = table->currentRow();
    if (row < 0) return QString();
    return table->item(row, 0)->data(kResIdRole).toString();
}

// 审批通过：调核心 batch_approve（内部只对"待审批"生效）
void AdminPage::onApprove()
{
    const QString resId = selectedResId(m_table);
    if (resId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条预约记录"));
        return;
    }
    const int n = AppContext::get().manager().batch_approve(
        { resId.toStdString() });

    if (n == 0) {
        QMessageBox::warning(this, QStringLiteral("无法通过"),
                             QStringLiteral("只有「待审批」状态的记录才能审批通过。"));
        return;
    }
    refreshTable();
    emit dataChanged();
}

// 确认归还：调核心 complete_reservation（仅"已通过"可归还，并累计使用次数）
void AdminPage::onReturn()
{
    const QString resId = selectedResId(m_table);
    if (resId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条预约记录"));
        return;
    }
    const bool ok = AppContext::get().manager().complete_reservation(
        resId.toStdString(), DateTime::now());

    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("无法归还"),
                             QStringLiteral("只有「已通过」状态的记录才能确认归还。"));
        return;
    }
    refreshTable();
    emit dataChanged();
}

// 导出全部预约记录为 CSV
void AdminPage::onExportCsv()
{
    const auto& list = AppContext::get().manager().reservations();
    if (list.empty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("当前没有预约记录可导出"));
        return;
    }

    const QString defaultName = QStringLiteral("预约记录_%1.csv")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    const QString filePath = QFileDialog::getSaveFileName(this,
        QStringLiteral("导出预约记录"), defaultName,
        QStringLiteral("CSV 文件 (*.csv)"));
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("导出失败"),
                             QStringLiteral("无法打开文件进行写入"));
        return;
    }

    QTextStream out(&file);
    out << "\xEF\xBB\xBF";
    out << "预约ID,设备,用户ID,开始时间,结束时间,状态\n";

    for (const Reservation& r : list) {
        out << AppContext::toQString(r.id()) << ","
            << AppContext::get().equipmentName(r.equipment_id()) << ","
            << r.user_id() << ","
            << AppContext::toQString(r.start_time().to_string()) << ","
            << AppContext::toQString(r.end_time().to_string()) << ","
            << AppContext::toQString(r.status_name()) << "\n";
    }

    file.close();
    QMessageBox::information(this, QStringLiteral("导出成功"),
                             QStringLiteral("已导出 %1 条预约记录到：\n%2").arg(list.size()).arg(filePath));
}
