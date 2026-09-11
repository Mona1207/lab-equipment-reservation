#pragma once
#include <string>
#include "DateTime.h"

// 说明：本头文件按 INTERFACE.md 契约补齐（算法组 src/MaintenanceTask.cpp 已依赖此声明）。

enum class MaintenanceType { Periodic, UsageBased };  // 周期保养 / 次数保养

class MaintenanceTask {
public:
    MaintenanceTask(const std::string& id, const std::string& equipment_id,
                    MaintenanceType type, const DateTime& scheduled_date,
                    int scheduled_usage = 0);          // scheduled_usage：创建时设备已使用次数

    const std::string& id() const;
    const std::string& equipment_id() const;
    MaintenanceType type() const;
    const DateTime& scheduled_date() const;
    int scheduled_usage() const;

    bool executed() const;
    void mark_executed();

    bool is_due(const DateTime& now) const;            // 未执行 且 计划日期已到

    std::string to_string() const;

private:
    std::string id_, equipment_id_;
    MaintenanceType type_;
    DateTime scheduled_date_;
    int scheduled_usage_ = 0;
    bool executed_ = false;
};
