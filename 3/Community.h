#ifndef COMMUNITY_H
#define COMMUNITY_H
#include "Learning.h"

namespace learning
{

class Post
{
  private:
    std::shared_ptr<User> author;
    std::string text;
    bool hidden;

  public:
    Post(std::shared_ptr<User> author = {}, std::string text = {}, bool hidden = false);
    const std::shared_ptr<User> &getAuthor() const;
    const std::string &getText() const;
    bool getHidden() const;
    void edit(const std::shared_ptr<User> &actor, std::string body);
    bool visible() const;
};

class Thread
{
  private:
    std::string title;
    std::vector<std::shared_ptr<Post>> posts;
    bool locked;

  public:
    Thread(std::string title = {}, std::vector<std::shared_ptr<Post>> posts = {}, bool locked = false);
    const std::string &getTitle() const;
    const std::vector<std::shared_ptr<Post>> &getPosts() const;
    bool getLocked() const;
    void reply(std::shared_ptr<Post> post);
    int visibleCount() const;
};

class Discussion
{
  private:
    std::shared_ptr<Course> course;
    std::vector<std::shared_ptr<Thread>> threads;
    bool enabled;

  public:
    Discussion(std::shared_ptr<Course> course = {}, std::vector<std::shared_ptr<Thread>> threads = {},
               bool enabled = true);
    const std::shared_ptr<Course> &getCourse() const;
    const std::vector<std::shared_ptr<Thread>> &getThreads() const;
    bool getEnabled() const;
    void open(std::shared_ptr<Thread> thread);
    std::shared_ptr<Thread> find(const std::string &title) const;
};

class Notification
{
  private:
    std::shared_ptr<User> recipient;
    std::string text;
    bool read;

  public:
    Notification(std::shared_ptr<User> recipient = {}, std::string text = {}, bool read = false);
    const std::shared_ptr<User> &getRecipient() const;
    const std::string &getText() const;
    bool getRead() const;
    void acknowledge(const std::shared_ptr<User> &actor);
    bool pending() const;
};

class Inbox
{
  private:
    std::shared_ptr<User> owner;
    std::vector<std::shared_ptr<Notification>> notifications;
    int capacity;

  public:
    Inbox(std::shared_ptr<User> owner = {}, std::vector<std::shared_ptr<Notification>> notifications = {},
          int capacity = 100);
    const std::shared_ptr<User> &getOwner() const;
    const std::vector<std::shared_ptr<Notification>> &getNotifications() const;
    int getCapacity() const;
    void deliver(std::shared_ptr<Notification> notification);
    int unread() const;
};

class Message
{
  private:
    std::shared_ptr<User> sender;
    std::shared_ptr<User> recipient;
    std::string text;

  public:
    Message(std::shared_ptr<User> sender = {}, std::shared_ptr<User> recipient = {}, std::string text = {});
    const std::shared_ptr<User> &getSender() const;
    const std::shared_ptr<User> &getRecipient() const;
    const std::string &getText() const;
    void compose(std::string body);
    bool involves(const std::shared_ptr<User> &user) const;
};

class Conversation
{
  private:
    std::shared_ptr<User> first;
    std::shared_ptr<User> second;
    std::vector<std::shared_ptr<Message>> messages;

  public:
    Conversation(std::shared_ptr<User> first = {}, std::shared_ptr<User> second = {},
                 std::vector<std::shared_ptr<Message>> messages = {});
    const std::shared_ptr<User> &getFirst() const;
    const std::shared_ptr<User> &getSecond() const;
    const std::vector<std::shared_ptr<Message>> &getMessages() const;
    void send(std::shared_ptr<Message> message);
    int receivedBy(const std::shared_ptr<User> &user) const;
};

class Event
{
  private:
    std::string title;
    int start;
    int end;

  public:
    Event(std::string title = {}, int start = 0, int end = 1);
    const std::string &getTitle() const;
    int getStart() const;
    int getEnd() const;
    bool overlaps(const Event &other) const;
    void reschedule(int begin, int finish);
};

class Calendar
{
  private:
    std::shared_ptr<User> owner;
    std::vector<std::shared_ptr<Event>> events;
    int capacity;

  public:
    Calendar(std::shared_ptr<User> owner = {}, std::vector<std::shared_ptr<Event>> events = {}, int capacity = 100);
    const std::shared_ptr<User> &getOwner() const;
    const std::vector<std::shared_ptr<Event>> &getEvents() const;
    int getCapacity() const;
    void schedule(std::shared_ptr<Event> event);
    int occupied() const;
};

class StudyGroup
{
  private:
    std::string name;
    std::vector<std::shared_ptr<Student>> members;
    int capacity;

  public:
    StudyGroup(std::string name = {}, std::vector<std::shared_ptr<Student>> members = {}, int capacity = 10);
    const std::string &getName() const;
    const std::vector<std::shared_ptr<Student>> &getMembers() const;
    int getCapacity() const;
    void join(std::shared_ptr<Student> student);
    void leave(const std::shared_ptr<Student> &student);
};

class GroupMeeting
{
  private:
    std::shared_ptr<StudyGroup> group;
    std::shared_ptr<Event> event;
    std::vector<std::shared_ptr<Student>> attendees;

  public:
    GroupMeeting(std::shared_ptr<StudyGroup> group = {}, std::shared_ptr<Event> event = {},
                 std::vector<std::shared_ptr<Student>> attendees = {});
    const std::shared_ptr<StudyGroup> &getGroup() const;
    const std::shared_ptr<Event> &getEvent() const;
    const std::vector<std::shared_ptr<Student>> &getAttendees() const;
    void attend(std::shared_ptr<Student> student);
    double participation() const;
};
}
#endif
