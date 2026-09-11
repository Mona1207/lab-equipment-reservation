#include "MaintenancePage.h"   // 对应头文件
#include "AppContext.h"        // 界面适配层
#include "Theme.h"             // 全局主题（状态彩色单元格）

#include "MaintenanceTask.h"   // 核心保养任务实体
#include "Equipment.h"         // 核心设备实体（查设备名）

#include <QVBoxLayout>         // 垂直布局
#include <QHBoxLayout>         // 水平布局
#include <QTableWidget>        // 表格
#include <QHeaderView>         // 表头
#include <QPushButton>         // 按钮
#include <QMessageBox>         // 弹窗
#include <QLabel>              // 文本标签
#include <QBrush>              // 画刷
#include <QColor>              // 颜色

static const int kTaskIdRole = Qt::UserRole + 1;   // 自定义角色：单元格里藏任务id字符串

// 构造保养管理页
MaintenancePage::MaintenancePage(QWidget *parent)
    : QWidget(parent)
{
    // ---- 顶部统计摘要 ----
    m_summary = new QLabel(this);
    m_summary->setStyleSheet(QStringLiteral(
        "color: #2b3445; font-size: 13px; font-weight: bold; padding: 4px 0;"));

    // ---- 按钮行 ----
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

    // ---- 表格 ----
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

    refreshTable();   // 构造时先加载
}

// 刷新表格：先调 scan_due_maintenance 生成到期任务，再遍历全部任务
void MaintenancePage::refreshTable()
{
    // 扫描并自动生成到期保养任务（核心内部去重，不会重复生成）
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

        // 未执行行整行浅黄底突出
        if (!t.executed()) {
            for (int c = 0; c < 5; ++c) {
                auto *item = m_table->item(row, c);
                if (item) item->setBackground(QBrush(QColor(255, 248, 220)));
            }
        }
        ++row;
    }

    m_summary->setText(QStringLiteral("📋 共 %1 个保养任务（未执行 %2，已执行 %3）")
                           .arg(list.size()).arg(pendingCount).arg(doneCount));
}

// 文件内辅助：取当前选中行隐藏的任务id
static QString selectedTaskId(QTableWidget *table)
{
    const int row = table->currentRow();
    if (row < 0) return QString();
    return table->item(row, 0)->data(kTaskIdRole).toString();
}

// 执行选中保养任务
void MaintenancePage::onExecute()
{
    const QString taskId = selectedTaskId(m_table);
    if (taskId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条保养任务"));
        return;
    }

    // 二次确认
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
    emit dataChanged();   // 联动设备页（设备状态/使用次数可能变化）
}
