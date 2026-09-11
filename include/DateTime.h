#pragma once
#include <string>
#include <ostream>

// 时间值类型：年/月/日/时/分。内部转为"自1970-01-01起的分钟数"做区间运算。
// 说明：本头文件按 INTERFACE.md 契约补齐（算法组 .cpp 已依赖此声明）。
class DateTime {
public:
    DateTime(int year = 1970, int month = 1, int day = 1, int hour = 0, int minute = 0);

    int year()  const;
    int month() const;
    int day()   const;
    int hour()  const;
    int minute() const;

    long long to_minutes() const;              // 自1970-01-01 00:00起的分钟数
    static DateTime from_minutes(long long m); // 逆变换
    static DateTime now();                     // 当前系统时间

    std::string to_string() const;             // "YYYY-MM-DD HH:MM"
    bool same_day(const DateTime& other) const;

    long long operator-(const DateTime& other) const;   // 分钟差：*this - other

    friend bool operator==(const DateTime& a, const DateTime& b);
    friend bool operator!=(const DateTime& a, const DateTime& b);
    friend bool operator< (const DateTime& a, const DateTime& b);
    friend bool operator<=(const DateTime& a, const DateTime& b);
    friend bool operator> (const DateTime& a, const DateTime& b);
    friend bool operator>=(const DateTime& a, const DateTime& b);

private:
    int year_, month_, day_, hour_, minute_;
};

std::ostream& operator<<(std::ostream& os, const DateTime& dt);
