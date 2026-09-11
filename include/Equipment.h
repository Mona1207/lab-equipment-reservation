#pragma once
#include <string>
#include "DateTime.h"

// 说明：本头文件按 INTERFACE.md 契约补齐（算法组 src/Equipment.cpp 已依赖此声明）。

enum class EquipmentStatus { Available, Borrowed, Maintenance };  // 可用 / 已借出 / 维护中
enum class CycleType { Days, Uses };                              // 按天数 / 按使用次数

class Equipment {
public:
    Equipment(const std::string& id, const std::string& name, const std::string& spec,
              CycleType cycle_type, int cycle_value,
              EquipmentStatus status = EquipmentStatus::Available);

    const std::string& id() const;
    const std::string& name() const;
    const std::string& spec() const;

    EquipmentStatus status() const;
    void update_status(EquipmentStatus status);   // README: update_status()

    CycleType cycle_type() const;
    int cycle_value() const;                      // 天数 或 次数（构造时至少为1）

    int usage_count() const;                      // 累计使用次数
    const DateTime& last_maintenance() const;     // 上次保养时间（构造时=当前时间）

    void record_usage();                          // 归还时调用：使用次数 +1
    bool need_maintenance(const DateTime& now = DateTime::now()) const;  // README: need_maintenance()
    void reset_usage_count();                     // 按次数保养完成后清零
    void reset_last_maintenance(const DateTime& when);  // 按周期保养完成后重置

    std::string status_name() const;              // "可用"/"已借出"/"维护中"
    std::string to_string() const;

private:
    std::string id_, name_, spec_;
    EquipmentStatus status_;
    CycleType cycle_type_;
    int cycle_value_;
    int usage_count_ = 0;
    DateTime last_maintenance_;
};

std::ostream& operator<<(std::ostream& os, const Equipment& eq);
