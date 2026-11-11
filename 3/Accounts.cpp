#include "Accounts.h"

namespace learning
{

Profile::Profile(std::string name, std::string bio, std::string timezone)
    : name(std::move(name)), bio(std::move(bio)), timezone(std::move(timezone))
{
}

const std::string &Profile::getName() const
{
    return name;
}

const std::string &Profile::getBio() const
{
    return bio;
}

const std::string &Profile::getTimezone() const
{
    return timezone;
}

void Profile::introduce(std::string text)
{
    nonempty(text);
    bio = std::move(text);
}

std::string Profile::signature() const
{
    nonempty(name);
    return name + " (" + timezone + ")";
}

User::User(std::string email, std::shared_ptr<Profile> profile, bool active)
    : email(std::move(email)), profile(std::move(profile)), active(std::move(active))
{
    if (!this->email.empty())
        changeEmail(this->email);
}

const std::string &User::getEmail() const
{
    return email;
}

const std::shared_ptr<Profile> &User::getProfile() const
{
    return profile;
}

bool User::getActive() const
{
    return active;
}

void User::changeEmail(std::string value)
{
    auto p = value.find('@');
    require(p != std::string::npos && p > 0 && p + 1 < value.size() && value.find('@', p + 1) == std::string::npos &&
                value.find_first_of(" \t\n\r") == std::string::npos,
            "Invalid email");
    email = std::move(value);
}

void User::deactivate()
{
    if (!active)
        throw AccessDenied("Account already inactive");
    active = false;
}

Student::Student(std::shared_ptr<User> user, int credits, std::vector<std::string> interests)
    : user(std::move(user)), credits(std::move(credits)), interests(std::move(interests))
{
    require(credits >= 0, "Negative credits");
}

const std::shared_ptr<User> &Student::getUser() const
{
    return user;
}

int Student::getCredits() const
{
    return credits;
}

const std::vector<std::string> &Student::getInterests() const
{
    return interests;
}

void Student::earnCredits(int count)
{
    require(count > 0 && count <= 1000000 && credits <= 1000000 - count, "Invalid credits");
    credits += count;
}

void Student::followTopic(std::string topic)
{
    nonempty(topic);
    require(std::find(interests.begin(), interests.end(), topic) == interests.end(), "Topic already followed");
    interests.push_back(std::move(topic));
}

Instructor::Instructor(std::shared_ptr<User> user, std::string expertise, int experience)
    : user(std::move(user)), expertise(std::move(expertise)), experience(std::move(experience))
{
    require(experience >= 0, "Negative experience");
}

const std::shared_ptr<User> &Instructor::getUser() const
{
    return user;
}

const std::string &Instructor::getExpertise() const
{
    return expertise;
}

int Instructor::getExperience() const
{
    return experience;
}

void Instructor::qualify(std::string topic, int years)
{
    nonempty(topic);
    require(years >= 0 && years <= 100, "Invalid experience");
    expertise = std::move(topic);
    experience = years;
}

bool Instructor::canTeach(const std::string &topic) const
{
    present(user);
    return user->getActive() && expertise == topic;
}
}
