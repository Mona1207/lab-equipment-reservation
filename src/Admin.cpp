#include "Admin.h"

#include <utility>

Admin::Admin() = default;

Admin::Admin(int id, std::string name, std::string password)
    : User(id, std::move(name), std::move(password))
{
}

QString Admin::getRole() const
{
    return QStringLiteral("Admin");
}
