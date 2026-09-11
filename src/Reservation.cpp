/**
 * @file    Reservation.cpp
 * @brief   Reservation 预约实体实现。
 *
 * 设计要点（面向对象）：
 *  - 预约是"值语义"实体：Manager 中以值存放、拷贝/移动传递，生命周期清晰；
 *  - overlaps() 把"区间相交判定"这一核心算法封装为对象自身的方法。
 *    约定半开区间 [start, end)：一次预约占用的时间片包含起点、不包含终点，
 *    这样背靠背的预约（12:00~13:00 与 13:00~14:00）不冲突 —— 数学上更严谨；
 *  - 状态转移只能通过 set_status()，状态值域由强类型枚举约定。
 */

#include "Reservation.h"

Reservation::Reservation(const std::string& id, const std::string& equipment_id,
                         int user_id, const DateTime& start_time,
                         const DateTime& end_time)
    : id_(id),
      equipment_id_(equipment_id),
      user_id_(user_id),
      start_time_(start_time),
      end_time_(end_time),
      status_(ReservationStatus::Pending) {}

const std::string& Reservation::id() const { return id_; }
const std::string& Reservation::equipment_id() const { return equipment_id_; }
int Reservation::user_id() const { return user_id_; }
const DateTime& Reservation::start_time() const { return start_time_; }
const DateTime& Reservation::end_time() const { return end_time_; }

ReservationStatus Reservation::status() const { return status_; }
void Reservation::set_status(ReservationStatus status) { status_ = status; }

bool Reservation::overlaps(const Reservation& other) const {
    return start_time_.to_minutes() < other.end_time_.to_minutes() &&
           other.start_time_.to_minutes() < end_time_.to_minutes();
}

long long Reservation::duration_minutes() const {
    return end_time_ - start_time_;
}

std::string Reservation::status_name() const {
    switch (status_) {
        case ReservationStatus::Pending:  return "待审批";
        case ReservationStatus::Approved: return "已通过";
        case ReservationStatus::Rejected: return "已拒绝";
        case ReservationStatus::Returned: return "已归还";
    case ReservationStatus::Cancelled: return "已取消";
    }
    return "未知";
}

std::string Reservation::to_string() const {
    return id_ + " 设备" + equipment_id_ + " 用户" + std::to_string(user_id_) + " " +
           start_time_.to_string() + "~" + end_time_.to_string() + " [" + status_name() + "]";
}

std::ostream& operator<<(std::ostream& os, const Reservation& r) {
    return os << r.to_string();
}
