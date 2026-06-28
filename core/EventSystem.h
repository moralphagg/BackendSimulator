#pragma once
#include <QString>
#include <QList>

enum class EventType {
    MorningCoffee,
    AfternoonSlump,
    LateNightCoding,

    ClientCall,
    BugReport,
    ServerDown,
    Distraction,

    HungryAlert,
    InternetBill,
    LaundryReminder,

    LectureStart,
    LabDeadline,
    ExamWarning
};

struct GameEvent {
    EventType   type;
    int         triggerDay;
    int         triggerHour;
    bool        triggered = false;
    QString     description;
    bool        requiresResponse = false;
};