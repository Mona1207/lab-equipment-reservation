#pragma once

#include "User.h"

class Teacher final : public User {
public:
    Teacher();
    Teacher(int id, std::string name, std::string password);

    QString getRole() const override;
};
