#pragma once

#include <QString>
#include <string>

// 用户抽象基类：保存所有角色共有的信息。
class User {
public:
    User();
    User(int id, std::string name, std::string password);
    virtual ~User() = default;

    int getId() const noexcept;
    const std::string& getName() const noexcept;
    const std::string& getPassword() const noexcept;

    void setId(int newId) noexcept;
    void setName(std::string newName);
    void setPassword(std::string newPassword);

    // 第1周冻结的核心多态接口
    virtual QString getRole() const = 0;

    // 兼容现有预约算法，不改变第1周冻结属性
    bool canAutoApprove() const;
    std::string role_name() const;
    std::string to_string() const;

private:
    int id{0};
    std::string name;
    std::string password;
};
