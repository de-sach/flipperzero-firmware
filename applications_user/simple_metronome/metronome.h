#pragma once

typedef enum _NotificationType {
    NOTIF_SOUND = 0u,
    NOTIF_BUZZER = 1u,
    NOTIF_LED = 2u,
    NOTIF_MAX,
} NotificationType;

typedef enum _RythmType {
    RYTHM_NONE,
    RYTHM_2_4,
    RYTHM_3_4,
    RYTHM_4_4,
    RYTHM_6_8,
    RYTHM_MAX,
} RythmType;