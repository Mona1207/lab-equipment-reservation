#include "Student.h"

#include <utility>

Student::Student() = default;

Student::Student(int id, std::string name, std::string password)
    : User(id, std::move(name), std::move(password))
{
}

QString Student::getRole() const
{
    return QStringLiteral("Student");
}
