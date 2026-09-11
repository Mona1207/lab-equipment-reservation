/**
 * @file    ReservationManager.cpp
 * @brief   ReservationManager 核心业务类实现 —— 本项目"算法"部分的主战场。
 *
 * 面向对象设计要点：
 *  - 【组合】管理类聚合用户 / 设备 / 预约 / 维护任务四类对象的容器，是系统的"门面"；
 *  - 【所有权】用户与设备以 std::unique_ptr 持有（防止对象切片、明确单点所有权），
 *    预约以 std::deque 按值存放（push_back 不使既有元素引用失效，可按 id 安全取指针）；
 *  - 【依赖注入】通过 std::function<DateTime()> 时钟注入"当前时间"，
 *    业务逻辑自身不依赖真实时钟，可测试、可演示（配合固定时钟完全可复现）；
 *  - 【多态驱动】审批/抢占决策调用 User::canAutoApprove()，新增角色无需改动本类；
 *  - 【值类型结果】ApplyResult / ImportResult 为轻量值类型，表达"结果"而非"副作用"；
 *  - 【禁止拷贝】持有 unique_ptr 的容器天然不可拷贝，显式 delete 拷贝构造/赋值，
 *    表达管理对象的"唯一拥有"语义。
 *
 * 核心算法：
 *  1) 区间冲突检测：半开区间相交判定（见 Reservation::overlaps），O(n) 扫描；
 *  2) 优先级抢占：教师/管理员 > 学生（canAutoApprove 即角色优先级），已审批 > 待审批，
 *     只有"新预约高优先级且冲突方全部为学生待审批"时才自动调整（拒绝冲突、通过新预约）；
 *  3) 空闲时段：先收集占用区间，排序后扫描线（sweep line）求空隙，输出长度≥min_minutes 的空隙；
 *  4) 保养扫描：按设备保养规则（周期/次数）检查 need_maintenance，自动生成未执行任务；
 *  5) 统计：使用率 = 区间内占用时长 / 区间总时长；排行榜按使用率 / 次数排序（std::sort + lambda）。
 */

#include "ReservationManager.h"

#include <algorithm>
#include <cstdio>
#include <cctype>
#include <fstream>
#include <sstream>

namespace {

constexpr int kWorkStartMinutes = 8 * 60;
constexpr int kWorkEndMinutes = 22 * 60;

const char* status_label(ReservationStatus s) {
    switch (s) {
        case ReservationStatus::Pending:  return "待审批";
        case ReservationStatus::Approved: return "已通过";
        case ReservationStatus::Rejected: return "已拒绝";
        case ReservationStatus::Returned: return "已归还";
    }
    return "未知";
}

bool is_student(const User* u) {
    return u != nullptr && !u->canAutoApprove();
}

std::string trim(const std::string& s) {
    std::string r = s;
    r.erase(r.begin(), std::find_if(r.begin(), r.end(),
                                    [](unsigned char c) { return !std::isspace(c); }));
    r.erase(std::find_if(r.rbegin(), r.rend(),
                         [](unsigned char c) { return !std::isspace(c); })
                .base(),
            r.end());
    return r;
}

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (in_quotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    cur.push_back('"');
                    ++i;
                } else {
                    in_quotes = false;
                }
            } else {
                cur.push_back(c);
            }
        } else {
            if (c == '"') {
                in_quotes = true;
            } else if (c == ',') {
                fields.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
    }
    fields.push_back(cur);
    for (auto& f : fields) {
        f = trim(f);
    }
    return fields;
}

EquipmentStatus parse_equipment_status(const std::string& s) {
    const std::string t = lower(trim(s));
    if (t == "borrowed" || t == "已借出" || t == "借出") return EquipmentStatus::Borrowed;
    if (t == "maintenance" || t == "maintain" || t == "维护中" || t == "维修") return EquipmentStatus::Maintenance;
    return EquipmentStatus::Available;
}

bool parse_cycle_type(const std::string& s, CycleType& out) {
    const std::string t = lower(trim(s));
    if (t == "days" || t == "day" || t == "d" || t == "天" || t == "周期") {
        out = CycleType::Days;
        return true;
    }
    if (t == "uses" || t == "use" || t == "u" || t == "次" || t == "次数") {
        out = CycleType::Uses;
        return true;
    }
    return false;
}

} // namespace

ApplyResult::ApplyResult(ReservationStatus s, std::string msg)
    : status(s), message(std::move(msg)) {}

std::string ApplyResult::to_string() const {
    std::string s = std::string(status_label(status)) + " | " + message;
    if (!affected_ids.empty()) {
        s += "（被调整预约:";
        for (size_t i = 0; i < affected_ids.size(); ++i) {
            s += (i ? "、" : "") + affected_ids[i];
        }
        s += "）";
    }
    return s;
}

ReservationManager::ReservationManager(Clock clock) : clock_(std::move(clock)) {}

std::string ReservationManager::next_id(const std::string& prefix) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "-%04d", next_sequence_++);
    return prefix + buf;
}

User* ReservationManager::add_user(std::unique_ptr<User> user) {
    users_.push_back(std::move(user));
    return users_.back().get();
}

User* ReservationManager::find_user(int id) {
    const auto it = std::find_if(users_.begin(), users_.end(),
                                 [&](const std::unique_ptr<User>& u) { return u->getId() == id; });
    return it == users_.end() ? nullptr : it->get();
}

const std::vector<std::unique_ptr<User>>& ReservationManager::users() const { return users_; }

Equipment* ReservationManager::add_equipment(const std::string& id, const std::string& name,
                                             const std::string& spec, CycleType cycle_type,
                                             int cycle_value, EquipmentStatus status) {
    equipments_.push_back(std::make_unique<Equipment>(id, name, spec, cycle_type, cycle_value, status));
    return equipments_.back().get();
}

Equipment* ReservationManager::find_equipment(const std::string& id) {
    const auto it = std::find_if(equipments_.begin(), equipments_.end(),
                                 [&](const std::unique_ptr<Equipment>& e) { return e->id() == id; });
    return it == equipments_.end() ? nullptr : it->get();
}

bool ReservationManager::update_equipment_status(const std::string& id, EquipmentStatus status) {
    Equipment* eq = find_equipment(id);
    if (!eq) return false;
    eq->update_status(status);
    return true;
}

bool ReservationManager::update_equipment(const std::string& id, const std::string& name,
                                           const std::string& spec, CycleType cycle_type, int cycle_value) {
    Equipment* eq = find_equipment(id);
    if (!eq) return false;
    const EquipmentStatus old_status = eq->status();
    const int old_usage = eq->usage_count();
    const DateTime old_maint = eq->last_maintenance();
    *eq = Equipment(id, name, spec, cycle_type, cycle_value, old_status);
    eq->reset_usage_count();
    for (int i = 0; i < old_usage; ++i) eq->record_usage();
    eq->reset_last_maintenance(old_maint);
    return true;
}

bool ReservationManager::remove_equipment(const std::string& id) {
    const auto it = std::find_if(equipments_.begin(), equipments_.end(),
                                 [&](const std::unique_ptr<Equipment>& e) { return e->id() == id; });
    if (it == equipments_.end()) return false;
    equipments_.erase(it);
    return true;
}

const std::vector<std::unique_ptr<Equipment>>& ReservationManager::equipments() const { return equipments_; }

const Reservation* ReservationManager::find_reservation(const std::string& id) const {
    const auto it = std::find_if(reservations_.begin(), reservations_.end(),
                                 [&](const Reservation& r) { return r.id() == id; });
    return it == reservations_.end() ? nullptr : &(*it);
}

std::vector<Reservation> ReservationManager::check_conflict(const Reservation& new_reservation) const {
    std::vector<Reservation> conflicts;
    for (const Reservation& r : reservations_) {
        if (r.status() != ReservationStatus::Pending &&
            r.status() != ReservationStatus::Approved) {
            continue;
        }
        if (r.equipment_id() != new_reservation.equipment_id()) continue;
        if (r.overlaps(new_reservation)) {
            conflicts.push_back(r);
        }
    }
    return conflicts;
}

ApplyResult ReservationManager::apply_reservation(Reservation new_reservation) {
    const DateTime now = clock_();

    const User* user = find_user(new_reservation.user_id());
    if (!user) {
        return ApplyResult(ReservationStatus::Rejected,
                           "预约失败：用户不存在 (" + std::to_string(new_reservation.user_id()) + ")");
    }
    if (!find_equipment(new_reservation.equipment_id())) {
        return ApplyResult(ReservationStatus::Rejected,
                           "预约失败：设备不存在 (" + new_reservation.equipment_id() + ")");
    }
    if (!(new_reservation.end_time() > new_reservation.start_time())) {
        return ApplyResult(ReservationStatus::Rejected, "预约失败：结束时间必须晚于开始时间");
    }
    if (new_reservation.start_time() < now) {
        return ApplyResult(ReservationStatus::Rejected,
                           "预约失败：不允许预约过去的时间（当前 " + now.to_string() + "）");
    }

    const std::vector<Reservation> conflicts = check_conflict(new_reservation);
    if (conflicts.empty()) {
        const ReservationStatus target =
            user->canAutoApprove() ? ReservationStatus::Approved : ReservationStatus::Pending;
        const std::string message = user->canAutoApprove()
                                        ? "自动通过（" + user->role_name() + "角色免审批）"
                                        : "已提交，等待管理员审批";
        reservations_.push_back(std::move(new_reservation));
        reservations_.back().set_status(target);
        return ApplyResult(target, message);
    }

    const bool can_preempt =
        !is_student(user) &&
        std::all_of(conflicts.begin(), conflicts.end(), [&](const Reservation& c) {
            return c.status() == ReservationStatus::Pending && is_student(find_user(c.user_id()));
        });

    if (can_preempt) {
        ApplyResult result(ReservationStatus::Approved,
                           "自动通过（冲突方为低优先级学生待审批，按 教师>学生 规则抢占）");
        for (const Reservation& c : conflicts) {
            for (Reservation& stored : reservations_) {
                if (stored.id() == c.id()) {
                    stored.set_status(ReservationStatus::Rejected);
                    result.affected_ids.push_back(c.id());
                    break;
                }
            }
        }
        reservations_.push_back(std::move(new_reservation));
        reservations_.back().set_status(ReservationStatus::Approved);
        return result;
    }

    std::string message = "预约被拒绝：与已有 " + std::to_string(conflicts.size()) +
                          " 条预约时间冲突（规则：教师>学生，已审批>待审批）";
    const auto slots =
        get_available_slots(new_reservation.equipment_id(), new_reservation.start_time(),
                            static_cast<int>(new_reservation.duration_minutes()));
    if (!slots.empty()) {
        message += "；当日可用时段示例：";
        const size_t kShow = std::min<size_t>(slots.size(), 3);
        for (size_t i = 0; i < kShow; ++i) {
            message += (i ? "、" : "") + slots[i].first.to_string() + "~" + slots[i].second.to_string();
        }
    }
    return ApplyResult(ReservationStatus::Rejected, message);
}

std::vector<std::pair<DateTime, DateTime>> ReservationManager::get_available_slots(
    const std::string& equipment_id, const DateTime& date, int min_minutes) const {
    std::vector<std::pair<DateTime, DateTime>> free_slots;
    const DateTime now = clock_();

    const long long day0 = date.to_minutes() - (date.hour() * 60 + date.minute());
    long long win_start = day0 + kWorkStartMinutes;
    const long long win_end = day0 + kWorkEndMinutes;
    if (date.same_day(now)) {
        win_start = std::max(win_start, now.to_minutes());
    }
    if (win_start >= win_end) {
        return free_slots;
    }

    std::vector<std::pair<long long, long long>> occupied;
    for (const Reservation& r : reservations_) {
        if (r.equipment_id() != equipment_id) continue;
        if (r.status() != ReservationStatus::Pending &&
            r.status() != ReservationStatus::Approved) {
            continue;
        }
        const long long s = r.start_time().to_minutes();
        const long long e = r.end_time().to_minutes();
        if (e <= win_start || s >= win_end) continue;
        occupied.push_back({std::max(s, win_start), std::min(e, win_end)});
    }

    std::sort(occupied.begin(), occupied.end());
    long long cursor = win_start;
    for (const auto& [s, e] : occupied) {
        if (s > cursor && s - cursor >= min_minutes) {
            free_slots.emplace_back(DateTime::from_minutes(cursor), DateTime::from_minutes(s));
        }
        cursor = std::max(cursor, e);
    }
    if (win_end > cursor && win_end - cursor >= min_minutes) {
        free_slots.emplace_back(DateTime::from_minutes(cursor), DateTime::from_minutes(win_end));
    }
    return free_slots;
}

int ReservationManager::batch_approve(const std::vector<std::string>& reservation_ids) {
    int approved = 0;
    for (const std::string& id : reservation_ids) {
        for (Reservation& r : reservations_) {
            if (r.id() == id && r.status() == ReservationStatus::Pending) {
                r.set_status(ReservationStatus::Approved);
                ++approved;
                break;
            }
        }
    }
    return approved;
}

bool ReservationManager::complete_reservation(const std::string& reservation_id, const DateTime& when) {
    (void)when;
    for (Reservation& r : reservations_) {
        if (r.id() != reservation_id) continue;
        if (r.status() != ReservationStatus::Approved) return false;
        r.set_status(ReservationStatus::Returned);
        Equipment* eq = find_equipment(r.equipment_id());
        if (eq) {
            eq->record_usage();
            if (eq->status() == EquipmentStatus::Borrowed) {
                eq->update_status(EquipmentStatus::Available);
            }
        }
        return true;
    }
    return false;
}

bool ReservationManager::cancel_reservation(const std::string& reservation_id) {
    for (Reservation& r : reservations_) {
        if (r.id() != reservation_id) continue;
        if (r.status() != ReservationStatus::Pending &&
            r.status() != ReservationStatus::Approved) {
            return false;
        }
        r.set_status(ReservationStatus::Cancelled);
        return true;
    }
    return false;
}

const std::deque<Reservation>& ReservationManager::reservations() const { return reservations_; }

std::vector<MaintenanceTask> ReservationManager::scan_due_maintenance() {
    const DateTime now = clock_();
    std::vector<MaintenanceTask> due;

    for (const std::unique_ptr<Equipment>& eq : equipments_) {
        if (!eq->need_maintenance(now)) continue;
        const bool has_open_task = std::any_of(
            maintenance_tasks_.begin(), maintenance_tasks_.end(),
            [&](const MaintenanceTask& t) { return t.equipment_id() == eq->id() && !t.executed(); });
        if (has_open_task) continue;

        const MaintenanceType type =
            eq->cycle_type() == CycleType::Days ? MaintenanceType::Periodic : MaintenanceType::UsageBased;
        const MaintenanceTask task(next_id("MT"), eq->id(), type, now, eq->usage_count());
        maintenance_tasks_.push_back(task);
        due.push_back(task);
    }

    for (const MaintenanceTask& t : maintenance_tasks_) {
        if (!t.is_due(now)) continue;
        const bool already = std::any_of(due.begin(), due.end(),
                                         [&](const MaintenanceTask& d) { return d.id() == t.id(); });
        if (!already) due.push_back(t);
    }

    std::sort(due.begin(), due.end(), [](const MaintenanceTask& a, const MaintenanceTask& b) {
        return a.equipment_id() < b.equipment_id();
    });
    return due;
}

bool ReservationManager::execute_maintenance_task(const std::string& task_id) {
    for (MaintenanceTask& t : maintenance_tasks_) {
        if (t.id() != task_id) continue;
        if (t.executed()) return false;
        Equipment* eq = find_equipment(t.equipment_id());
        if (!eq) return false;

        if (t.type() == MaintenanceType::Periodic) {
            eq->reset_last_maintenance(clock_());
        } else {
            eq->reset_usage_count();
        }
        t.mark_executed();
        return true;
    }
    return false;
}

const std::vector<MaintenanceTask>& ReservationManager::maintenance_tasks() const { return maintenance_tasks_; }

double ReservationManager::usage_rate(const std::string& equipment_id, const DateTime& from,
                                      const DateTime& to) const {
    const long long span = to - from;
    if (span <= 0) return 0.0;

    long long used = 0;
    for (const Reservation& r : reservations_) {
        if (r.equipment_id() != equipment_id) continue;
        if (r.status() != ReservationStatus::Approved &&
            r.status() != ReservationStatus::Returned) {
            continue;
        }
        const long long s = std::max(r.start_time().to_minutes(), from.to_minutes());
        const long long e = std::min(r.end_time().to_minutes(), to.to_minutes());
        if (e > s) used += e - s;
    }
    return std::min(1.0, static_cast<double>(used) / static_cast<double>(span));
}

std::vector<std::pair<std::string, double>> ReservationManager::ranking_by_time(
    const DateTime& from, const DateTime& to) const {
    std::vector<std::pair<std::string, double>> ranking;
    for (const std::unique_ptr<Equipment>& eq : equipments_) {
        ranking.emplace_back(eq->id(), usage_rate(eq->id(), from, to));
    }
    std::sort(ranking.begin(), ranking.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    return ranking;
}

std::vector<std::pair<std::string, int>> ReservationManager::ranking_by_count(
    const DateTime& from, const DateTime& to) const {
    std::vector<std::pair<std::string, int>> ranking;
    for (const std::unique_ptr<Equipment>& eq : equipments_) {
        int count = 0;
        for (const Reservation& r : reservations_) {
            if (r.equipment_id() != eq->id()) continue;
            if (r.status() != ReservationStatus::Approved &&
                r.status() != ReservationStatus::Returned) {
                continue;
            }
            if (r.end_time().to_minutes() > from.to_minutes() &&
                r.start_time().to_minutes() < to.to_minutes()) {
                ++count;
            }
        }
        ranking.emplace_back(eq->id(), count);
    }
    std::sort(ranking.begin(), ranking.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    return ranking;
}

ImportResult ReservationManager::import_equipment_csv(const std::string& path) {
    ImportResult result;
    std::ifstream in(path);
    if (!in) {
        ++result.failed;
        result.errors.push_back("无法打开文件: " + path);
        return result;
    }

    std::string line;
    int line_no = 0;
    bool header_checked = false;
    constexpr size_t kMaxErrors = 20;

    auto record_error = [&](const std::string& msg) {
        ++result.failed;
        if (result.errors.size() < kMaxErrors) result.errors.push_back(msg);
    };

    while (std::getline(in, line)) {
        ++line_no;
        if (trim(line).empty()) continue;
        const std::vector<std::string> f = split_csv_line(line);

        if (!header_checked) {
            header_checked = true;
            const std::string first = lower(f.empty() ? "" : f[0]);
            if (first == "id" || first.find("编号") != std::string::npos) {
                continue;
            }
        }

        if (f.size() < 6) {
            record_error("第 " + std::to_string(line_no) + " 行：字段不足（期望6列：" +
                         "id,name,spec,status,cycle_type,cycle_value）");
            continue;
        }

        if (find_equipment(f[0])) {
            ++result.skipped;
            if (result.errors.size() < kMaxErrors) {
                result.errors.push_back("第 " + std::to_string(line_no) + " 行：设备编号已存在 → " + f[0]);
            }
            continue;
        }

        CycleType cycle_type{};
        if (!parse_cycle_type(f[4], cycle_type)) {
            record_error("第 " + std::to_string(line_no) + " 行：无法识别保养周期类型 " + f[4] +
                         "（可用: days/uses 或 天/次）");
            continue;
        }

        int cycle_value = 0;
        if (!(std::istringstream(f[5]) >> cycle_value) || cycle_value <= 0) {
            record_error("第 " + std::to_string(line_no) + " 行：保养周期值非法 → " + f[5]);
            continue;
        }

        add_equipment(f[0], f[1], f[2], cycle_type, cycle_value,
                      parse_equipment_status(f[3]));
        ++result.added;
    }
    return result;
}
