# Лабораторная работа №3: платформа онлайн-обучения

Объектная модель на C++17 в стиле лабораторной №2: закрытые поля, конструкторы,
раздельные заголовки и реализации, наследование и консольное меню.
Приложение использует стандартную библиотеку, тесты — UnitTest++.

## Запуск

Нужны компилятор C++17 и Make (Linux/macOS). Для тестов нужны UnitTest++ и pkg-config,
как в лабораторной №2. На macOS: `brew install unittest-cpp pkg-config`;
на Debian/Ubuntu: `sudo apt install libunittest++-dev pkg-config`.
Без тестов приложение собирается без сторонних библиотек.

```sh
make run       # собрать и открыть меню
make example   # автоматический пример обучения и наследования
make test      # проверки успешных операций и ошибок
make sanitize  # проверки с AddressSanitizer и UndefinedBehaviorSanitizer
make clean     # удалить исполняемые файлы
```

`make` собирает `learning`, `make test` — `learning_test`. Компилятор можно выбрать: `make CXX=clang++`.
Код возврата: 0 — успех, 1 — исключение или провал проверки, 2 — неверные аргументы.

## Устройство и правила модели

| Файлы | Содержимое |
|---|---|
| `Accounts.h/.cpp` | Профили, пользователи, студенты, преподаватели |
| `Content.h/.cpp` | Материалы, уроки, модули, курсы, каталог и поиск |
| `Learning.h/.cpp` | Обучение, тесты, задания, оценки, сертификаты, учебные цели |
| `Community.h/.cpp` | Обсуждения, сообщения, уведомления, расписание и группы |
| `Finance.h/.cpp` | Купоны, платежи, счета, подписки и возвраты |
| `Platform.h/.cpp` | Запись на курсы и учет студентов |
| `Common.h/.cpp`, `Exceptions.h` | Проверки данных, тип суммы и исключения |
| `Scenario.h` | Общие исходные данные для меню, примера и тестов |
| `main.cpp` | Консольное меню и пример (`--example`) |
| `LearningTest.cpp` | Отдельная программа тестов UnitTest++ |
| `Makefile` | Сборка с учетом зависимостей заголовков, запуск и проверки |

Сценарий: преподаватель публикует модуль и курс, студент записывается,
завершает урок и получает сертификат. Тесты дополнительно проверяют тестирование,
сдачу заданий, оценки, платежи, расписание, сообщения и ограничения доступа.

Предметные сущности объявлены через `class`, поля закрыты (`private`).
Конструктор принимает значения полей в порядке таблицы ниже; в заголовках указаны
значения по умолчанию. Передаваемые коллекции связей проверяются на пустые указатели.
Для каждого поля `x` есть метод `getX() const`: числа возвращаются по значению,
строки, связи и коллекции — по константной ссылке. Сеттеров нет; изменение состояния
выполняется предметными методами. Конструкторы, деструктор и геттеры не входят
в количество содержательных методов.

Связи используют `shared_ptr`, коллекции — `vector`; циклического владения нет.
Обязательные для операции связи проверяются перед использованием.
Идентичность сущностей определяется экземпляром объекта.

`VideoLesson` и `ArticleLesson` виртуально наследуют `Lesson`, а `MixedLesson`
наследует обе специализации. Поэтому комбинированный урок содержит один базовый
объект `Lesson`. Метод `duration` виртуальный, вызывается через базовую ссылку;
деструктор `Lesson` виртуальный. Оператор `<<` печатает название и длительность.
В меню и тестах показаны полиморфный вызов и совпадение адресов базового объекта.

Денежные суммы — целые минимальные единицы одной условной валюты, диапазон
0–1 000 000 000; скидка округляется вниз. Оплата и возврат моделируют состояние,
без списания реальных денег. Цена не ограничивает запись на курс: платежный сценарий
в модели независим от записи. Купон рассчитывает скидку без расходования;
`redeem` вызывается отдельно при использовании купона.
Даты заданий и подписок — целые дни от условного начала отсчета, граница сдачи
включительна, граница окончания подписки исключительна. Интервалы событий —
полуоткрытые `[start, end)` в условных единицах. Длительность уроков — секунды.
Поиск чувствителен к регистру, время чтения считает слова по пробельным разделителям.
Сертификат требует завершения всех уроков; оценки заданий и тестов учитываются отдельно.

Это консольная учебная объектная модель: данные живут в памяти процесса,
без БД, веб-интерфейса, сетевой авторизации и многопоточности.
Пароли и секреты не используются. UnitTest++ нужен только для тестов.
Дополнительные условия из `lab2.md` не проверены: файл задания не предоставлен.

## Количественные требования

| Показатель | Требуется | Реализовано |
|---|---:|---:|
| Предметные классы (без исключений и тестового сценария) | 50 | 53 |
| Поля предметных классов | 150 | 156 |
| Содержательные методы (без конструкторов и вспомогательных функций) | 100 | 105 |
| Уникальные направленные связи между предметными классами | 30 | 72 |
| Собственные классы исключений | 12 | 12 |

Связь `A → B` учитывается один раз, если `B` входит в поле или параметр метода `A`;
`shared_ptr<B>`, `vector<B>` и `vector<shared_ptr<B>>` тоже задают связь.
Таблица перечисляет все поля и все предметные методы. В таблице указаны собственные поля и предметные методы без повторного учета наследуемых.
Для всех полей также доступны геттеры по описанному выше правилу.

## Классы

| Класс | Число полей | Число методов | Связанные классы | Поля и типы | Поведение методов |
|---|---:|---:|---|---|---|
| Profile | 3 | 2 | — | `name: std::string`; `bio: std::string`; `timezone: std::string` | `introduce` — обновляет биографию<br>`signature` — формирует подпись с именем и часовым поясом |
| User | 3 | 2 | Profile | `email: std::string`; `profile: std::shared_ptr<Profile>`; `active: bool` | `changeEmail` — проверяет и меняет email<br>`deactivate` — деактивирует активную учетную запись |
| Student | 3 | 2 | User | `user: std::shared_ptr<User>`; `credits: int`; `interests: std::vector<std::string>` | `earnCredits` — начисляет положительное число учебных баллов<br>`followTopic` — добавляет интерес без повторов |
| Instructor | 3 | 2 | User | `user: std::shared_ptr<User>`; `expertise: std::string`; `experience: int` | `qualify` — указывает специализацию и стаж<br>`canTeach` — проверяет возможность преподавать тему |
| Tag | 3 | 2 | — | `name: std::string`; `description: std::string`; `visible: bool` | `matches` — ищет подстроку в видимой метке<br>`hide` — скрывает метку |
| Category | 3 | 2 | Tag | `name: std::string`; `tags: std::vector<std::shared_ptr<Tag>>`; `enabled: bool` | `addTag` — добавляет метку с уникальным именем<br>`covers` — проверяет соответствие категории запросу |
| Video | 3 | 2 | — | `title: std::string`; `url: std::string`; `seconds: int` | `remaining` — вычисляет оставшиеся секунды<br>`trim` — обрезает длительность по границам фрагмента |
| Article | 3 | 2 | — | `title: std::string`; `body: std::string`; `wordsPerMinute: int` | `readingMinutes` — рассчитывает время чтения по числу слов<br>`append` — добавляет абзац |
| Attachment | 3 | 2 | — | `filename: std::string`; `mediaType: std::string`; `bytes: int` | `fits` — проверяет ограничение размера<br>`rename` — переименовывает файл без компонентов пути |
| Lesson | 3 | 2 | Article, Video | `title: std::string`; `video: std::shared_ptr<Video>`; `article: std::shared_ptr<Article>` | `duration` — суммирует длительность видео и чтения<br>`contains` — ищет текст в названии или статье |
| Module | 3 | 2 | Lesson | `title: std::string`; `lessons: std::vector<std::shared_ptr<Lesson>>`; `published: bool` | `addLesson` — добавляет уникальный урок до публикации<br>`publish` — публикует непустой модуль с содержимым |
| Course | 5 | 2 | Instructor, Module | `title: std::string`; `instructor: std::shared_ptr<Instructor>`; `modules: std::vector<std::shared_ptr<Module>>`; `price: Money`; `published: bool` | `publish` — публикует подготовленный курс без повторяющихся уроков<br>`lessonCount` — считает уроки всех модулей |
| Catalog | 3 | 2 | Category, Course | `courses: std::vector<std::shared_ptr<Course>>`; `title: std::string`; `category: std::shared_ptr<Category>` | `include` — включает курс без повторов<br>`search` — ищет опубликованные курсы по названию |
| Enrollment | 3 | 2 | Course, Student | `student: std::shared_ptr<Student>`; `course: std::shared_ptr<Course>`; `active: bool` | `enroll` — активирует запись при доступности курса и аккаунта<br>`withdraw` — отменяет активную запись |
| Progress | 3 | 2 | Enrollment, Lesson | `enrollment: std::shared_ptr<Enrollment>`; `completed: std::vector<std::shared_ptr<Lesson>>`; `targetPercent: int` | `complete` — завершает принадлежащий курсу урок без повторов<br>`percent` — рассчитывает процент завершения |
| LearningSession | 3 | 2 | Enrollment, Lesson | `enrollment: std::shared_ptr<Enrollment>`; `lesson: std::shared_ptr<Lesson>`; `watched: int` | `study` — учитывает время изучения урока своего курса в пределах длительности<br>`finished` — проверяет завершение сессии |
| Bookmark | 3 | 2 | Lesson, Student | `student: std::shared_ptr<Student>`; `lesson: std::shared_ptr<Lesson>`; `position: int` | `jump` — переносит закладку на допустимую секунду<br>`remaining` — рассчитывает оставшееся время |
| Note | 3 | 2 | Lesson, Student | `student: std::shared_ptr<Student>`; `lesson: std::shared_ptr<Lesson>`; `text: std::string` | `append` — добавляет фрагмент заметки<br>`mentions` — ищет упоминание термина |
| Question | 3 | 2 | — | `text: std::string`; `options: std::vector<std::string>`; `correct: int` | `accepts` — проверяет выбранный вариант ответа<br>`addOption` — добавляет уникальный вариант |
| Quiz | 3 | 2 | Lesson, Question | `lesson: std::shared_ptr<Lesson>`; `questions: std::vector<std::shared_ptr<Question>>`; `passPercent: int` | `addQuestion` — добавляет корректный уникальный вопрос<br>`passed` — проверяет прохождение по числу верных ответов |
| Answer | 3 | 2 | Question | `question: std::shared_ptr<Question>`; `choice: int`; `submitted: bool` | `submit` — однократно отправляет допустимый вариант<br>`correct` — проверяет правильность отправленного ответа |
| Attempt | 4 | 2 | Answer, Quiz, Student | `student: std::shared_ptr<Student>`; `quiz: std::shared_ptr<Quiz>`; `answers: std::vector<Answer>`; `closed: bool` | `answer` — принимает отправленный ответ на вопрос теста без повторов<br>`finish` — закрывает полную попытку и определяет результат |
| Assignment | 3 | 2 | Lesson | `lesson: std::shared_ptr<Lesson>`; `description: std::string`; `deadline: int` | `extend` — продлевает срок сдачи<br>`accepts` — проверяет допустимость дня сдачи |
| Submission | 4 | 2 | Assignment, Student | `assignment: std::shared_ptr<Assignment>`; `student: std::shared_ptr<Student>`; `content: std::string`; `submitted: bool` | `submit` — отправляет решение до срока<br>`revise` — исправляет отправленное решение до срока |
| Rubric | 3 | 2 | — | `criterion: std::string`; `maximum: int`; `passScore: int` | `passes` — проверяет балл и проходной порог<br>`normalize` — переводит балл в проценты |
| Grade | 3 | 2 | Rubric, Submission | `submission: std::shared_ptr<Submission>`; `rubric: std::shared_ptr<Rubric>`; `score: int` | `award` — выставляет допустимую оценку отправленному решению<br>`passed` — проверяет прохождение |
| Gradebook | 3 | 2 | Grade, Student | `student: std::shared_ptr<Student>`; `grades: std::vector<std::shared_ptr<Grade>>`; `requiredAverage: double` | `record` — записывает оценку своего студента без повторного оценивания решения<br>`average` — рассчитывает средний процент |
| Certificate | 3 | 2 | Progress | `progress: std::shared_ptr<Progress>`; `serial: std::string`; `issued: bool` | `issue` — выдает сертификат после завершения всех уроков<br>`verify` — сверяет номер выданного сертификата |
| Review | 3 | 2 | Enrollment | `enrollment: std::shared_ptr<Enrollment>`; `stars: int`; `text: std::string` | `publish` — публикует отзыв записанного студента с оценкой 1–5<br>`recommends` — определяет рекомендацию по оценке от 4 |
| Post | 3 | 2 | User | `author: std::shared_ptr<User>`; `text: std::string`; `hidden: bool` | `edit` — разрешает редактирование активному автору видимого сообщения<br>`visible` — определяет видимость сообщения |
| Thread | 3 | 2 | Post | `title: std::string`; `posts: std::vector<std::shared_ptr<Post>>`; `locked: bool` | `reply` — добавляет видимое сообщение в открытую тему<br>`visibleCount` — считает видимые сообщения |
| Discussion | 3 | 2 | Course, Thread | `course: std::shared_ptr<Course>`; `threads: std::vector<std::shared_ptr<Thread>>`; `enabled: bool` | `open` — открывает тему в доступном обсуждении<br>`find` — находит тему по названию |
| Notification | 3 | 2 | User | `recipient: std::shared_ptr<User>`; `text: std::string`; `read: bool` | `acknowledge` — помечает уведомление прочитанным от имени получателя<br>`pending` — проверяет наличие непрочитанного уведомления активному пользователю |
| Inbox | 3 | 2 | Notification, User | `owner: std::shared_ptr<User>`; `notifications: std::vector<std::shared_ptr<Notification>>`; `capacity: int` | `deliver` — доставляет уведомление владельцу с учетом вместимости<br>`unread` — считает непрочитанные уведомления |
| Message | 3 | 2 | User | `sender: std::shared_ptr<User>`; `recipient: std::shared_ptr<User>`; `text: std::string` | `compose` — формирует сообщение между разными активными пользователями<br>`involves` — проверяет участие пользователя |
| Conversation | 3 | 2 | Message, User | `first: std::shared_ptr<User>`; `second: std::shared_ptr<User>`; `messages: std::vector<std::shared_ptr<Message>>` | `send` — отправляет сообщение между участниками беседы<br>`receivedBy` — считает полученные сообщения участника |
| Event | 3 | 2 | — | `title: std::string`; `start: int`; `end: int` | `overlaps` — проверяет пересечение временных интервалов<br>`reschedule` — переносит событие |
| Calendar | 3 | 2 | Event, User | `owner: std::shared_ptr<User>`; `events: std::vector<std::shared_ptr<Event>>`; `capacity: int` | `schedule` — добавляет событие без пересечений<br>`occupied` — суммирует занятое время |
| StudyGoal | 3 | 2 | — | `title: std::string`; `target: int`; `achieved: int` | `advance` — увеличивает прогресс в пределах цели<br>`reached` — проверяет достижение цели |
| StudyPlan | 3 | 2 | Student, StudyGoal | `student: std::shared_ptr<Student>`; `goals: std::vector<std::shared_ptr<StudyGoal>>`; `dailyMinutes: int` | `addGoal` — добавляет уникальную корректную цель<br>`completed` — проверяет выполнение всех целей непустого плана |
| StudyGroup | 3 | 2 | Student | `name: std::string`; `members: std::vector<std::shared_ptr<Student>>`; `capacity: int` | `join` — принимает активного студента с учетом вместимости<br>`leave` — исключает существующего участника |
| GroupMeeting | 3 | 2 | Event, Student, StudyGroup | `group: std::shared_ptr<StudyGroup>`; `event: std::shared_ptr<Event>`; `attendees: std::vector<std::shared_ptr<Student>>` | `attend` — регистрирует участника группы без повторов<br>`participation` — вычисляет долю текущих участников группы среди посетителей |
| SearchQuery | 3 | 2 | Catalog, Course | `text: std::string`; `budget: Money`; `minimumLessons: int` | `accepts` — проверяет курс по тексту бюджету и числу уроков<br>`execute` — фильтрует каталог |
| Coupon | 3 | 2 | — | `code: std::string`; `percent: int`; `uses: int` | `discount` — вычисляет скидку доступного купона<br>`redeem` — расходует одно применение |
| Payment | 4 | 4 | Student | `payer: std::shared_ptr<Student>`; `amount: Money`; `paid: bool`; `allocated: bool` | `capture` — моделирует однократное подтверждение платежа<br>`covers` — проверяет покрытие суммы подтвержденным платежом<br>`allocate` — закрепляет платеж за счетом один раз<br>`reverse` — отменяет подтверждение платежа |
| Invoice | 4 | 2 | Coupon, Course, Payment | `payment: std::shared_ptr<Payment>`; `course: std::shared_ptr<Course>`; `total: Money`; `calculated: bool` | `calculate` — рассчитывает стоимость курса со скидкой<br>`settle` — связывает счет с подтвержденным еще не использованным платежом |
| Subscription | 3 | 2 | Student | `student: std::shared_ptr<Student>`; `expires: int`; `cancelled: bool` | `renew` — продлевает подписку от текущего дня или существующего окончания<br>`active` — проверяет действие подписки в указанный день |
| Refund | 3 | 2 | Payment | `payment: std::shared_ptr<Payment>`; `reason: std::string`; `processed: bool` | `process` — однократно отменяет подтвержденный платеж<br>`amount` — возвращает сумму выполненного возврата |
| Wishlist | 3 | 2 | Course, Student | `student: std::shared_ptr<Student>`; `courses: std::vector<std::shared_ptr<Course>>`; `capacity: int` | `save` — сохраняет опубликованный курс с учетом вместимости<br>`remove` — удаляет сохраненный курс |
| Platform | 3 | 2 | Catalog, Course, Enrollment, Student | `name: std::string`; `catalog: std::shared_ptr<Catalog>`; `enrollments: std::vector<std::shared_ptr<Enrollment>>` | `enroll` — записывает студента на курс каталога без повторной активной записи<br>`activeStudents` — считает уникальных студентов с активными записями |
| VideoLesson | 0 | 1 | Video | Собственных полей нет; поля Lesson наследуются | `duration` — длительность только видео |
| ArticleLesson | 0 | 1 | Article | Собственных полей нет; поля Lesson наследуются | `duration` — время чтения статьи в секундах |
| MixedLesson | 0 | 1 | Article, Video | Собственных полей нет; один общий базовый Lesson | `duration` — сумма времени видео и чтения |

## Исключения

Все 12 исключений наследуются от `std::runtime_error`; текст доступен через `what()`.

| Исключение | Причина |
|---|---|
| `ValidationError` | Пустое значение, недопустимый диапазон, пустая обязательная связь, повтор или неверное состояние операции |
| `AccessDenied` | Неактивная учетная запись, чужое сообщение/уведомление, закрытая тема, посторонний участник |
| `CourseUnavailable` | Курс не опубликован или аккаунт студента неактивен при записи |
| `AlreadyEnrolled` | Повторная активная запись на курс |
| `NotEnrolled` | Действие требует активной записи |
| `LessonLocked` | Урок не принадлежит курсу или не содержит учебных материалов |
| `AttemptLimit` | Попытка уже содержит ответы на все вопросы |
| `DeadlineMissed` | Сдача или исправление решения после срока |
| `InvalidGrade` | Балл вне диапазона критерия |
| `PaymentFailed` | Неподтвержденный, недостаточный или повторный платеж, повторный возврат |
| `CertificateUnavailable` | Курс не завершен, запись неактивна или сертификат уже выдан |
| `ResourceNotFound` | Тема, участник группы или сохраненный курс не найден |

`make test` проверяет все 12 типов исключений, а также отсутствие изменения состояния
после отклоненных операций в основных сценариях. Проверки работают и при `NDEBUG`. Отдельные тесты проверяют конструкторы,
виртуальный вызов и единственность базового объекта при ромбовидном наследовании.
