#ifndef LEARNING_H
#define LEARNING_H
#include "Content.h"

namespace learning
{

class Enrollment
{
  private:
    std::shared_ptr<Student> student;
    std::shared_ptr<Course> course;
    bool active;

  public:
    Enrollment(std::shared_ptr<Student> student = {}, std::shared_ptr<Course> course = {}, bool active = false);
    const std::shared_ptr<Student> &getStudent() const;
    const std::shared_ptr<Course> &getCourse() const;
    bool getActive() const;
    void enroll();
    void withdraw();
};

class Progress
{
  private:
    std::shared_ptr<Enrollment> enrollment;
    std::vector<std::shared_ptr<Lesson>> completed;
    int targetPercent;

  public:
    Progress(std::shared_ptr<Enrollment> enrollment = {}, std::vector<std::shared_ptr<Lesson>> completed = {},
             int targetPercent = 100);
    const std::shared_ptr<Enrollment> &getEnrollment() const;
    const std::vector<std::shared_ptr<Lesson>> &getCompleted() const;
    int getTargetPercent() const;
    void complete(std::shared_ptr<Lesson> lesson);
    int percent() const;
};

class LearningSession
{
  private:
    std::shared_ptr<Enrollment> enrollment;
    std::shared_ptr<Lesson> lesson;
    int watched;

  public:
    LearningSession(std::shared_ptr<Enrollment> enrollment = {}, std::shared_ptr<Lesson> lesson = {}, int watched = 0);
    const std::shared_ptr<Enrollment> &getEnrollment() const;
    const std::shared_ptr<Lesson> &getLesson() const;
    int getWatched() const;
    void study(int seconds);
    bool finished() const;
};

class Bookmark
{
  private:
    std::shared_ptr<Student> student;
    std::shared_ptr<Lesson> lesson;
    int position;

  public:
    Bookmark(std::shared_ptr<Student> student = {}, std::shared_ptr<Lesson> lesson = {}, int position = 0);
    const std::shared_ptr<Student> &getStudent() const;
    const std::shared_ptr<Lesson> &getLesson() const;
    int getPosition() const;
    void jump(int second);
    int remaining() const;
};

class Note
{
  private:
    std::shared_ptr<Student> student;
    std::shared_ptr<Lesson> lesson;
    std::string text;

  public:
    Note(std::shared_ptr<Student> student = {}, std::shared_ptr<Lesson> lesson = {}, std::string text = {});
    const std::shared_ptr<Student> &getStudent() const;
    const std::shared_ptr<Lesson> &getLesson() const;
    const std::string &getText() const;
    void append(std::string fragment);
    bool mentions(const std::string &term) const;
};

class Question
{
  private:
    std::string text;
    std::vector<std::string> options;
    int correct;

  public:
    Question(std::string text = {}, std::vector<std::string> options = {}, int correct = 0);
    const std::string &getText() const;
    const std::vector<std::string> &getOptions() const;
    int getCorrect() const;
    bool accepts(int choice) const;
    void addOption(std::string option);
};

class Quiz
{
  private:
    std::shared_ptr<Lesson> lesson;
    std::vector<std::shared_ptr<Question>> questions;
    int passPercent;

  public:
    Quiz(std::shared_ptr<Lesson> lesson = {}, std::vector<std::shared_ptr<Question>> questions = {},
         int passPercent = 60);
    const std::shared_ptr<Lesson> &getLesson() const;
    const std::vector<std::shared_ptr<Question>> &getQuestions() const;
    int getPassPercent() const;
    void addQuestion(std::shared_ptr<Question> question);
    bool passed(int correct) const;
};

class Answer
{
  private:
    std::shared_ptr<Question> question;
    int choice;
    bool submitted;

  public:
    Answer(std::shared_ptr<Question> question = {}, int choice = 0, bool submitted = false);
    const std::shared_ptr<Question> &getQuestion() const;
    int getChoice() const;
    bool getSubmitted() const;
    void submit(int value);
    bool correct() const;
};

class Attempt
{
  private:
    std::shared_ptr<Student> student;
    std::shared_ptr<Quiz> quiz;
    std::vector<Answer> answers;
    bool closed;

  public:
    Attempt(std::shared_ptr<Student> student = {}, std::shared_ptr<Quiz> quiz = {}, std::vector<Answer> answers = {},
            bool closed = false);
    const std::shared_ptr<Student> &getStudent() const;
    const std::shared_ptr<Quiz> &getQuiz() const;
    const std::vector<Answer> &getAnswers() const;
    bool getClosed() const;
    void answer(Answer value);
    bool finish();
};

class Assignment
{
  private:
    std::shared_ptr<Lesson> lesson;
    std::string description;
    int deadline;

  public:
    Assignment(std::shared_ptr<Lesson> lesson = {}, std::string description = {}, int deadline = 0);
    const std::shared_ptr<Lesson> &getLesson() const;
    const std::string &getDescription() const;
    int getDeadline() const;
    void extend(int days);
    bool accepts(int day) const;
};

class Submission
{
  private:
    std::shared_ptr<Assignment> assignment;
    std::shared_ptr<Student> student;
    std::string content;
    bool submitted;

  public:
    Submission(std::shared_ptr<Assignment> assignment = {}, std::shared_ptr<Student> student = {},
               std::string content = {}, bool submitted = false);
    const std::shared_ptr<Assignment> &getAssignment() const;
    const std::shared_ptr<Student> &getStudent() const;
    const std::string &getContent() const;
    bool getSubmitted() const;
    void submit(std::string text, int day);
    void revise(std::string text, int day);
};

class Rubric
{
  private:
    std::string criterion;
    int maximum;
    int passScore;

  public:
    Rubric(std::string criterion = {}, int maximum = 100, int passScore = 60);
    const std::string &getCriterion() const;
    int getMaximum() const;
    int getPassScore() const;
    bool passes(int score) const;
    double normalize(int score) const;
};

class Grade
{
  private:
    std::shared_ptr<Submission> submission;
    std::shared_ptr<Rubric> rubric;
    int score;

  public:
    Grade(std::shared_ptr<Submission> submission = {}, std::shared_ptr<Rubric> rubric = {}, int score = 0);
    const std::shared_ptr<Submission> &getSubmission() const;
    const std::shared_ptr<Rubric> &getRubric() const;
    int getScore() const;
    void award(int value);
    bool passed() const;
};

class Gradebook
{
  private:
    std::shared_ptr<Student> student;
    std::vector<std::shared_ptr<Grade>> grades;
    double requiredAverage;

  public:
    Gradebook(std::shared_ptr<Student> student = {}, std::vector<std::shared_ptr<Grade>> grades = {},
              double requiredAverage = 60);
    const std::shared_ptr<Student> &getStudent() const;
    const std::vector<std::shared_ptr<Grade>> &getGrades() const;
    double getRequiredAverage() const;
    void record(std::shared_ptr<Grade> grade);
    double average() const;
};

class Certificate
{
  private:
    std::shared_ptr<Progress> progress;
    std::string serial;
    bool issued;

  public:
    Certificate(std::shared_ptr<Progress> progress = {}, std::string serial = {}, bool issued = false);
    const std::shared_ptr<Progress> &getProgress() const;
    const std::string &getSerial() const;
    bool getIssued() const;
    void issue(std::string number);
    bool verify(const std::string &number) const;
};

class Review
{
  private:
    std::shared_ptr<Enrollment> enrollment;
    int stars;
    std::string text;

  public:
    Review(std::shared_ptr<Enrollment> enrollment = {}, int stars = 0, std::string text = {});
    const std::shared_ptr<Enrollment> &getEnrollment() const;
    int getStars() const;
    const std::string &getText() const;
    void publish(int rating, std::string body);
    bool recommends() const;
};

class StudyGoal
{
  private:
    std::string title;
    int target;
    int achieved;

  public:
    StudyGoal(std::string title = {}, int target = 1, int achieved = 0);
    const std::string &getTitle() const;
    int getTarget() const;
    int getAchieved() const;
    void advance(int units);
    bool reached() const;
};

class StudyPlan
{
  private:
    std::shared_ptr<Student> student;
    std::vector<std::shared_ptr<StudyGoal>> goals;
    int dailyMinutes;

  public:
    StudyPlan(std::shared_ptr<Student> student = {}, std::vector<std::shared_ptr<StudyGoal>> goals = {},
              int dailyMinutes = 30);
    const std::shared_ptr<Student> &getStudent() const;
    const std::vector<std::shared_ptr<StudyGoal>> &getGoals() const;
    int getDailyMinutes() const;
    void addGoal(std::shared_ptr<StudyGoal> goal);
    bool completed() const;
};
}
#endif
