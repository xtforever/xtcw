#ifndef ALARM_LOG_H
#define ALARM_LOG_H

typedef struct alarm_log_entry {
    char *text; /* static allocated - do not free */
    int id,
        minute_of_day,
        pri;
} alarm_log_t;

#endif
