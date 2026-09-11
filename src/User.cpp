#include "User.h"

#include <utility>

User::User() = default;

User::User(int id, std::string name, std::string password)
    : id(id),
      name(std::move(name)),
      password(std::move(password))
{
}

int User::getId() const noexcept
{
    return id;
}

const std::string& User::getName() const noexcept
{
    return name;
}

const std::string& User::getPassword() const noexcept
{
    return password;
}

void User::setId(int newId) noexcept
{
    id = newId;
}

void User::setName(std::string newName)
{
    name = std::move(newName);
}

void User::setPassword(std::string newPassword)
{
    password = std::move(newPassword);
}

bool User::canAutoApprove() const
{
    const QString role = getRole();
    return role == QStringLiteral("Teacher")
        || role == QStringLiteral("Admin");
}

std::string User::role_name() const
{
    const QString role = getRole();

    if (role == QStringLiteral("Student")) {
        return "学生";
    }

    if (role == QStringLiteral("Teacher")) {
        return "教师";
    }

    if (role == QStringLiteral("Admin")) {
        return "管理员";
    }

    return role.toStdString();
}

std::string User::to_string() const
{
    return std::to_string(id) + " " + name + "(" + role_name() + ")";
}
