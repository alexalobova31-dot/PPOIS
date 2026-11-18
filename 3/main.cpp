#include "Scenario.h"
#include <iostream>
#include <sstream>

namespace
{
int readNumber(const std::string &prompt)
{
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line))
        throw ValidationError("Ввод завершен");
    std::istringstream input(line);
    int value;
    char extra;
    if (!(input >> value) || (input >> extra))
        throw ValidationError("Введите целое число");
    return value;
}

void inheritanceExample()
{
    const MixedLesson lesson("Комбинированный урок", std::make_shared<Video>("Видео", "local:video", 120),
                             std::make_shared<Article>("Статья", "один два три", 3));
    const Lesson &first = static_cast<const VideoLesson &>(lesson);
    const Lesson &second = static_cast<const ArticleLesson &>(lesson);
    std::cout << first << '\n' << "Один базовый объект Lesson: " << (&first == &second ? "да" : "нет") << '\n';
}

void example()
{
    Scenario s;
    auto enrollment = s.platform.enroll(s.student, s.course);
    Progress progress(enrollment);
    progress.complete(s.lesson);
    Certificate certificate(std::make_shared<Progress>(progress));
    certificate.issue("CERT-001");
    std::cout << "Курс: " << s.course->getTitle() << '\n'
              << "Урок: " << *s.lesson << '\n'
              << "Прогресс: " << progress.percent() << "%\n"
              << "Сертификат: " << certificate.getSerial() << '\n';
    inheritanceExample();
}

void menu()
{
    Scenario s;
    std::shared_ptr<Enrollment> enrollment;
    std::shared_ptr<Progress> progress;
    std::shared_ptr<Certificate> certificate;
    for (;;)
    {
        std::cout << "\n1. Каталог курсов\n2. Записаться на курс\n3. Завершить урок\n"
                     "4. Прогресс\n5. Получить сертификат\n6. Пример наследования\n0. Выход\n";
        try
        {
            int choice = readNumber("Выберите пункт: ");
            if (choice == 0)
                return;
            switch (choice)
            {
            case 1:
                std::cout << s.course->getTitle() << ": " << s.course->lessonCount() << " урок(ов)\n";
                break;
            case 2:
                enrollment = s.platform.enroll(s.student, s.course);
                progress = std::make_shared<Progress>(enrollment);
                certificate = std::make_shared<Certificate>(progress);
                std::cout << "Запись оформлена.\n";
                break;
            case 3:
                if (!progress)
                    throw NotEnrolled("Сначала запишитесь на курс");
                std::cout << *s.lesson << '\n';
                progress->complete(s.lesson);
                std::cout << "Урок завершен.\n";
                break;
            case 4:
                if (!progress)
                    throw NotEnrolled("Сначала запишитесь на курс");
                std::cout << "Прогресс: " << progress->percent() << "%\n";
                break;
            case 5:
                if (!certificate)
                    throw NotEnrolled("Сначала запишитесь на курс");
                certificate->issue("CERT-001");
                std::cout << "Сертификат: " << certificate->getSerial() << '\n';
                break;
            case 6:
                inheritanceExample();
                break;
            default:
                std::cout << "Нет такого пункта.\n";
            }
        }
        catch (const std::exception &error)
        {
            if (std::cin.eof())
                return;
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}
}

int main(int argc, char **argv)
{
    try
    {
        if (argc == 1)
            menu();
        else if (argc == 2 && std::string(argv[1]) == "--example")
            example();
        else
        {
            std::cerr << "Использование: learning [--example]\n";
            return 2;
        }
    }
    catch (const std::exception &error)
    {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
