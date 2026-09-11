#include "Teacher.h"

#include <utility>

Teacher::Teacher() = default;

Teacher::Teacher(int id, std::string name, std::string password)
    : User(id, std::move(name), std::move(password))
{
}

QString Teacher::getRole() const
{
    return QStringLiteral("Teacher");
}
