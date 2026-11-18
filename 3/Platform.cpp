#include "Platform.h"

namespace learning
{

Platform::Platform(std::string name, std::shared_ptr<Catalog> catalog,
                   std::vector<std::shared_ptr<Enrollment>> enrollments)
    : name(std::move(name)), catalog(std::move(catalog)), enrollments(std::move(enrollments))
{
    for (const auto &item : this->enrollments)
        present(item);
}

const std::string &Platform::getName() const
{
    return name;
}

const std::shared_ptr<Catalog> &Platform::getCatalog() const
{
    return catalog;
}

const std::vector<std::shared_ptr<Enrollment>> &Platform::getEnrollments() const
{
    return enrollments;
}

std::shared_ptr<Enrollment> Platform::enroll(std::shared_ptr<Student> student, std::shared_ptr<Course> course)
{
    present(catalog);
    require(std::find(catalog->getCourses().begin(), catalog->getCourses().end(), course) !=
                catalog->getCourses().end(),
            "Course outside catalog");
    for (const auto &entry : enrollments)
        if (entry->getStudent() == student && entry->getCourse() == course && entry->getActive())
            throw AlreadyEnrolled("Already enrolled in course");
    auto entry = std::make_shared<Enrollment>(Enrollment{std::move(student), std::move(course), false});
    entry->enroll();
    enrollments.push_back(entry);
    return entry;
}

int Platform::activeStudents() const
{
    std::vector<std::shared_ptr<Student>> students;
    for (const auto &entry : enrollments)
        if (entry->getActive() && std::find(students.begin(), students.end(), entry->getStudent()) == students.end())
            students.push_back(entry->getStudent());
    return static_cast<int>(students.size());
}
}
