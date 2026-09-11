#pragma once

#include "User.h"

class Admin final : public User {
public:
    Admin();
    Admin(int id, std::string name, std::string password);

    QString getRole() const override;
};
