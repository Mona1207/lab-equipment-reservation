/**
 * @file    Equipment.cpp
 * @brief   Equipment 设备实体实现。
 *
 * 设计要点（面向对象）：
 *  - 实体拥有自己的"状态"并维护不变量（状态迁移只通过 update_status()）；
 *  - 聚合了值类型 DateTime / 枚举作为属性，对外隐藏存储细节；
 *  - 业务判断（need_maintenance 的两种策略：周期天数/累计次数）内聚在对象内部，
 *    调用方无需了解保养规则细节 —— 高内聚；
 *  - 状态名、描述等展示逻辑（status_name / to_string）同样由对象自己负责。
 */

#include "Equipment.h"

#include <algorithm>

Equipment::Equipment(const std::string& id, const std::string& name, const std::string& spec,
                     CycleType cycle_type, int cycle_value, EquipmentStatus status)
    : id_(id),
      name_(name),
      spec_(spec),
      status_(status),
      cycle_type_(cycle_type),
      cycle_value_(std::max(cycle_value, 1)),
      usage_count_(0),
      last_maintenance_(DateTime::now()) {}

const std::string& Equipment::id() const { return id_; }
const std::string& Equipment::name() const { return name_; }
const std::string& Equipment::spec() const { return spec_; }

EquipmentStatus Equipment::status() const { return status_; }
void Equipment::update_status(EquipmentStatus status) { status_ = status; }

CycleType Equipment::cycle_type() const { return cycle_type_; }
int Equipment::cycle_value() const { return cycle_value_; }

int Equipment::usage_count() const { return usage_count_; }
const DateTime& Equipment::last_maintenance() const { return last_maintenance_; }

void Equipment::record_usage() {
    ++usage_count_;
}

bool Equipment::need_maintenance(const DateTime& now) const {
    if (status_ == EquipmentStatus::Maintenance) {
        return false;
    }
    if (cycle_type_ == CycleType::Days) {
        const long long elapsed_minutes = now - last_maintenance_;
        return elapsed_minutes >= static_cast<long long>(cycle_value_) * 24 * 60;
    }
    return usage_count_ >= cycle_value_;
}

void Equipment::reset_usage_count() { usage_count_ = 0; }
void Equipment::reset_last_maintenance(const DateTime& when) { last_maintenance_ = when; }

std::string Equipment::status_name() const {
    switch (status_) {
        case EquipmentStatus::Available:   return "可用";
        case EquipmentStatus::Borrowed:    return "已借出";
        case EquipmentStatus::Maintenance: return "维护中";
    }
    return "未知";
}

std::string Equipment::to_string() const {
    std::string cycle = (cycle_type_ == CycleType::Days)
                            ? std::string("每") + std::to_string(cycle_value_) + "天"
                            : std::string("每") + std::to_string(cycle_value_) + "次";
    return id_ + " " + name_ + "(" + spec_ + ") 状态:" + status_name() +
           " 保养:" + cycle + " 已使用:" + std::to_string(usage_count_) + "次";
}

std::ostream& operator<<(std::ostream& os, const Equipment& eq) {
    return os << eq.to_string();
}
