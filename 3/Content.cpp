#include "Content.h"

namespace learning
{

Tag::Tag(std::string name, std::string description, bool visible)
    : name(std::move(name)), description(std::move(description)), visible(std::move(visible))
{
}

const std::string &Tag::getName() const
{
    return name;
}

const std::string &Tag::getDescription() const
{
    return description;
}

bool Tag::getVisible() const
{
    return visible;
}

bool Tag::matches(const std::string &query) const
{
    nonempty(query);
    return visible && name.find(query) != std::string::npos;
}

void Tag::hide()
{
    require(visible, "Tag already hidden");
    visible = false;
}

Category::Category(std::string name, std::vector<std::shared_ptr<Tag>> tags, bool enabled)
    : name(std::move(name)), tags(std::move(tags)), enabled(std::move(enabled))
{
    for (const auto &item : this->tags)
        present(item);
}

const std::string &Category::getName() const
{
    return name;
}

const std::vector<std::shared_ptr<Tag>> &Category::getTags() const
{
    return tags;
}

bool Category::getEnabled() const
{
    return enabled;
}

void Category::addTag(std::shared_ptr<Tag> tag)
{
    present(tag);
    require(std::none_of(tags.begin(), tags.end(), [&](const auto &t) { return t->getName() == tag->getName(); }),
            "Duplicate tag");
    tags.push_back(std::move(tag));
}

bool Category::covers(const std::string &query) const
{
    return enabled && std::any_of(tags.begin(), tags.end(), [&](const auto &t) { return t->matches(query); });
}

Video::Video(std::string title, std::string url, int seconds)
    : title(std::move(title)), url(std::move(url)), seconds(std::move(seconds))
{
    require(seconds >= 0, "Negative seconds");
}

const std::string &Video::getTitle() const
{
    return title;
}

const std::string &Video::getUrl() const
{
    return url;
}

int Video::getSeconds() const
{
    return seconds;
}

int Video::remaining(int watched) const
{
    require(seconds > 0 && watched >= 0 && watched <= seconds, "Invalid position");
    return seconds - watched;
}

void Video::trim(int start, int end)
{
    require(start >= 0 && end > start && end <= seconds, "Invalid video range");
    seconds = end - start;
}

Article::Article(std::string title, std::string body, int wordsPerMinute)
    : title(std::move(title)), body(std::move(body)), wordsPerMinute(std::move(wordsPerMinute))
{
    require(wordsPerMinute >= 0, "Negative wordsPerMinute");
}

const std::string &Article::getTitle() const
{
    return title;
}

const std::string &Article::getBody() const
{
    return body;
}

int Article::getWordsPerMinute() const
{
    return wordsPerMinute;
}

int Article::readingMinutes() const
{
    require(wordsPerMinute > 0, "Invalid reading speed");
    int words = 0;
    bool inside = false;
    for (char c : body)
    {
        bool space = c == ' ' || c == '\n' || c == '\t' || c == '\r';
        if (!space && !inside)
            ++words;
        inside = !space;
    }
    return words / wordsPerMinute + (words % wordsPerMinute != 0);
}

void Article::append(std::string text)
{
    nonempty(text);
    if (!body.empty())
        body += '\n';
    body += text;
}

Attachment::Attachment(std::string filename, std::string mediaType, int bytes)
    : filename(std::move(filename)), mediaType(std::move(mediaType)), bytes(std::move(bytes))
{
    require(bytes >= 0, "Negative bytes");
}

const std::string &Attachment::getFilename() const
{
    return filename;
}

const std::string &Attachment::getMediaType() const
{
    return mediaType;
}

int Attachment::getBytes() const
{
    return bytes;
}

bool Attachment::fits(int limit) const
{
    require(bytes >= 0 && limit >= 0, "Invalid size");
    return bytes <= limit;
}

void Attachment::rename(std::string name)
{
    nonempty(name);
    require(name != "." && name != ".." && name.find_first_of("/\\") == std::string::npos, "Invalid filename");
    filename = std::move(name);
}

Lesson::Lesson(std::string title, std::shared_ptr<Video> video, std::shared_ptr<Article> article)
    : title(std::move(title)), video(std::move(video)), article(std::move(article))
{
}

const std::string &Lesson::getTitle() const
{
    return title;
}

const std::shared_ptr<Video> &Lesson::getVideo() const
{
    return video;
}

const std::shared_ptr<Article> &Lesson::getArticle() const
{
    return article;
}

int Lesson::duration() const
{
    if (!video && !article)
        throw LessonLocked("Lesson has no content");
    return (video ? video->remaining(0) : 0) + (article ? article->readingMinutes() * 60 : 0);
}

bool Lesson::contains(const std::string &query) const
{
    nonempty(query);
    return title.find(query) != std::string::npos || (article && article->getBody().find(query) != std::string::npos);
}

Module::Module(std::string title, std::vector<std::shared_ptr<Lesson>> lessons, bool published)
    : title(std::move(title)), lessons(std::move(lessons)), published(std::move(published))
{
    for (const auto &item : this->lessons)
        present(item);
}

const std::string &Module::getTitle() const
{
    return title;
}

const std::vector<std::shared_ptr<Lesson>> &Module::getLessons() const
{
    return lessons;
}

bool Module::getPublished() const
{
    return published;
}

void Module::addLesson(std::shared_ptr<Lesson> lesson)
{
    present(lesson);
    require(!published, "Published module is immutable");
    require(std::find(lessons.begin(), lessons.end(), lesson) == lessons.end(), "Duplicate lesson");
    lessons.push_back(std::move(lesson));
}

void Module::publish()
{
    require(!lessons.empty(), "Empty module");
    for (const auto &lesson : lessons)
    {
        present(lesson);
        lesson->duration();
    }
    published = true;
}

Course::Course(std::string title, std::shared_ptr<Instructor> instructor, std::vector<std::shared_ptr<Module>> modules,
               Money price, bool published)
    : title(std::move(title)), instructor(std::move(instructor)), modules(std::move(modules)), price(std::move(price)),
      published(std::move(published))
{
    for (const auto &item : this->modules)
        present(item);
    money(price);
}

const std::string &Course::getTitle() const
{
    return title;
}

const std::shared_ptr<Instructor> &Course::getInstructor() const
{
    return instructor;
}

const std::vector<std::shared_ptr<Module>> &Course::getModules() const
{
    return modules;
}

Money Course::getPrice() const
{
    return price;
}

bool Course::getPublished() const
{
    return published;
}

void Course::publish()
{
    present(instructor);
    present(instructor->getUser());
    money(price);
    nonempty(title);
    require(instructor->getUser()->getActive() && !modules.empty(), "Course is incomplete");
    std::vector<std::shared_ptr<Lesson>> seen;
    for (const auto &module : modules)
    {
        present(module);
        require(module->getPublished() && !module->getLessons().empty(), "Unpublished or empty module");
        for (const auto &lesson : module->getLessons())
        {
            present(lesson);
            lesson->duration();
            require(std::find(seen.begin(), seen.end(), lesson) == seen.end(), "Duplicate lesson in course");
            seen.push_back(lesson);
        }
    }
    published = true;
}

int Course::lessonCount() const
{
    int count = 0;
    for (const auto &module : modules)
    {
        present(module);
        count += static_cast<int>(module->getLessons().size());
    }
    return count;
}

Catalog::Catalog(std::vector<std::shared_ptr<Course>> courses, std::string title, std::shared_ptr<Category> category)
    : courses(std::move(courses)), title(std::move(title)), category(std::move(category))
{
    for (const auto &item : this->courses)
        present(item);
}

const std::vector<std::shared_ptr<Course>> &Catalog::getCourses() const
{
    return courses;
}

const std::string &Catalog::getTitle() const
{
    return title;
}

const std::shared_ptr<Category> &Catalog::getCategory() const
{
    return category;
}

void Catalog::include(std::shared_ptr<Course> course)
{
    present(course);
    require(std::find(courses.begin(), courses.end(), course) == courses.end(), "Duplicate course");
    courses.push_back(std::move(course));
}

std::vector<std::shared_ptr<Course>> Catalog::search(const std::string &query) const
{
    nonempty(query);
    std::vector<std::shared_ptr<Course>> found;
    for (const auto &course : courses)
        if (course->getPublished() && course->getTitle().find(query) != std::string::npos)
            found.push_back(course);
    return found;
}

SearchQuery::SearchQuery(std::string text, Money budget, int minimumLessons)
    : text(std::move(text)), budget(std::move(budget)), minimumLessons(std::move(minimumLessons))
{
    money(budget);
    require(minimumLessons >= 0, "Negative minimumLessons");
}

const std::string &SearchQuery::getText() const
{
    return text;
}

Money SearchQuery::getBudget() const
{
    return budget;
}

int SearchQuery::getMinimumLessons() const
{
    return minimumLessons;
}

bool SearchQuery::accepts(const Course &course) const
{
    money(budget);
    require(minimumLessons >= 0, "Invalid minimum lessons");
    return course.getPublished() && course.getPrice() <= budget && course.lessonCount() >= minimumLessons &&
           course.getTitle().find(text) != std::string::npos;
}

std::vector<std::shared_ptr<Course>> SearchQuery::execute(const Catalog &catalog) const
{
    std::vector<std::shared_ptr<Course>> found;
    for (const auto &course : catalog.getCourses())
    {
        present(course);
        if (accepts(*course))
            found.push_back(course);
    }
    return found;
}

Wishlist::Wishlist(std::shared_ptr<Student> student, std::vector<std::shared_ptr<Course>> courses, int capacity)
    : student(std::move(student)), courses(std::move(courses)), capacity(std::move(capacity))
{
    for (const auto &item : this->courses)
        present(item);
    require(capacity >= 0, "Negative capacity");
}

const std::shared_ptr<Student> &Wishlist::getStudent() const
{
    return student;
}

const std::vector<std::shared_ptr<Course>> &Wishlist::getCourses() const
{
    return courses;
}

int Wishlist::getCapacity() const
{
    return capacity;
}

void Wishlist::save(std::shared_ptr<Course> course)
{
    present(course);
    require(course->getPublished() && capacity > 0 && courses.size() < static_cast<std::size_t>(capacity),
            "Wishlist unavailable");
    require(std::find(courses.begin(), courses.end(), course) == courses.end(), "Course already saved");
    courses.push_back(std::move(course));
}

void Wishlist::remove(const std::shared_ptr<Course> &course)
{
    auto it = std::find(courses.begin(), courses.end(), course);
    if (it == courses.end())
        throw ResourceNotFound("Saved course not found");
    courses.erase(it);
}

VideoLesson::VideoLesson(std::string title, std::shared_ptr<Video> video) : Lesson(std::move(title), video)
{
    present(video);
    video->remaining(0);
}
int VideoLesson::duration() const
{
    present(getVideo());
    return getVideo()->remaining(0);
}
ArticleLesson::ArticleLesson(std::string title, std::shared_ptr<Article> article)
    : Lesson(std::move(title), nullptr, article)
{
    present(article);
    article->readingMinutes();
}
int ArticleLesson::duration() const
{
    present(getArticle());
    return getArticle()->readingMinutes() * 60;
}
MixedLesson::MixedLesson(std::string title, std::shared_ptr<Video> video, std::shared_ptr<Article> article)
    : Lesson(title, video, article), VideoLesson(title, video), ArticleLesson(title, article)
{
}
int MixedLesson::duration() const
{
    return VideoLesson::duration() + ArticleLesson::duration();
}
std::ostream &operator<<(std::ostream &out, const Lesson &lesson)
{
    return out << lesson.getTitle() << " (" << lesson.duration() << " s)";
}
}
