#ifndef SCENARIO_H
#define SCENARIO_H
#include "Platform.h"
#include "Community.h"
#include "Finance.h"

using namespace learning;

struct Scenario
{
    std::shared_ptr<User> account;
    std::shared_ptr<Student> student;
    std::shared_ptr<Lesson> lesson;
    std::shared_ptr<Course> course;
    Platform platform;

    Scenario()
    {
        account = std::make_shared<User>("student@example.org");
        student = std::make_shared<Student>(account);
        auto teacher = std::make_shared<Instructor>(std::make_shared<User>("teacher@example.org"), "C++", 3);
        lesson = std::make_shared<Lesson>("Classes", std::make_shared<Video>("Classes", "local:classes", 120));
        auto module = std::make_shared<Module>("OOP");
        module->addLesson(lesson);
        module->publish();
        course = std::make_shared<Course>("C++ basics", teacher, std::vector<std::shared_ptr<Module>>{module}, 10000);
        course->publish();
        auto catalog = std::make_shared<Catalog>();
        catalog->include(course);
        platform = Platform("Online learning", catalog);
    }
};
#endif
