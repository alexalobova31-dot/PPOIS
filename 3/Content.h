#ifndef CONTENT_H
#define CONTENT_H
#include "Accounts.h"
#include <ostream>

namespace learning
{

class Tag
{
  private:
    std::string name;
    std::string description;
    bool visible;

  public:
    Tag(std::string name = {}, std::string description = {}, bool visible = true);
    const std::string &getName() const;
    const std::string &getDescription() const;
    bool getVisible() const;
    bool matches(const std::string &query) const;
    void hide();
};

class Category
{
  private:
    std::string name;
    std::vector<std::shared_ptr<Tag>> tags;
    bool enabled;

  public:
    Category(std::string name = {}, std::vector<std::shared_ptr<Tag>> tags = {}, bool enabled = true);
    const std::string &getName() const;
    const std::vector<std::shared_ptr<Tag>> &getTags() const;
    bool getEnabled() const;
    void addTag(std::shared_ptr<Tag> tag);
    bool covers(const std::string &query) const;
};

class Video
{
  private:
    std::string title;
    std::string url;
    int seconds;

  public:
    Video(std::string title = {}, std::string url = {}, int seconds = 0);
    const std::string &getTitle() const;
    const std::string &getUrl() const;
    int getSeconds() const;
    int remaining(int watched) const;
    void trim(int start, int end);
};

class Article
{
  private:
    std::string title;
    std::string body;
    int wordsPerMinute;

  public:
    Article(std::string title = {}, std::string body = {}, int wordsPerMinute = 200);
    const std::string &getTitle() const;
    const std::string &getBody() const;
    int getWordsPerMinute() const;
    int readingMinutes() const;
    void append(std::string text);
};

class Attachment
{
  private:
    std::string filename;
    std::string mediaType;
    int bytes;

  public:
    Attachment(std::string filename = {}, std::string mediaType = {}, int bytes = 0);
    const std::string &getFilename() const;
    const std::string &getMediaType() const;
    int getBytes() const;
    bool fits(int limit) const;
    void rename(std::string name);
};

class Lesson
{
  private:
    std::string title;
    std::shared_ptr<Video> video;
    std::shared_ptr<Article> article;

  public:
    Lesson(std::string title = {}, std::shared_ptr<Video> video = {}, std::shared_ptr<Article> article = {});
    virtual ~Lesson() = default;
    const std::string &getTitle() const;
    const std::shared_ptr<Video> &getVideo() const;
    const std::shared_ptr<Article> &getArticle() const;
    virtual int duration() const;
    virtual bool contains(const std::string &query) const;
};

class Module
{
  private:
    std::string title;
    std::vector<std::shared_ptr<Lesson>> lessons;
    bool published;

  public:
    Module(std::string title = {}, std::vector<std::shared_ptr<Lesson>> lessons = {}, bool published = false);
    const std::string &getTitle() const;
    const std::vector<std::shared_ptr<Lesson>> &getLessons() const;
    bool getPublished() const;
    void addLesson(std::shared_ptr<Lesson> lesson);
    void publish();
};

class Course
{
  private:
    std::string title;
    std::shared_ptr<Instructor> instructor;
    std::vector<std::shared_ptr<Module>> modules;
    Money price;
    bool published;

  public:
    Course(std::string title = {}, std::shared_ptr<Instructor> instructor = {},
           std::vector<std::shared_ptr<Module>> modules = {}, Money price = 0, bool published = false);
    const std::string &getTitle() const;
    const std::shared_ptr<Instructor> &getInstructor() const;
    const std::vector<std::shared_ptr<Module>> &getModules() const;
    Money getPrice() const;
    bool getPublished() const;
    void publish();
    int lessonCount() const;
};

class Catalog
{
  private:
    std::vector<std::shared_ptr<Course>> courses;
    std::string title;
    std::shared_ptr<Category> category;

  public:
    Catalog(std::vector<std::shared_ptr<Course>> courses = {}, std::string title = {},
            std::shared_ptr<Category> category = {});
    const std::vector<std::shared_ptr<Course>> &getCourses() const;
    const std::string &getTitle() const;
    const std::shared_ptr<Category> &getCategory() const;
    void include(std::shared_ptr<Course> course);
    std::vector<std::shared_ptr<Course>> search(const std::string &query) const;
};

class SearchQuery
{
  private:
    std::string text;
    Money budget;
    int minimumLessons;

  public:
    SearchQuery(std::string text = {}, Money budget = 0, int minimumLessons = 1);
    const std::string &getText() const;
    Money getBudget() const;
    int getMinimumLessons() const;
    bool accepts(const Course &course) const;
    std::vector<std::shared_ptr<Course>> execute(const Catalog &catalog) const;
};

class Wishlist
{
  private:
    std::shared_ptr<Student> student;
    std::vector<std::shared_ptr<Course>> courses;
    int capacity;

  public:
    Wishlist(std::shared_ptr<Student> student = {}, std::vector<std::shared_ptr<Course>> courses = {},
             int capacity = 20);
    const std::shared_ptr<Student> &getStudent() const;
    const std::vector<std::shared_ptr<Course>> &getCourses() const;
    int getCapacity() const;
    void save(std::shared_ptr<Course> course);
    void remove(const std::shared_ptr<Course> &course);
};

class VideoLesson : public virtual Lesson
{
  public:
    VideoLesson(std::string title, std::shared_ptr<Video> video);
    int duration() const override;
};

class ArticleLesson : public virtual Lesson
{
  public:
    ArticleLesson(std::string title, std::shared_ptr<Article> article);
    int duration() const override;
};

class MixedLesson : public VideoLesson, public ArticleLesson
{
  public:
    MixedLesson(std::string title, std::shared_ptr<Video> video, std::shared_ptr<Article> article);
    int duration() const override;
};

std::ostream &operator<<(std::ostream &out, const Lesson &lesson);
}
#endif
