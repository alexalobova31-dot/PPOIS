#include "Community.h"

namespace learning
{

Post::Post(std::shared_ptr<User> author, std::string text, bool hidden)
    : author(std::move(author)), text(std::move(text)), hidden(std::move(hidden))
{
}

const std::shared_ptr<User> &Post::getAuthor() const
{
    return author;
}

const std::string &Post::getText() const
{
    return text;
}

bool Post::getHidden() const
{
    return hidden;
}

void Post::edit(const std::shared_ptr<User> &actor, std::string body)
{
    present(author);
    if (!actor || actor != author || !actor->getActive() || hidden)
        throw AccessDenied("Cannot edit post");
    nonempty(body);
    text = std::move(body);
}

bool Post::visible() const
{
    present(author);
    return !hidden && author->getActive() && !text.empty();
}

Thread::Thread(std::string title, std::vector<std::shared_ptr<Post>> posts, bool locked)
    : title(std::move(title)), posts(std::move(posts)), locked(std::move(locked))
{
    for (const auto &item : this->posts)
        present(item);
}

const std::string &Thread::getTitle() const
{
    return title;
}

const std::vector<std::shared_ptr<Post>> &Thread::getPosts() const
{
    return posts;
}

bool Thread::getLocked() const
{
    return locked;
}

void Thread::reply(std::shared_ptr<Post> post)
{
    present(post);
    if (locked)
        throw AccessDenied("Thread locked");
    require(post->visible(), "Invisible post");
    require(std::find(posts.begin(), posts.end(), post) == posts.end(), "Duplicate post");
    posts.push_back(std::move(post));
}

int Thread::visibleCount() const
{
    return static_cast<int>(
        std::count_if(posts.begin(), posts.end(), [](const auto &post) { return post->visible(); }));
}

Discussion::Discussion(std::shared_ptr<Course> course, std::vector<std::shared_ptr<Thread>> threads, bool enabled)
    : course(std::move(course)), threads(std::move(threads)), enabled(std::move(enabled))
{
    for (const auto &item : this->threads)
        present(item);
}

const std::shared_ptr<Course> &Discussion::getCourse() const
{
    return course;
}

const std::vector<std::shared_ptr<Thread>> &Discussion::getThreads() const
{
    return threads;
}

bool Discussion::getEnabled() const
{
    return enabled;
}

void Discussion::open(std::shared_ptr<Thread> thread)
{
    present(thread);
    if (!enabled)
        throw AccessDenied("Discussion disabled");
    nonempty(thread->getTitle());
    require(std::find(threads.begin(), threads.end(), thread) == threads.end(), "Duplicate thread");
    threads.push_back(std::move(thread));
}

std::shared_ptr<Thread> Discussion::find(const std::string &title) const
{
    for (const auto &thread : threads)
        if (thread->getTitle() == title)
            return thread;
    throw ResourceNotFound("Thread not found");
}

Notification::Notification(std::shared_ptr<User> recipient, std::string text, bool read)
    : recipient(std::move(recipient)), text(std::move(text)), read(std::move(read))
{
}

const std::shared_ptr<User> &Notification::getRecipient() const
{
    return recipient;
}

const std::string &Notification::getText() const
{
    return text;
}

bool Notification::getRead() const
{
    return read;
}

void Notification::acknowledge(const std::shared_ptr<User> &actor)
{
    present(recipient);
    if (!actor || actor != recipient || !actor->getActive())
        throw AccessDenied("Cannot read notification");
    read = true;
}

bool Notification::pending() const
{
    present(recipient);
    return recipient->getActive() && !read;
}

Inbox::Inbox(std::shared_ptr<User> owner, std::vector<std::shared_ptr<Notification>> notifications, int capacity)
    : owner(std::move(owner)), notifications(std::move(notifications)), capacity(std::move(capacity))
{
    for (const auto &item : this->notifications)
        present(item);
    require(capacity >= 0, "Negative capacity");
}

const std::shared_ptr<User> &Inbox::getOwner() const
{
    return owner;
}

const std::vector<std::shared_ptr<Notification>> &Inbox::getNotifications() const
{
    return notifications;
}

int Inbox::getCapacity() const
{
    return capacity;
}

void Inbox::deliver(std::shared_ptr<Notification> notification)
{
    present(owner);
    present(notification);
    require(notification->getRecipient() == owner && owner->getActive(), "Invalid recipient");
    require(capacity > 0 && notifications.size() < static_cast<std::size_t>(capacity), "Inbox full");
    require(std::find(notifications.begin(), notifications.end(), notification) == notifications.end(),
            "Duplicate notification");
    nonempty(notification->getText());
    notifications.push_back(std::move(notification));
}

int Inbox::unread() const
{
    return static_cast<int>(
        std::count_if(notifications.begin(), notifications.end(), [](const auto &n) { return n->pending(); }));
}

Message::Message(std::shared_ptr<User> sender, std::shared_ptr<User> recipient, std::string text)
    : sender(std::move(sender)), recipient(std::move(recipient)), text(std::move(text))
{
}

const std::shared_ptr<User> &Message::getSender() const
{
    return sender;
}

const std::shared_ptr<User> &Message::getRecipient() const
{
    return recipient;
}

const std::string &Message::getText() const
{
    return text;
}

void Message::compose(std::string body)
{
    present(sender);
    present(recipient);
    if (!sender->getActive() || !recipient->getActive() || sender == recipient)
        throw AccessDenied("Cannot send message");
    nonempty(body);
    text = std::move(body);
}

bool Message::involves(const std::shared_ptr<User> &user) const
{
    present(user);
    return user == sender || user == recipient;
}

Conversation::Conversation(std::shared_ptr<User> first, std::shared_ptr<User> second,
                           std::vector<std::shared_ptr<Message>> messages)
    : first(std::move(first)), second(std::move(second)), messages(std::move(messages))
{
    for (const auto &item : this->messages)
        present(item);
}

const std::shared_ptr<User> &Conversation::getFirst() const
{
    return first;
}

const std::shared_ptr<User> &Conversation::getSecond() const
{
    return second;
}

const std::vector<std::shared_ptr<Message>> &Conversation::getMessages() const
{
    return messages;
}

void Conversation::send(std::shared_ptr<Message> message)
{
    present(first);
    present(second);
    present(message);
    require(first != second && ((message->getSender() == first && message->getRecipient() == second) ||
                                (message->getSender() == second && message->getRecipient() == first)),
            "Wrong conversation");
    message->compose(message->getText());
    require(std::find(messages.begin(), messages.end(), message) == messages.end(), "Duplicate message");
    messages.push_back(std::move(message));
}

int Conversation::receivedBy(const std::shared_ptr<User> &user) const
{
    if (!user || (user != first && user != second))
        throw AccessDenied("Not a participant");
    return static_cast<int>(
        std::count_if(messages.begin(), messages.end(), [&](const auto &m) { return m->getRecipient() == user; }));
}

Event::Event(std::string title, int start, int end)
    : title(std::move(title)), start(std::move(start)), end(std::move(end))
{
    require(start >= 0, "Negative start");
    require(end >= 0, "Negative end");
}

const std::string &Event::getTitle() const
{
    return title;
}

int Event::getStart() const
{
    return start;
}

int Event::getEnd() const
{
    return end;
}

bool Event::overlaps(const Event &other) const
{
    require(start >= 0 && end > start && other.getStart() >= 0 && other.getEnd() > other.getStart(),
            "Invalid event interval");
    return start < other.getEnd() && other.getStart() < end;
}

void Event::reschedule(int begin, int finish)
{
    require(begin >= 0 && finish > begin, "Invalid event interval");
    start = begin;
    end = finish;
}

Calendar::Calendar(std::shared_ptr<User> owner, std::vector<std::shared_ptr<Event>> events, int capacity)
    : owner(std::move(owner)), events(std::move(events)), capacity(std::move(capacity))
{
    for (const auto &item : this->events)
        present(item);
    require(capacity >= 0, "Negative capacity");
}

const std::shared_ptr<User> &Calendar::getOwner() const
{
    return owner;
}

const std::vector<std::shared_ptr<Event>> &Calendar::getEvents() const
{
    return events;
}

int Calendar::getCapacity() const
{
    return capacity;
}

void Calendar::schedule(std::shared_ptr<Event> event)
{
    present(event);
    nonempty(event->getTitle());
    event->overlaps(*event);
    require(capacity > 0 && events.size() < static_cast<std::size_t>(capacity), "Calendar full");
    for (const auto &existing : events)
        require(!existing->overlaps(*event), "Schedule conflict");
    events.push_back(std::move(event));
}

int Calendar::occupied() const
{
    int sum = 0;
    for (const auto &event : events)
    {
        present(event);
        require(event->getStart() >= 0 && event->getEnd() > event->getStart() &&
                    event->getEnd() - event->getStart() <= 1000000 - sum,
                "Invalid calendar duration");
        sum += event->getEnd() - event->getStart();
    }
    return sum;
}

StudyGroup::StudyGroup(std::string name, std::vector<std::shared_ptr<Student>> members, int capacity)
    : name(std::move(name)), members(std::move(members)), capacity(std::move(capacity))
{
    for (const auto &item : this->members)
        present(item);
    require(capacity >= 0, "Negative capacity");
}

const std::string &StudyGroup::getName() const
{
    return name;
}

const std::vector<std::shared_ptr<Student>> &StudyGroup::getMembers() const
{
    return members;
}

int StudyGroup::getCapacity() const
{
    return capacity;
}

void StudyGroup::join(std::shared_ptr<Student> student)
{
    present(student);
    present(student->getUser());
    require(student->getUser()->getActive() && capacity > 0 && members.size() < static_cast<std::size_t>(capacity),
            "Group unavailable");
    require(std::find(members.begin(), members.end(), student) == members.end(), "Already a member");
    members.push_back(std::move(student));
}

void StudyGroup::leave(const std::shared_ptr<Student> &student)
{
    auto it = std::find(members.begin(), members.end(), student);
    if (it == members.end())
        throw ResourceNotFound("Member not found");
    members.erase(it);
}

GroupMeeting::GroupMeeting(std::shared_ptr<StudyGroup> group, std::shared_ptr<Event> event,
                           std::vector<std::shared_ptr<Student>> attendees)
    : group(std::move(group)), event(std::move(event)), attendees(std::move(attendees))
{
    for (const auto &item : this->attendees)
        present(item);
}

const std::shared_ptr<StudyGroup> &GroupMeeting::getGroup() const
{
    return group;
}

const std::shared_ptr<Event> &GroupMeeting::getEvent() const
{
    return event;
}

const std::vector<std::shared_ptr<Student>> &GroupMeeting::getAttendees() const
{
    return attendees;
}

void GroupMeeting::attend(std::shared_ptr<Student> student)
{
    present(group);
    present(student);
    if (std::find(group->getMembers().begin(), group->getMembers().end(), student) == group->getMembers().end())
        throw AccessDenied("Not a group member");
    require(std::find(attendees.begin(), attendees.end(), student) == attendees.end(), "Already attending");
    attendees.push_back(std::move(student));
}

double GroupMeeting::participation() const
{
    present(group);
    if (group->getMembers().empty())
        return 0;
    int count = 0;
    for (const auto &member : group->getMembers())
        if (std::find(attendees.begin(), attendees.end(), member) != attendees.end())
            ++count;
    return 100.0 * count / group->getMembers().size();
}
}
