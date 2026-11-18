#ifndef ACCOUNTS_H
#define ACCOUNTS_H
#include "Common.h"

namespace learning
{

class Profile
{
  private:
    std::string name;
    std::string bio;
    std::string timezone;

  public:
    Profile(std::string name = {}, std::string bio = {}, std::string timezone = {});
    const std::string &getName() const;
    const std::string &getBio() const;
    const std::string &getTimezone() const;
    void introduce(std::string text);
    std::string signature() const;
};

class User
{
  private:
    std::string email;
    std::shared_ptr<Profile> profile;
    bool active;

  public:
    User(std::string email = {}, std::shared_ptr<Profile> profile = {}, bool active = true);
    const std::string &getEmail() const;
    const std::shared_ptr<Profile> &getProfile() const;
    bool getActive() const;
    void changeEmail(std::string value);
    void deactivate();
};

class Student
{
  private:
    std::shared_ptr<User> user;
    int credits;
    std::vector<std::string> interests;

  public:
    Student(std::shared_ptr<User> user = {}, int credits = 0, std::vector<std::string> interests = {});
    const std::shared_ptr<User> &getUser() const;
    int getCredits() const;
    const std::vector<std::string> &getInterests() const;
    void earnCredits(int count);
    void followTopic(std::string topic);
};

class Instructor
{
  private:
    std::shared_ptr<User> user;
    std::string expertise;
    int experience;

  public:
    Instructor(std::shared_ptr<User> user = {}, std::string expertise = {}, int experience = 0);
    const std::shared_ptr<User> &getUser() const;
    const std::string &getExpertise() const;
    int getExperience() const;
    void qualify(std::string topic, int years);
    bool canTeach(const std::string &topic) const;
};
}
#endif
