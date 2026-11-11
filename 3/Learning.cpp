#include "Learning.h"

namespace learning
{

Enrollment::Enrollment(std::shared_ptr<Student> student, std::shared_ptr<Course> course, bool active)
    : student(std::move(student)), course(std::move(course)), active(std::move(active))
{
}

const std::shared_ptr<Student> &Enrollment::getStudent() const
{
    return student;
}

const std::shared_ptr<Course> &Enrollment::getCourse() const
{
    return course;
}

bool Enrollment::getActive() const
{
    return active;
}

void Enrollment::enroll()
{
    present(student);
    present(student->getUser());
    present(course);
    if (active)
        throw AlreadyEnrolled("Enrollment already active");
    if (!student->getUser()->getActive() || !course->getPublished())
        throw CourseUnavailable("Course or account unavailable");
    active = true;
}

void Enrollment::withdraw()
{
    if (!active)
        throw NotEnrolled("Enrollment inactive");
    active = false;
}

Progress::Progress(std::shared_ptr<Enrollment> enrollment, std::vector<std::shared_ptr<Lesson>> completed,
                   int targetPercent)
    : enrollment(std::move(enrollment)), completed(std::move(completed)), targetPercent(std::move(targetPercent))
{
    for (const auto &item : this->completed)
        present(item);
    require(targetPercent >= 0, "Negative targetPercent");
}

const std::shared_ptr<Enrollment> &Progress::getEnrollment() const
{
    return enrollment;
}

const std::vector<std::shared_ptr<Lesson>> &Progress::getCompleted() const
{
    return completed;
}

int Progress::getTargetPercent() const
{
    return targetPercent;
}

void Progress::complete(std::shared_ptr<Lesson> lesson)
{
    present(enrollment);
    present(enrollment->getCourse());
    present(lesson);
    if (!enrollment->getActive())
        throw NotEnrolled("Enrollment inactive");
    bool belongs = false;
    for (const auto &module : enrollment->getCourse()->getModules())
    {
        present(module);
        belongs = belongs || std::find(module->getLessons().begin(), module->getLessons().end(), lesson) !=
                                 module->getLessons().end();
    }
    if (!belongs)
        throw LessonLocked("Lesson is outside course");
    lesson->duration();
    require(std::find(completed.begin(), completed.end(), lesson) == completed.end(), "Lesson already completed");
    completed.push_back(std::move(lesson));
}

int Progress::percent() const
{
    present(enrollment);
    present(enrollment->getCourse());
    int total = enrollment->getCourse()->lessonCount();
    return total == 0 ? 0 : static_cast<int>(completed.size() * 100 / total);
}

LearningSession::LearningSession(std::shared_ptr<Enrollment> enrollment, std::shared_ptr<Lesson> lesson, int watched)
    : enrollment(std::move(enrollment)), lesson(std::move(lesson)), watched(std::move(watched))
{
    require(watched >= 0, "Negative watched");
}

const std::shared_ptr<Enrollment> &LearningSession::getEnrollment() const
{
    return enrollment;
}

const std::shared_ptr<Lesson> &LearningSession::getLesson() const
{
    return lesson;
}

int LearningSession::getWatched() const
{
    return watched;
}

void LearningSession::study(int seconds)
{
    present(enrollment);
    present(enrollment->getCourse());
    present(lesson);
    if (!enrollment->getActive())
        throw NotEnrolled("Enrollment inactive");
    bool belongs = false;
    for (const auto &module : enrollment->getCourse()->getModules())
    {
        present(module);
        belongs = belongs || std::find(module->getLessons().begin(), module->getLessons().end(), lesson) !=
                                 module->getLessons().end();
    }
    if (!belongs)
        throw LessonLocked("Lesson is outside course");
    require(seconds > 0 && watched >= 0 && seconds <= lesson->duration() - watched, "Invalid study interval");
    watched += seconds;
}

bool LearningSession::finished() const
{
    present(lesson);
    return watched >= lesson->duration();
}

Bookmark::Bookmark(std::shared_ptr<Student> student, std::shared_ptr<Lesson> lesson, int position)
    : student(std::move(student)), lesson(std::move(lesson)), position(std::move(position))
{
    require(position >= 0, "Negative position");
}

const std::shared_ptr<Student> &Bookmark::getStudent() const
{
    return student;
}

const std::shared_ptr<Lesson> &Bookmark::getLesson() const
{
    return lesson;
}

int Bookmark::getPosition() const
{
    return position;
}

void Bookmark::jump(int second)
{
    present(lesson);
    require(second >= 0 && second <= lesson->duration(), "Invalid bookmark");
    position = second;
}

int Bookmark::remaining() const
{
    present(lesson);
    require(position >= 0 && position <= lesson->duration(), "Invalid bookmark");
    return lesson->duration() - position;
}

Note::Note(std::shared_ptr<Student> student, std::shared_ptr<Lesson> lesson, std::string text)
    : student(std::move(student)), lesson(std::move(lesson)), text(std::move(text))
{
}

const std::shared_ptr<Student> &Note::getStudent() const
{
    return student;
}

const std::shared_ptr<Lesson> &Note::getLesson() const
{
    return lesson;
}

const std::string &Note::getText() const
{
    return text;
}

void Note::append(std::string fragment)
{
    nonempty(fragment);
    if (!text.empty())
        text += '\n';
    text += fragment;
}

bool Note::mentions(const std::string &term) const
{
    nonempty(term);
    return text.find(term) != std::string::npos;
}

Question::Question(std::string text, std::vector<std::string> options, int correct)
    : text(std::move(text)), options(std::move(options)), correct(std::move(correct))
{
    require(correct >= 0, "Negative correct");
}

const std::string &Question::getText() const
{
    return text;
}

const std::vector<std::string> &Question::getOptions() const
{
    return options;
}

int Question::getCorrect() const
{
    return correct;
}

bool Question::accepts(int choice) const
{
    require(!options.empty() && correct >= 0 && correct < static_cast<int>(options.size()), "Invalid question");
    require(choice >= 0 && choice < static_cast<int>(options.size()), "Invalid choice");
    return choice == correct;
}

void Question::addOption(std::string option)
{
    nonempty(option);
    require(std::find(options.begin(), options.end(), option) == options.end(), "Duplicate option");
    options.push_back(std::move(option));
}

Quiz::Quiz(std::shared_ptr<Lesson> lesson, std::vector<std::shared_ptr<Question>> questions, int passPercent)
    : lesson(std::move(lesson)), questions(std::move(questions)), passPercent(std::move(passPercent))
{
    for (const auto &item : this->questions)
        present(item);
    require(passPercent >= 0, "Negative passPercent");
}

const std::shared_ptr<Lesson> &Quiz::getLesson() const
{
    return lesson;
}

const std::vector<std::shared_ptr<Question>> &Quiz::getQuestions() const
{
    return questions;
}

int Quiz::getPassPercent() const
{
    return passPercent;
}

void Quiz::addQuestion(std::shared_ptr<Question> question)
{
    present(question);
    nonempty(question->getText());
    question->accepts(question->getCorrect());
    require(std::find(questions.begin(), questions.end(), question) == questions.end(), "Duplicate question");
    questions.push_back(std::move(question));
}

bool Quiz::passed(int correct) const
{
    require(!questions.empty() && passPercent >= 0 && passPercent <= 100 && correct >= 0 &&
                correct <= static_cast<int>(questions.size()),
            "Invalid quiz result");
    return correct * 100 >= passPercent * static_cast<int>(questions.size());
}

Answer::Answer(std::shared_ptr<Question> question, int choice, bool submitted)
    : question(std::move(question)), choice(std::move(choice)), submitted(std::move(submitted))
{
    require(choice >= 0, "Negative choice");
}

const std::shared_ptr<Question> &Answer::getQuestion() const
{
    return question;
}

int Answer::getChoice() const
{
    return choice;
}

bool Answer::getSubmitted() const
{
    return submitted;
}

void Answer::submit(int value)
{
    present(question);
    require(!submitted, "Answer already submitted");
    question->accepts(value);
    choice = value;
    submitted = true;
}

bool Answer::correct() const
{
    present(question);
    require(submitted, "Answer not submitted");
    return question->accepts(choice);
}

Attempt::Attempt(std::shared_ptr<Student> student, std::shared_ptr<Quiz> quiz, std::vector<Answer> answers, bool closed)
    : student(std::move(student)), quiz(std::move(quiz)), answers(std::move(answers)), closed(std::move(closed))
{
}

const std::shared_ptr<Student> &Attempt::getStudent() const
{
    return student;
}

const std::shared_ptr<Quiz> &Attempt::getQuiz() const
{
    return quiz;
}

const std::vector<Answer> &Attempt::getAnswers() const
{
    return answers;
}

bool Attempt::getClosed() const
{
    return closed;
}

void Attempt::answer(Answer value)
{
    present(quiz);
    require(!closed, "Attempt closed");
    if (answers.size() >= quiz->getQuestions().size())
        throw AttemptLimit("All questions answered");
    require(value.getSubmitted() && std::find(quiz->getQuestions().begin(), quiz->getQuestions().end(),
                                              value.getQuestion()) != quiz->getQuestions().end(),
            "Foreign or unsubmitted answer");
    require(std::none_of(answers.begin(), answers.end(),
                         [&](const auto &a) { return a.getQuestion() == value.getQuestion(); }),
            "Duplicate answer");
    answers.push_back(std::move(value));
}

bool Attempt::finish()
{
    present(quiz);
    require(!closed && answers.size() == quiz->getQuestions().size(), "Attempt incomplete or closed");
    int correct =
        static_cast<int>(std::count_if(answers.begin(), answers.end(), [](const auto &a) { return a.correct(); }));
    bool result = quiz->passed(correct);
    closed = true;
    return result;
}

Assignment::Assignment(std::shared_ptr<Lesson> lesson, std::string description, int deadline)
    : lesson(std::move(lesson)), description(std::move(description)), deadline(std::move(deadline))
{
    require(deadline >= 0, "Negative deadline");
}

const std::shared_ptr<Lesson> &Assignment::getLesson() const
{
    return lesson;
}

const std::string &Assignment::getDescription() const
{
    return description;
}

int Assignment::getDeadline() const
{
    return deadline;
}

void Assignment::extend(int days)
{
    require(days > 0 && days <= 365 && deadline >= 0 && deadline <= 1000000 - days, "Invalid extension");
    deadline += days;
}

bool Assignment::accepts(int day) const
{
    require(day >= 0 && deadline >= 0, "Invalid date");
    return day <= deadline;
}

Submission::Submission(std::shared_ptr<Assignment> assignment, std::shared_ptr<Student> student, std::string content,
                       bool submitted)
    : assignment(std::move(assignment)), student(std::move(student)), content(std::move(content)),
      submitted(std::move(submitted))
{
}

const std::shared_ptr<Assignment> &Submission::getAssignment() const
{
    return assignment;
}

const std::shared_ptr<Student> &Submission::getStudent() const
{
    return student;
}

const std::string &Submission::getContent() const
{
    return content;
}

bool Submission::getSubmitted() const
{
    return submitted;
}

void Submission::submit(std::string text, int day)
{
    present(assignment);
    present(student);
    require(!submitted, "Already submitted");
    if (!assignment->accepts(day))
        throw DeadlineMissed("Assignment deadline passed");
    nonempty(text);
    content = std::move(text);
    submitted = true;
}

void Submission::revise(std::string text, int day)
{
    present(assignment);
    require(submitted, "Nothing to revise");
    if (!assignment->accepts(day))
        throw DeadlineMissed("Revision deadline passed");
    nonempty(text);
    content = std::move(text);
}

Rubric::Rubric(std::string criterion, int maximum, int passScore)
    : criterion(std::move(criterion)), maximum(std::move(maximum)), passScore(std::move(passScore))
{
    require(maximum >= 0, "Negative maximum");
    require(passScore >= 0, "Negative passScore");
    passes(0);
}

const std::string &Rubric::getCriterion() const
{
    return criterion;
}

int Rubric::getMaximum() const
{
    return maximum;
}

int Rubric::getPassScore() const
{
    return passScore;
}

bool Rubric::passes(int score) const
{
    require(maximum > 0 && passScore >= 0 && passScore <= maximum, "Invalid rubric");
    if (score < 0 || score > maximum)
        throw InvalidGrade("Score outside rubric");
    return score >= passScore;
}

double Rubric::normalize(int score) const
{
    passes(score);
    return 100.0 * score / maximum;
}

Grade::Grade(std::shared_ptr<Submission> submission, std::shared_ptr<Rubric> rubric, int score)
    : submission(std::move(submission)), rubric(std::move(rubric)), score(std::move(score))
{
    require(score >= 0, "Negative score");
}

const std::shared_ptr<Submission> &Grade::getSubmission() const
{
    return submission;
}

const std::shared_ptr<Rubric> &Grade::getRubric() const
{
    return rubric;
}

int Grade::getScore() const
{
    return score;
}

void Grade::award(int value)
{
    present(submission);
    present(rubric);
    require(submission->getSubmitted(), "Submission not submitted");
    rubric->passes(value);
    score = value;
}

bool Grade::passed() const
{
    present(submission);
    present(rubric);
    require(submission->getSubmitted(), "Submission not submitted");
    return rubric->passes(score);
}

Gradebook::Gradebook(std::shared_ptr<Student> student, std::vector<std::shared_ptr<Grade>> grades,
                     double requiredAverage)
    : student(std::move(student)), grades(std::move(grades)), requiredAverage(std::move(requiredAverage))
{
    for (const auto &item : this->grades)
        present(item);
}

const std::shared_ptr<Student> &Gradebook::getStudent() const
{
    return student;
}

const std::vector<std::shared_ptr<Grade>> &Gradebook::getGrades() const
{
    return grades;
}

double Gradebook::getRequiredAverage() const
{
    return requiredAverage;
}

void Gradebook::record(std::shared_ptr<Grade> grade)
{
    present(student);
    present(grade);
    present(grade->getSubmission());
    require(grade->getSubmission()->getStudent() == student, "Foreign grade");
    grade->passed();
    require(std::none_of(grades.begin(), grades.end(),
                         [&](const auto &g) { return g->getSubmission() == grade->getSubmission(); }),
            "Submission already graded");
    grades.push_back(std::move(grade));
}

double Gradebook::average() const
{
    require(!grades.empty(), "No grades");
    double sum = 0;
    for (const auto &grade : grades)
        sum += grade->getRubric()->normalize(grade->getScore());
    return sum / grades.size();
}

Certificate::Certificate(std::shared_ptr<Progress> progress, std::string serial, bool issued)
    : progress(std::move(progress)), serial(std::move(serial)), issued(std::move(issued))
{
}

const std::shared_ptr<Progress> &Certificate::getProgress() const
{
    return progress;
}

const std::string &Certificate::getSerial() const
{
    return serial;
}

bool Certificate::getIssued() const
{
    return issued;
}

void Certificate::issue(std::string number)
{
    present(progress);
    present(progress->getEnrollment());
    if (issued || !progress->getEnrollment()->getActive() || progress->percent() != 100)
        throw CertificateUnavailable("Course incomplete or certificate already issued");
    nonempty(number);
    serial = std::move(number);
    issued = true;
}

bool Certificate::verify(const std::string &number) const
{
    return issued && !number.empty() && serial == number;
}

Review::Review(std::shared_ptr<Enrollment> enrollment, int stars, std::string text)
    : enrollment(std::move(enrollment)), stars(std::move(stars)), text(std::move(text))
{
    require(stars >= 0, "Negative stars");
}

const std::shared_ptr<Enrollment> &Review::getEnrollment() const
{
    return enrollment;
}

int Review::getStars() const
{
    return stars;
}

const std::string &Review::getText() const
{
    return text;
}

void Review::publish(int rating, std::string body)
{
    present(enrollment);
    if (!enrollment->getActive())
        throw NotEnrolled("Enrollment inactive");
    require(rating >= 1 && rating <= 5, "Invalid rating");
    nonempty(body);
    stars = rating;
    text = std::move(body);
}

bool Review::recommends() const
{
    require(stars >= 1 && stars <= 5, "Review not published");
    return stars >= 4;
}

StudyGoal::StudyGoal(std::string title, int target, int achieved)
    : title(std::move(title)), target(std::move(target)), achieved(std::move(achieved))
{
    require(target >= 0, "Negative target");
    require(achieved >= 0, "Negative achieved");
}

const std::string &StudyGoal::getTitle() const
{
    return title;
}

int StudyGoal::getTarget() const
{
    return target;
}

int StudyGoal::getAchieved() const
{
    return achieved;
}

void StudyGoal::advance(int units)
{
    require(target > 0 && achieved >= 0 && units > 0 && units <= target - achieved, "Invalid goal progress");
    achieved += units;
}

bool StudyGoal::reached() const
{
    require(target > 0 && achieved >= 0 && achieved <= target, "Invalid goal");
    return achieved == target;
}

StudyPlan::StudyPlan(std::shared_ptr<Student> student, std::vector<std::shared_ptr<StudyGoal>> goals, int dailyMinutes)
    : student(std::move(student)), goals(std::move(goals)), dailyMinutes(std::move(dailyMinutes))
{
    for (const auto &item : this->goals)
        present(item);
    require(dailyMinutes >= 0, "Negative dailyMinutes");
}

const std::shared_ptr<Student> &StudyPlan::getStudent() const
{
    return student;
}

const std::vector<std::shared_ptr<StudyGoal>> &StudyPlan::getGoals() const
{
    return goals;
}

int StudyPlan::getDailyMinutes() const
{
    return dailyMinutes;
}

void StudyPlan::addGoal(std::shared_ptr<StudyGoal> goal)
{
    present(goal);
    goal->reached();
    require(std::find(goals.begin(), goals.end(), goal) == goals.end(), "Duplicate goal");
    goals.push_back(std::move(goal));
}

bool StudyPlan::completed() const
{
    return !goals.empty() && std::all_of(goals.begin(), goals.end(), [](const auto &g) { return g->reached(); });
}
}
