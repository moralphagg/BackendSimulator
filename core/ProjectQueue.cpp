#include "ProjectQueue.h"
#include <QRandomGenerator>

static const QList<Project> ALL_PROJECTS = {
    {"Простой CRUD API",                   "Web",             1,         200,    2},
    {"Конвертер валют API",                "FinTech",         1,         220,    2},
    {"Todo REST сервис",                   "Web",             1,         250,    2},
    {"API заметок",                        "Web",             1,         280,    2},

    {"База данных пользователей",          "Database",        2,         400,    3},
    {"Сервис авторизации",                 "Security",        2,         450,    3},
    {"Система комментариев",               "Social",          2,         500,    4},
    {"Форумный backend",                   "Social",          2,         550,    4},

    {"Микросервис аутентификации",         "Security",        3,         800,    5},
    {"Почтовый сервис",                    "Communication",   3,         850,    5},
    {"Сервис уведомлений",                 "Communication",   3,         900,    5},
    {"Система логирования",                "DevOps",          3,        1000,    6},

    {"Система кэширования Redis",          "Infrastructure",  4,        1500,    8},
    {"IoT платформа",                      "Cloud",           4,        1800,   10},
    {"Real-time чат сервер",               "Communication",   4,        2200,   12},
    {"CI/CD пайплайн",                     "DevOps",          4,        2500,   12},

    {"ML рекомендательная система",        "AI",              5,        3000,   14},
    {"Blockchain API",                     "Blockchain",      5,        3500,   15},
    {"Kubernetes кластер",                 "DevOps",          5,        4000,   18},

    {"GraphQL API Gateway",                "Infrastructure",  6,        4500,   20},
    {"Serverless архитектура",             "Cloud",           6,        5000,   22},

    {"Система потоковой обработки Kafka",  "Infrastructure",  7,        6000,   24},
    {"Elasticsearch поисковый кластер",    "Infrastructure",  7,        6500,   25},

    {"Микросервисная архитектура",         "Enterprise",      8,        8000,   30},
    {"AI платформа",                       "AI",              8,        8500,   32},

    {"Backend маркетплейса",               "E-Commerce",      9,       10000,   35},
    {"Система компьютерного зрения",       "AI",              9,       11000,   36},

    {"Глобальная платформа доставки",      "E-Commerce",     10,       15000,   40},
    {"Архитектура реального времени",      "Infrastructure", 10,       16000,   42},

    {"Биометрическая система",             "Security",       11,       20000,   45},

    {"Квантовый computing API",            "FutureTech",     12,       25000,   50},

    {"ИИ супер-ассистент",                 "AI",             13,       30000,   55},

    {"Космическая база данных",            "SpaceTech",      14,       35000,   58},

    {"Межгалактическая сеть",              "SpaceTech",      15,       40000,   60},

    {"Виртуальная вселенная",              "FutureTech",     16,       50000,   65},

    {"Нейро-интерфейс API",                "FutureTech",     17,       60000,   70},

    {"Квантовая криптография",             "Security",       18,       75000,   75},

    {"ИИ правительство",                   "AI",             20,      100000,   80},

    {"Матрица полного погружения",         "FutureTech",     25,      200000,   90},

    {"а что эта backend",                  "SecretCap",     322,  -999999999,  429},

};

Project ProjectQueue::generateProject(int level)
{
    int minDiff = 1;

    int maxDiff =
        qMin(
            5,
            level + 2
            );

    QList<Project> available;

    for (const Project &p : ALL_PROJECTS)
    {
        if (
            p.difficulty >= minDiff &&
            p.difficulty <= maxDiff
            )
            available.append(p);
    }

    if (available.isEmpty())
        available = ALL_PROJECTS.mid(0, 10);

    Project p =
        available[
            QRandomGenerator::global()->bounded(
                available.size()
                )
    ];

    p.progress = 0;

    p.bugs =
        QRandomGenerator::global()->bounded(
            p.difficulty,
            p.difficulty * 3 + 1
            );

    int baseDays = 10 + p.difficulty * 3;
    int variance = QRandomGenerator::global()->bounded(-2, 4);
    p.deadlineDay = qMax(7, baseDays + variance);

    return p;
}
void ProjectQueue::refill(GameState &state) {
    while (state.projectQueue.size() < 3)
        state.projectQueue.enqueue(generateProject(state.level));
}