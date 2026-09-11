#pragma once
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <functional>
#include <utility>
#include "DateTime.h"
#include "User.h"
#include "Equipment.h"
#include "Reservation.h"
#include "MaintenanceTask.h"

// 说明：本头文件按 INTERFACE.md 契约补齐（算法组 src/ReservationManager.cpp 已依赖此声明）。

// 预约申请结果（值类型）
struct ApplyResult {
    ReservationStatus status = ReservationStatus::Rejected;
    std::string message;                    // 面向用户的中文提示
    std::vector<std::string> affected_ids;  // 被自动调整(拒绝)的冲突预约

    ApplyResult() = default;
    ApplyResult(ReservationStatus s, std::string msg);
    std::string to_string() const;          // "已通过 | 自动通过（教师 角色免审批）"
};

// CSV 批量导入结果
struct ImportResult {
    int added = 0;                          // 新增
    int skipped = 0;                        // 跳过（重复编号等）
    int failed = 0;                         // 失败（行格式错误）
    std::vector<std::string> errors;        // 具体错误（含行号，最多记录20条）
};

class ReservationManager {
public:
    using Clock = std::function<DateTime()>;   // 依赖注入：可注入时钟，便于测试/演示
    explicit ReservationManager(Clock clock = [] { return DateTime::now(); });

    ReservationManager(const ReservationManager&) = delete;            // 持有 unique_ptr，禁止拷贝
    ReservationManager& operator=(const ReservationManager&) = delete;

    // ---- 用户 ----
    User* add_user(std::unique_ptr<User> user);              // 所有权转移，内部以 unique_ptr 持有
    User* find_user(int id);                                 // 与 User::getId() 一致
    const std::vector<std::unique_ptr<User>>& users() const;

    // ---- 设备（增删改查）----
    Equipment* add_equipment(const std::string& id, const std::string& name,
                             const std::string& spec, CycleType cycle_type, int cycle_value,
                             EquipmentStatus status = EquipmentStatus::Available);
    Equipment* find_equipment(const std::string& id);
    bool update_equipment_status(const std::string& id, EquipmentStatus status);
    bool update_equipment(const std::string& id, const std::string& name,
                          const std::string& spec, CycleType cycle_type, int cycle_value);
    bool remove_equipment(const std::string& id);            // 相关历史预约保留
    const std::vector<std::unique_ptr<Equipment>>& equipments() const;

    // ---- 预约 ----
    const Reservation* find_reservation(const std::string& id) const;
    std::vector<Reservation> check_conflict(const Reservation& new_reservation) const;
    ApplyResult apply_reservation(Reservation new_reservation);        // 自动审批/转人工/抢占/拒绝
    std::vector<std::pair<DateTime, DateTime>> get_available_slots(
        const std::string& equipment_id, const DateTime& date, int min_minutes = 60) const;
    int batch_approve(const std::vector<std::string>& reservation_ids); // 批量审批，返回通过数
    bool complete_reservation(const std::string& reservation_id, const DateTime& when); // 归还
    bool cancel_reservation(const std::string& reservation_id);   // 取消预约（待审批/已通过→已取消，释放时段）
    const std::deque<Reservation>& reservations() const;

    // ---- 维护 ----
    std::vector<MaintenanceTask> scan_due_maintenance();    // 扫描并自动生成到期任务
    bool execute_maintenance_task(const std::string& task_id);
    const std::vector<MaintenanceTask>& maintenance_tasks() const;

    // ---- 统计 ----
    double usage_rate(const std::string& equipment_id, const DateTime& from, const DateTime& to) const;
    std::vector<std::pair<std::string, double>> ranking_by_time(const DateTime& from, const DateTime& to) const;
    std::vector<std::pair<std::string, int>> ranking_by_count(const DateTime& from, const DateTime& to) const;

    // ---- 批量导入（CSV：id,name,spec,status,cycle_type,cycle_value）----
    ImportResult import_equipment_csv(const std::string& path);

private:
    Clock clock_;
    std::vector<std::unique_ptr<User>> users_;
    std::vector<std::unique_ptr<Equipment>> equipments_;
    std::deque<Reservation> reservations_;       // 用 deque：push_back 不使引用失效
    std::vector<MaintenanceTask> maintenance_tasks_;
    int next_sequence_ = 1;
    std::string next_id(const std::string& prefix);   // "MT-0001"
};
