#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <stdexcept>
#include <string>
namespace learning
{
class ValidationError : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class AccessDenied : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class CourseUnavailable : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class AlreadyEnrolled : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class NotEnrolled : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class LessonLocked : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class AttemptLimit : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class DeadlineMissed : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class InvalidGrade : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class PaymentFailed : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class CertificateUnavailable : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
class ResourceNotFound : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
#endif
