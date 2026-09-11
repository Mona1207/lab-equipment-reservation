#pragma once
#include <string>
#include "DateTime.h"

// 说明：本头文件按 INTERFACE.md 契约补齐（算法组 src/Reservation.cpp 已依赖此声明）。

enum class ReservationStatus { Pending, Approved, Rejected, Returned, Cancelled }; // 待审批/已通过/已拒绝/已归还/已取消

class Reservation {
public:
    Reservation(const std::string& id, const std::string& equipment_id,
                int user_id, const DateTime& start_time, const DateTime& end_time);

    const std::string& id() const;
    const std::string& equipment_id() const;
    int user_id() const;                 // 外键：对应 User::getId()（.h 组冻结为 int）
    const DateTime& start_time() const;
    const DateTime& end_time() const;

    ReservationStatus status() const;
    void set_status(ReservationStatus status);    // 初始为 Pending
    std::string status_name() const;              // "待审批"/"已通过"/"已拒绝"/"已归还"

    bool overlaps(const Reservation& other) const;     // 半开区间相交：[s1,e1) ∩ [s2,e2)
    long long duration_minutes() const;                // end - start（分钟）

    std::string to_string() const;

private:
    std::string id_, equipment_id_;
    int user_id_;
    DateTime start_time_, end_time_;
    ReservationStatus status_ = ReservationStatus::Pending;
};

std::ostream& operator<<(std::ostream& os, const Reservation& r);
