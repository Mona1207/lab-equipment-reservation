// ============================================================
// 文件说明：MaintenancePage.cpp —— 保养管理页具体实现
// ============================================================

#include "MaintenancePage.h"
#include "AppContext.h"
#include "Theme.h"

#include "MaintenanceTask.h"

// 引入 Qt 控件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>

// ============================================================
// 构造函数：创建保养管理页
// ============================================================
MaintenancePage::MaintenancePage(QWidget *parent)
    : QWidget(parent)
{
    // ---- 顶部按钮行 ----
    m_executeBtn = new QPushButton(QStringLiteral("✔ 执行保养"), this);
    QPushButton *refreshBtn = new QPushButton(QStringLiteral("🔄 刷新"), this);

    connect(m_executeBtn, &QPushButton::clicked, this, &MaintenancePage::onExecute);
    connect(refreshBtn, &QPushButton::clicked, this, &MaintenancePage::onRefresh);

    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(m_executeBtn);
    btnRow->addWidget(refreshBtn);
    btnRow->addStretch();

    // ---- 保养任务表格（5列：任务ID、设备ID、触发原因、到期时间、状态）----
    m_table = new QTableWidget(0, 5, this);
    m_table->setHorizontalHeaderLabels(
        { QStringLiteral("任务ID"), QStringLiteral("设备ID"),
          QStringLiteral("触发原因"), QStringLiteral("到期时间"), QStringLiteral("状态") });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 只读
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);  // 斑马纹
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // ---- 整体垂直布局 ----
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(16);
    lay->addLayout(btnRow);  // 顶部按钮
    lay->addWidget(m_table); // 表格（拉伸占满）

    refreshTable();  // 初始加载
}

// ============================================================
// refreshTable()：刷新保养任务表格
// ============================================================
void MaintenancePage::refreshTable()
{
    // 先扫描一下，生成新的到期保养任务
    AppContext::get().manager().scan_due_maintenance();

    // 取所有保养任务
    const auto& tasks = AppContext::get().manager().maintenance_tasks();
    m_table->setRowCount(static_cast<int>(tasks.size()));

    // 填充每一行
    for (int i = 0; i < static_cast<int>(tasks.size()); ++i) {
        const MaintenanceTask& t = tasks.at(i);

        // 第0列：任务ID
        m_table->setItem(i, 0, new QTableWidgetItem(
            AppContext::toQString(t.id())));
        // 第1列：设备ID
        m_table->setItem(i, 1, new QTableWidgetItem(
            AppContext::toQString(t.equipment_id())));
        // 第2列：触发原因（文字说明）
        m_table->setItem(i, 2, new QTableWidgetItem(
            AppContext::toQString(t.description())));
        // 第3列：到期时间
        m_table->setItem(i, 3, new QTableWidgetItem(
            AppContext::toQString(t.due_time().to_string())));

        // 第4列：状态（已执行=灰色，待执行=黄色）
        QTableWidgetItem *statusItem;
        if (t.executed()) {
            statusItem = new QTableWidgetItem(QStringLiteral("已完成"));
            statusItem->setForeground(QColor(120, 120, 120));
        } else {
            statusItem = Theme::makeStatusItem(QStringLiteral("待保养"));
        }
        m_table->setItem(i, 4, statusItem);
    }
}

// ============================================================
// onExecute()：执行保养
// 把选中的保养任务标记为已完成，设备状态恢复为可用
// ============================================================
void MaintenancePage::onExecute()
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请先选中一条保养任务"));
        return;
    }

    // 从表格里取任务ID
    const std::string taskId = m_table->item(row, 0)->text().toStdString();

    // 调用核心函数执行保养
    if (!AppContext::get().manager().execute_maintenance(taskId)) {
        QMessageBox::warning(this, QStringLiteral("操作失败"),
                             QStringLiteral("该保养任务无法执行，可能已完成或不存在。"));
        return;
    }

    QMessageBox::information(this, QStringLiteral("保养完成"),
                             QStringLiteral("保养任务已完成，设备状态已更新为可用。"));
    refreshTable();
}

// 刷新按钮
void MaintenancePage::onRefresh()
{
    refreshTable();
}
