/**
 * @file    DateTime.cpp
 * @brief   DateTime 值类型实现。
 *
 * 设计要点（面向对象）：
 *  - 值类型（value type）：不可变引用语义，拷贝/赋值廉价且安全；
 *  - 运算符重载：<、<=、>、>=、==、!=、-、operator<< 让区间运算代码像数学表达式一样直观；
 *  - 封装：内部字段私有，对外只暴露 to_minutes()/to_string()，底层表示可自由更换。
 *
 * 时间运算统一换算为"自 1970-01-01 00:00 起的分钟数"（单调刻度），
 * 区间相交、时长计算全部使用整数运算，避免浮点误差。
 */

#include "DateTime.h"

#include <cstdio>
#include <ctime>

namespace {

constexpr long long kMinutesPerDay = 24LL * 60;
constexpr long long kEpochOffset   = 719468LL;   // 0000-03-01 -> 1970-01-01 的天数差

long long days_from_civil(int y, unsigned m, unsigned d) {
    y -= (m <= 2);
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long long>(doe) - kEpochOffset;
}

void civil_from_days(long long z, int& y, unsigned& m, unsigned& d) {
    z += kEpochOffset;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);
}

} // namespace

DateTime::DateTime(int year, int month, int day, int hour, int minute)
    : year_(year), month_(month), day_(day), hour_(hour), minute_(minute) {}

int DateTime::year()  const { return year_; }
int DateTime::month() const { return month_; }
int DateTime::day()   const { return day_; }
int DateTime::hour()  const { return hour_; }
int DateTime::minute() const { return minute_; }

long long DateTime::to_minutes() const {
    return days_from_civil(year_, static_cast<unsigned>(month_), static_cast<unsigned>(day_)) *
               kMinutesPerDay +
           hour_ * 60 + minute_;
}

DateTime DateTime::from_minutes(long long m) {
    int y;
    unsigned mo, d;
    civil_from_days(m / kMinutesPerDay, y, mo, d);
    const long long rem = m % kMinutesPerDay;
    return DateTime(y, static_cast<int>(mo), static_cast<int>(d),
                    static_cast<int>(rem / 60), static_cast<int>(rem % 60));
}

DateTime DateTime::now() {
    const std::time_t t = std::time(nullptr);
    std::tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    return DateTime(tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday, tmv.tm_hour, tmv.tm_min);
}

std::string DateTime::to_string() const {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
                  year_, month_, day_, hour_, minute_);
    return buf;
}

bool DateTime::same_day(const DateTime& other) const {
    return year_ == other.year_ && month_ == other.month_ && day_ == other.day_;
}

long long DateTime::operator-(const DateTime& other) const {
    return to_minutes() - other.to_minutes();
}

bool operator==(const DateTime& a, const DateTime& b) { return a.to_minutes() == b.to_minutes(); }
bool operator!=(const DateTime& a, const DateTime& b) { return a.to_minutes() != b.to_minutes(); }
bool operator< (const DateTime& a, const DateTime& b) { return a.to_minutes() <  b.to_minutes(); }
bool operator<=(const DateTime& a, const DateTime& b) { return a.to_minutes() <= b.to_minutes(); }
bool operator> (const DateTime& a, const DateTime& b) { return a.to_minutes() >  b.to_minutes(); }
bool operator>=(const DateTime& a, const DateTime& b) { return a.to_minutes() >= b.to_minutes(); }

std::ostream& operator<<(std::ostream& os, const DateTime& dt) {
    return os << dt.to_string();
}
