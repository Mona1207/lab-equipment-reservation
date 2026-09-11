/**
 * @file    MaintenanceTask.cpp
 * @brief   MaintenanceTask 保养任务实体实现。
 *
 * 设计要点（面向对象）：
 *  - 保养任务记录"建议值"（scheduled_usage：生成任务时设备已使用次数），
 *    执行完成后只有一个 executed 标志位，任务本身是不可变记录（除状态位）；
 *  - is_due() 封装到期判定：未执行 且 计划日期不晚于当前时间。
 */

#include "MaintenanceTask.h"

MaintenanceTask::MaintenanceTask(const std::string& id, const std::string& equipment_id,
                                 MaintenanceType type, const DateTime& scheduled_date,
                                 int scheduled_usage)
    : id_(id),
      equipment_id_(equipment_id),
      type_(type),
      scheduled_date_(scheduled_date),
      scheduled_usage_(scheduled_usage),
      executed_(false) {}

const std::string& MaintenanceTask::id() const { return id_; }
const std::string& MaintenanceTask::equipment_id() const { return equipment_id_; }
MaintenanceType MaintenanceTask::type() const { return type_; }
const DateTime& MaintenanceTask::scheduled_date() const { return scheduled_date_; }
int MaintenanceTask::scheduled_usage() const { return scheduled_usage_; }

bool MaintenanceTask::executed() const { return executed_; }
void MaintenanceTask::mark_executed() { executed_ = true; }

bool MaintenanceTask::is_due(const DateTime& now) const {
    return !executed_ && scheduled_date_ <= now;
}

std::string MaintenanceTask::to_string() const {
    return id_ + " 设备" + equipment_id_ + " " +
           (type_ == MaintenanceType::Periodic ? "周期保养" : "次数保养") +
           " 计划:" + scheduled_date_.to_string() +
           (executed_ ? " [已执行]" : " [未执行]");
}
