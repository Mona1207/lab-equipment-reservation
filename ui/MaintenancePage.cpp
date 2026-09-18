#include "MaintenancePage.h"
#include "AppContext.h"
#include "Theme.h"
#include "MaintenanceTask.h"
#include "Equipment.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QBrush>
#include <QColor>

static const int kTaskIdRole = Qt::UserRole + 1;

MaintenancePage::MaintenancePage(QWidget *parent)
    : QWidget(parent)
{
    m_summary = new QLabel(this);
    m_summary->setStyleSheet(QStringLiteral(
        "color: #1f2937; font-size: 13px; font-weight: bold; padding: 4px 0;"));

    QPushButton *execBtn = new QPushButton(QStringLiteral("🔧 执行选中保养"), this);
    execBtn->setProperty("primary", true);
    QPushButton *refBtn  = new QPushButton(QStringLiteral("刷新列表"), this);
    connect(execBtn, &QPushButton::clicked, this, &MaintenancePage::onExecute);
    connect(refBtn,  &QPushButton::clicked, this, &MaintenancePage::refreshTable);

    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(m_summary);
    btnRow->addStretch();
    btnRow->addWidget(execBtn);
    btnRow->addWidget(refBtn);

    m_table = new QTableWidget(0, 5, this);
    m_table->setHorizontalHeaderLabels(
        { QStringLiteral("任务ID"), QStringLiteral("设备"), QStringLiteral("类型"),
          QStringLiteral("计划日期"), QStringLiteral("状态") });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(12);
    lay->addLayout(btnRow);
    lay->addWidget(m_table);

    refreshTable();
}

void MaintenancePage::refreshTable()
{
    AppContext::get().manager().scan_due_maintenance();

    const auto& list = AppContext::get().manager().maintenance_tasks();
    m_table->setRowCount(static_cast<int>(list.size()));

    int pendingCount = 0;
    int doneCount = 0;
    int row = 0;

    for (const MaintenanceTask& t : list) {
        const QString equipName = AppContext::get().equipmentName(t.equipment_id());
        const QString typeName = (t.type() == MaintenanceType::Periodic)
                                      ? QStringLiteral("周期保养")
                                      : QStringLiteral("次数保养");
        const QString statusName = t.executed() ? QStringLiteral("已执行") : QStringLiteral("未执行");

        if (t.executed()) ++doneCount; else ++pendingCount;

        auto *idItem = new QTableWidgetItem(AppContext::toQString(t.id()));
        idItem->setData(kTaskIdRole, AppContext::toQString(t.id()));
        m_table->setItem(row, 0, idItem);
        m_table->setItem(row, 1, new QTableWidgetItem(equipName));
        m_table->setItem(row, 2, new QTableWidgetItem(typeName));
        m_table->setItem(row, 3, new QTableWidgetItem(AppContext::toQString(t.scheduled_date().to_string())));

        auto *statusItem = Theme::makeStatusItem(statusName);
        statusItem->setData(kTaskIdRole, AppContext::toQString(t.id()));
        m_table->setItem(row, 4, statusItem);

        // 未执行行整行浅琥珀底突出
        if (!t.executed()) {
            for (int c = 0; c < 5; ++c) {
                auto *item = m_table->item(row, c);
                if (item) item->setBackground(QBrush(QColor(255, 248, 225)));
            }
        }
        ++row;
    }

    m_summary->setText(QStringLiteral("📋 共 %1 个保养任务（未执行 %2，已执行 %3）")
                           .arg(list.size()).arg(pendingCount).arg(doneCount));
}

static QString selectedTaskId(QTableWidget *table)
{
    const int row = table->currentRow();
    if (row < 0) return QString();
    return table->item(row, 0)->data(kTaskIdRole).toString();
}

void MaintenancePage::onExecute()
{
    const QString taskId = selectedTaskId(m_table);
    if (taskId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条保养任务"));
        return;
    }

    const auto answer = QMessageBox::question(this,
        QStringLiteral("执行保养"),
        QStringLiteral("确定执行保养任务「%1」吗？\n执行后设备保养计数器将重置。").arg(taskId),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    const bool ok = AppContext::get().manager().execute_maintenance_task(taskId.toStdString());
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("执行失败"),
                             QStringLiteral("该任务可能已执行，或对应设备已被删除。"));
        return;
    }

    QMessageBox::information(this, QStringLiteral("执行成功"),
                             QStringLiteral("保养任务 %1 已执行，设备已恢复正常状态。").arg(taskId));
    refreshTable();
    emit dataChanged();
}
