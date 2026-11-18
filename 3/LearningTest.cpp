#include "Scenario.h"
#include <UnitTest++/UnitTest++.h>

TEST(EnrollmentProgressAndCertificate)
{
    Scenario s;
    CHECK_THROW(s.account->changeEmail("bad@@mail"), ValidationError);
    CHECK(s.account->getEmail() == "student@example.org");
    auto enrollment = s.platform.enroll(s.student, s.course);
    CHECK_THROW(s.platform.enroll(s.student, s.course), AlreadyEnrolled);
    CHECK(s.platform.activeStudents() == 1);
    Progress progress{enrollment, {}, 100};
    Certificate certificate{std::make_shared<Progress>(progress), "", false};
    CHECK_THROW(certificate.issue("CERT-1"), CertificateUnavailable);
    auto foreign = std::make_shared<Lesson>(*s.lesson);
    CHECK_THROW(progress.complete(foreign), LessonLocked);
    LearningSession session{enrollment, s.lesson, 0};
    session.study(60);
    CHECK_THROW(session.study(61), ValidationError);
    CHECK(session.getWatched() == 60);
    session.study(60);
    CHECK(session.finished());
    progress.complete(s.lesson);
    CHECK_THROW(progress.complete(s.lesson), ValidationError);
    CHECK(progress.percent() == 100);
    *certificate.getProgress() = progress;
    certificate.issue("CERT-1");
    CHECK(certificate.verify("CERT-1") && !certificate.verify("wrong"));
    CHECK_THROW(certificate.issue("CERT-2"), CertificateUnavailable);
    enrollment->withdraw();
    CHECK_THROW(progress.complete(s.lesson), NotEnrolled);
    auto hidden = std::make_shared<Course>("Hidden", s.course->getInstructor());
    Enrollment pending(s.student, hidden);
    CHECK_THROW(pending.enroll(), CourseUnavailable);
    auto duplicate = std::make_shared<Course>(
        "Duplicate", s.course->getInstructor(),
        std::vector<std::shared_ptr<Module>>{s.course->getModules().front(), s.course->getModules().front()});
    CHECK_THROW(duplicate->publish(), ValidationError);
    enrollment->enroll();
}

TEST(QuizAssignmentsAndGrades)
{
    Scenario s;
    auto question = std::make_shared<Question>(Question{"2 + 2?", {"3", "4"}, 1});
    auto quiz = std::make_shared<Quiz>(s.lesson);
    quiz->addQuestion(question);
    Attempt attempt{s.student, quiz, {}, false};
    CHECK_THROW(attempt.finish(), ValidationError);
    Answer answer{question, 0, false};
    CHECK_THROW(answer.submit(2), ValidationError);
    answer.submit(1);
    attempt.answer(answer);
    CHECK_THROW(attempt.answer(answer), AttemptLimit);
    CHECK(attempt.finish());
    CHECK_THROW(attempt.finish(), ValidationError);
    auto assignment = std::make_shared<Assignment>(Assignment{s.lesson, "Implement a class", 10});
    auto submission = std::make_shared<Submission>(Submission{assignment, s.student, "", false});
    CHECK_THROW(submission->submit("solution", 11), DeadlineMissed);
    CHECK(!submission->getSubmitted());
    submission->submit("solution", 10);
    auto rubric = std::make_shared<Rubric>(Rubric{"Correctness", 20, 12});
    auto grade = std::make_shared<Grade>(Grade{submission, rubric, 0});
    CHECK_THROW(grade->award(21), InvalidGrade);
    grade->award(16);
    CHECK(grade->passed());
    Gradebook book{s.student, {}, 60};
    book.record(grade);
    CHECK(book.average() == 80);
    CHECK_THROW(book.record(grade), ValidationError);
}

TEST(PaymentsDiscountsAndRefunds)
{
    Scenario s;
    auto payment = std::make_shared<Payment>(s.student, 8000);
    Invoice invoice(nullptr, s.course);
    CHECK_THROW(invoice.settle(payment), ValidationError);
    Coupon coupon{"LEARN20", 20, 1};
    invoice.calculate(&coupon);
    CHECK(invoice.getTotal() == 8000);
    CHECK_THROW(invoice.settle(payment), PaymentFailed);
    payment->capture();
    CHECK_THROW(payment->capture(), PaymentFailed);
    invoice.settle(payment);
    Invoice other(nullptr, s.course);
    other.calculate(&coupon);
    CHECK_THROW(other.settle(payment), PaymentFailed);
    coupon.redeem();
    CHECK_THROW(coupon.discount(10000), ValidationError);
    Refund refund{payment, "Course cancelled", false};
    refund.process();
    CHECK(refund.amount() == 8000 && !payment->getPaid());
    CHECK_THROW(payment->capture(), PaymentFailed);
    CHECK_THROW(refund.process(), PaymentFailed);
    Subscription subscription{s.student, 0, false};
    subscription.renew(10, 30);
    CHECK(subscription.active(39) && !subscription.active(40));
    subscription.renew(20, 10);
    CHECK(subscription.getExpires() == 50);
}

TEST(CalendarAndStudyGroups)
{
    Scenario s;
    auto event = std::make_shared<Event>(Event{"Lesson", 10, 20});
    Calendar calendar{s.account, {}, 10};
    calendar.schedule(event);
    CHECK_THROW(calendar.schedule(std::make_shared<Event>(Event{"Conflict", 19, 30})), ValidationError);
    calendar.schedule(std::make_shared<Event>(Event{"Next", 20, 30}));
    CHECK(calendar.occupied() == 20);
    auto group = std::make_shared<StudyGroup>();
    group->join(s.student);
    CHECK_THROW(group->join(s.student), ValidationError);
    GroupMeeting meeting{group, event, {}};
    meeting.attend(s.student);
    CHECK(meeting.participation() == 100);
    group->leave(s.student);
    CHECK_THROW(group->leave(s.student), ResourceNotFound);
}

TEST(CommunicationAndSearch)
{
    Scenario s;
    auto stranger = std::make_shared<User>();
    auto post = std::make_shared<Post>(Post{s.account, "Question", false});
    CHECK_THROW(post->edit(stranger, "Changed"), AccessDenied);
    post->edit(s.account, "Updated question");
    Thread thread{"Help", {}, false};
    thread.reply(post);
    CHECK(thread.visibleCount() == 1);
    Thread locked("Closed", {}, true);
    CHECK_THROW(locked.reply(post), AccessDenied);
    auto notification = std::make_shared<Notification>(Notification{s.account, "New lesson", false});
    Inbox inbox{s.account, {}, 10};
    inbox.deliver(notification);
    CHECK(inbox.unread() == 1);
    CHECK_THROW(notification->acknowledge(stranger), AccessDenied);
    notification->acknowledge(s.account);
    CHECK(inbox.unread() == 0);
    Message message{s.account, stranger, ""};
    message.compose("Hello");
    Conversation conversation{s.account, stranger, {}};
    conversation.send(std::make_shared<Message>(message));
    CHECK(conversation.receivedBy(stranger) == 1);
    Wishlist wishlist{s.student, {}, 20};
    wishlist.save(s.course);
    CHECK_THROW(wishlist.save(s.course), ValidationError);
    wishlist.remove(s.course);
    CHECK_THROW(wishlist.remove(s.course), ResourceNotFound);
    SearchQuery query{"C++", 10000, 1};
    CHECK(query.execute(*s.platform.getCatalog()).size() == 1);
    query = SearchQuery("C++", 9999, 1);
    CHECK(query.execute(*s.platform.getCatalog()).empty());
}

TEST(VirtualDiamondHasSingleLesson)
{
    auto video = std::make_shared<Video>("Video", "local:video", 120);
    auto article = std::make_shared<Article>("Article", "one two three", 3);
    MixedLesson mixed("Combined", video, article);
    const Lesson *first = static_cast<const VideoLesson *>(&mixed);
    const Lesson *second = static_cast<const ArticleLesson *>(&mixed);
    CHECK(first == second);
    CHECK_EQUAL(180, first->duration());
    CHECK_EQUAL(120, VideoLesson("Video", video).duration());
    CHECK_EQUAL(60, ArticleLesson("Text", article).duration());
}

TEST(ConstructorsRejectInvalidArguments)
{
    CHECK_THROW(User("invalid"), ValidationError);
    CHECK_THROW(Category("Category", {nullptr}), ValidationError);
    CHECK_THROW(Video("Video", "local:video", -1), ValidationError);
    CHECK_THROW(Payment(nullptr, -1), ValidationError);
    CHECK_THROW(Rubric("Invalid", 0, 0), ValidationError);
    CHECK_THROW(VideoLesson("Video", nullptr), ValidationError);
}

int main()
{
    return UnitTest::RunAllTests();
}
