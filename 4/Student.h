#pragma once

#include <ostream>
#include <string>
#include <utility>

class Student {
    std::string name_;
    int grade_;

public:
    Student(std::string name, int grade) : name_(std::move(name)), grade_(grade) {}
    const std::string& name() const { return name_; }
    int grade() const { return grade_; }
    bool operator<(const Student& other) const { return grade_ < other.grade_; }
    bool operator==(const Student& other) const
    {
        return name_ == other.name_ && grade_ == other.grade_;
    }
    friend std::ostream& operator<<(std::ostream& out, const Student& student)
    {
        return out << student.name_ << " (" << student.grade_ << ')';
    }
};
