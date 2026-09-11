/**
 * @file    main.cpp
 * @brief   实验室设备预约管理系统 —— 算法/实现部分 演示程序。
 *
 * 本文件串起全部核心功能,并内置一组自我检查(每个检查点输出 [通过]/[失败]),
 * 直接运行即可验证算法正确性:
 *   1. 抽象基类与多态      —— User/Student/Teacher/Admin 的动态派发
 *   2. 预约流程            —— 教师自动通过、学生转人工、冲突拒绝
 *   3. 冲突检测与优先级    —— 半开区间相交、教师>学生、已审批>待审批、抢占
 *   4. 空闲时段            —— 扫描线算法求当日可用窗口
 *   5. 批量操作            —— 批量审批、CSV 批量导入(含去重/容错)
 *   6. 保养提醒            —— 周期型/次数型到期判定、生成与执行保养任务
 *   7. 统计与排行榜        —— 使用率、按时间/次数排序
 *
 * 为便于演示与自动化测试,时间以"注入"方式固定(依赖注入 + 固定时钟)。
 */

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "Admin.h"
#include "Equipment.h"
#include "MaintenanceTask.h"
#include "Reservation.h"
#include "ReservationManager.h"
#include "Student.h"
#include "Teacher.h"
#include "User.h"

static_assert(!std::is_copy_constructible<ReservationManager>::value,
              "ReservationManager 持有唯一所有权,应禁止拷贝");
static_assert(std::is_copy_constructible<Reservation>::value,
              "Reservation 应为可拷贝的值类型");
static_assert(std::is_base_of<User, Student>::value && std::is_base_of<User, Teacher>::value &&
                  std::is_base_of<User, Admin>::value,
              "三种角色都必须继承 User");

static int g_checked = 0;
static int g_failed = 0;

void check(bool ok, const std::string& what) {
    ++g_checked;
    std::cout << (ok ? "  [通过] " : "  [失败] ") << what << "\n";
    if (!ok) ++g_failed;
}

void section(const std::string& title) {
    std::cout << "\n========== " << title << " ==========\n";
}

int main() {
    using namespace std::string_literals;

    const DateTime fixed_now(2026, 9, 10, 9, 0);
    ReservationManager mgr([&] { return fixed_now; });

    section("1. 初始化:抽象基类 + 多态(用户)与设备");
    mgr.add_user(std::make_unique<Admin>(1, "李老师", "admin123"));
    mgr.add_user(std::make_unique<Teacher>(2, "王教授", "teach123"));
    mgr.add_user(std::make_unique<Student>(3, "张三", "stud123"));
    mgr.add_equipment("EQ-001", "电子显微镜", "EM-X100", CycleType::Days, 30);
    mgr.add_equipment("EQ-002", "高速离心机", "HSC-200", CycleType::Uses, 50);
    mgr.add_equipment("EQ-003", "数字示波器", "OSC-500", CycleType::Uses, 20);

    std::cout << "  用户列表(基类指针多态调用):\n";
    for (const auto& u : mgr.users()) {
        std::cout << "    " << u->to_string()
                  << "  自动审批=" << (u->canAutoApprove() ? "是" : "否") << "\n";
    }
    std::cout << "  设备列表:\n";
    for (const auto& e : mgr.equipments()) std::cout << "    " << *e << "\n";

    check(mgr.find_user(3)->canAutoApprove() == false, "多态:学生不可自动审批");
    check(mgr.find_user(2)->canAutoApprove() == true, "多态:教师自动审批");
    check(mgr.find_user(1)->canAutoApprove() == true, "多态:管理员自动审批");

    section("2. 预约流程:教师自动通过 / 学生转人工");
    ApplyResult r1 = mgr.apply_reservation(Reservation("R-0001", "EQ-001", 2,
                                                       DateTime(2026, 9, 10, 10, 0),
                                                       DateTime(2026, 9, 10, 12, 0)));
    std::cout << "  R-0001(教师): " << r1.to_string() << "\n";
    check(r1.status == ReservationStatus::Approved, "教师预约自动通过");

    ApplyResult r2 = mgr.apply_reservation(Reservation("R-0002", "EQ-001", 3,
                                                       DateTime(2026, 9, 10, 13, 0),
                                                       DateTime(2026, 9, 10, 14, 0)));
    std::cout << "  R-0002(学生): " << r2.to_string() << "\n";
    check(r2.status == ReservationStatus::Pending, "学生预约转人工(待审批)");

    section("3. 冲突检测与拒绝(附可用时段建议)");
    const Reservation probe("R-0003", "EQ-001", 3, DateTime(2026, 9, 10, 10, 30),
                            DateTime(2026, 9, 10, 11, 30));
    const auto conflicts = mgr.check_conflict(probe);
    std::cout << "  R-0003 与" << conflicts.size() << "条预约冲突:\n";
    for (const auto& c : conflicts) std::cout << "    -> " << c << "\n";
    check(conflicts.size() == 1 && conflicts[0].id() == "R-0001", "区间相交检测正确");

    ApplyResult r3 = mgr.apply_reservation(probe);
    std::cout << "  R-0003(学生,与已通过教师预约冲突): " << r3.to_string() << "\n";
    check(r3.status == ReservationStatus::Rejected, "学生与已通过预约冲突 → 拒绝");
    check(r3.message.find("可用时段示例") != std::string::npos, "拒绝提示附带可用时段建议");

    section("4. 优先级抢占:教师 > 待审批学生");
    ApplyResult r4 = mgr.apply_reservation(Reservation("R-0004", "EQ-001", 2,
                                                       DateTime(2026, 9, 10, 13, 30),
                                                       DateTime(2026, 9, 10, 14, 30)));
    std::cout << "  R-0004(教师,与学生待审批冲突): " << r4.to_string() << "\n";
    check(r4.status == ReservationStatus::Approved, "教师预约抢占成功(自动通过)");
    if (const Reservation* r = mgr.find_reservation("R-0002")) {
        check(r->status() == ReservationStatus::Rejected, "被抢占的学生预约已自动拒绝");
    } else {
        check(false, "R-0002 应仍存在");
    }

    ApplyResult r8 = mgr.apply_reservation(Reservation("R-0008", "EQ-001", 2,
                                                       DateTime(2026, 9, 10, 11, 0),
                                                       DateTime(2026, 9, 10, 12, 0)));
    std::cout << "  R-0008(教师,与已通过教师预约冲突): " << r8.to_string() << "\n";
    check(r8.status == ReservationStatus::Rejected, "同级(教师 vs 教师已审批)不抢占 → 拒绝");

    section("5. 空闲时段(扫描线算法)");
    const auto slots60 = mgr.get_available_slots("EQ-001", DateTime(2026, 9, 10, 0, 0), 60);
    std::cout << "  当日可用时段(min 60分钟):\n";
    for (const auto& [s, e] : slots60) std::cout << "    " << s << " ~ " << e << "\n";
    check(slots60.size() == 3, "空闲时段数量正确(3段)");
    check(slots60.front().first == DateTime(2026, 9, 10, 9, 0) &&
              slots60.front().second == DateTime(2026, 9, 10, 10, 0),
          "首个空闲段从当前时间(09:00)开始");
    check(mgr.get_available_slots("EQ-001", DateTime(2026, 9, 10, 0, 0), 120).size() == 1,
          "min_minutes=120 时仅保留足够长的时段");

    section("6. 批量审批");
    mgr.apply_reservation(Reservation("R-0005", "EQ-002", 3,
                                      DateTime(2026, 9, 10, 9, 0), DateTime(2026, 9, 10, 10, 0)));
    mgr.apply_reservation(Reservation("R-0006", "EQ-002", 3,
                                      DateTime(2026, 9, 10, 10, 0), DateTime(2026, 9, 10, 11, 0)));
    std::cout << "  R-0005 / R-0006 均已提交待审批\n";
    const int approved = mgr.batch_approve({"R-0005", "R-0006", "R-9999"});
    std::cout << "  批量审批返回: " << approved << " 条通过\n";
    check(approved == 2, "批量审批通过2条(不存在id被跳过)");

    section("7. 使用与归还(状态迁移)");
    mgr.update_equipment_status("EQ-002", EquipmentStatus::Borrowed);
    const bool completed = mgr.complete_reservation("R-0005", DateTime(2026, 9, 10, 10, 0));
    std::cout << "  R-0005 归还结果: " << (completed ? "成功" : "失败") << "\n";
    check(completed && mgr.find_reservation("R-0005")->status() == ReservationStatus::Returned,
          "归还后预约状态=已归还");
    check(mgr.find_equipment("EQ-002")->usage_count() == 1 &&
              mgr.find_equipment("EQ-002")->status() == EquipmentStatus::Available,
          "设备使用次数+1 且状态回到可用");

    section("8. 保养提醒(周期型 + 次数型)");
    for (int i = 0; i < 20; ++i) mgr.find_equipment("EQ-003")->record_usage();
    mgr.find_equipment("EQ-001")->reset_last_maintenance(DateTime(2026, 8, 1, 0, 0));
    check(mgr.find_equipment("EQ-003")->need_maintenance(fixed_now), "次数型设备到期判定正确");
    check(mgr.find_equipment("EQ-001")->need_maintenance(fixed_now), "周期型设备到期判定正确");

    const auto due_tasks = mgr.scan_due_maintenance();
    std::cout << "  扫描到 " << due_tasks.size() << " 个到期保养任务:\n";
    for (const auto& t : due_tasks) std::cout << "    " << t.to_string() << "\n";
    check(due_tasks.size() == 2, "自动生成2个保养任务");

    const bool exec1 = mgr.execute_maintenance_task("MT-0001");
    const bool exec2 = mgr.execute_maintenance_task("MT-0002");
    std::cout << "  执行保养任务: " << (exec1 ? "MT-0001 成功" : "MT-0001 失败") << " / "
              << (exec2 ? "MT-0002 成功" : "MT-0002 失败") << "\n";
    check(exec1 && exec2 && !mgr.find_equipment("EQ-001")->need_maintenance(fixed_now) &&
              mgr.find_equipment("EQ-003")->usage_count() == 0,
          "执行保养后设备恢复(不再到期)");

    section("9. 批量导入(CSV,含表头识别/去重/容错)");
    const std::filesystem::path csv_path =
        std::filesystem::temp_directory_path() / "labres_equipment_import.csv";
    {
        std::ofstream csv(csv_path);
        csv << "id,name,spec,status,cycle_type,cycle_value\n";
        csv << "EQ-004,PCR扩增仪,P-200,available,uses,100\n";
        csv << "EQ-005,恒温烘箱,H-300,borrowed,days,15\n";
        csv << "EQ-001,电子显微镜,EM-X100,available,days,30\n";
        csv << "EQ-006,坏行设备,X-1,available,monthly,30\n";
    }
    const ImportResult import = mgr.import_equipment_csv(csv_path.string());
    std::cout << "  导入结果: 新增" << import.added << " 跳过" << import.skipped
              << " 失败" << import.failed << "\n";
    for (const auto& err : import.errors) std::cout << "    " << err << "\n";
    check(import.added == 2 && import.skipped == 1 && import.failed == 1,
          "CSV导入:2新增/1重复跳过/1非法失败");
    check(mgr.equipments().size() == 5, "设备总数变为5台");
    std::filesystem::remove(csv_path);

    section("10. 统计与排行榜");
    const DateTime day_start(2026, 9, 10, 0, 0), day_end(2026, 9, 10, 23, 59);
    const double rate_eq001 = mgr.usage_rate("EQ-001", day_start, day_end);
    std::cout << "  EQ-001 当日使用率: " << rate_eq001 * 100.0 << "%\n";
    check(rate_eq001 > 0.12 && rate_eq001 < 0.13, "EQ-001 使用率=12.5%(计算正确)");

    std::cout << "  当日使用率排行榜:\n";
    const auto rank_time = mgr.ranking_by_time(day_start, day_end);
    for (const auto& [id, rate] : rank_time)
        std::cout << "    " << id << " -> " << rate * 100.0 << "%\n";
    check(!rank_time.empty() && rank_time.front().first == "EQ-001", "使用率榜首=EQ-001");

    const auto rank_count = mgr.ranking_by_count(day_start, day_end);
    std::cout << "  当日预约次数排行榜:\n";
    for (const auto& [id, cnt] : rank_count) std::cout << "    " << id << " -> " << cnt << " 次\n";
    check(!rank_count.empty() && rank_count.front().first == "EQ-001" &&
              rank_count.front().second == 2,
          "次数榜首=EQ-001(2次,并列按编号稳定排序)");

    section("11. 删除设备与最终清单");
    check(mgr.remove_equipment("EQ-005"), "删除设备 EQ-005 成功");
    std::cout << "  全部预约记录:\n";
    for (const auto& r : mgr.reservations()) std::cout << "    " << r << "\n";
    std::cout << "  全部设备:\n";
    for (const auto& e : mgr.equipments()) std::cout << "    " << *e << "\n";

    section("汇总");
    std::cout << "  共执行 " << g_checked << " 项检查,失败 " << g_failed << " 项。\n";
    return g_failed == 0 ? 0 : 1;
}
