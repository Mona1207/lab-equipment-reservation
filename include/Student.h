#pragma once

#include "User.h"

class Student final : public User {
public:
    Student();
    Student(int id, std::string name, std::string password);

    QString getRole() const override;
};
