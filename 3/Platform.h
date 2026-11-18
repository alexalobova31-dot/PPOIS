#ifndef PLATFORM_H
#define PLATFORM_H
#include "Learning.h"

namespace learning
{

class Platform
{
  private:
    std::string name;
    std::shared_ptr<Catalog> catalog;
    std::vector<std::shared_ptr<Enrollment>> enrollments;

  public:
    Platform(std::string name = {}, std::shared_ptr<Catalog> catalog = {},
             std::vector<std::shared_ptr<Enrollment>> enrollments = {});
    const std::string &getName() const;
    const std::shared_ptr<Catalog> &getCatalog() const;
    const std::vector<std::shared_ptr<Enrollment>> &getEnrollments() const;
    std::shared_ptr<Enrollment> enroll(std::shared_ptr<Student> student, std::shared_ptr<Course> course);
    int activeStudents() const;
};
}
#endif
