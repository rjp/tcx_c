struct trackpoint_s
{
    char *time;
    int time_epoch;
    float latitude;
    float longitude;
    float altitude;
    float distance;
    int bpm;
    int cadence;
    float speed;
    int watts;
    struct trackpoint_s *next;
};
typedef struct trackpoint_s trackpoint_t;

struct track_s
{
    int c_points;
    trackpoint_t *points;
    struct track_s *next;
};
typedef struct track_s track_t;

struct lap_s
{
    char *start_time;
    int start_epoch;
    float distance;
    float seconds;
    float speed_max;
    int calories;
    int bpm_avg;
    int bpm_max;
    int cadence;
    char *intensity;
    char *trigger;
    int c_points;
    int c_tracks;
    int watts_max;
    float watts_avg;
    track_t *tracks;
    struct lap_s *next;
};
typedef struct lap_s lap_t;

struct activity_s {
    char *type;
    char *date;
    int epoch_date;
    lap_t *laps;
    char *creator;
    char *unitid;
    struct activity_s *next;
    struct trackpoint_s *last_point;
};
typedef struct activity_s activity_t;

struct tcx_s {
    activity_t *activities;
    int multisport;
    char *filename;
};
typedef struct tcx_s tcx_t;

void add_activity(activity_t *);
void add_lap(lap_t *);
void add_track(track_t *);
void add_trackpoint(trackpoint_t *);
activity_t *new_activity_t(void);
lap_t *new_lap_t(void);
track_t *new_track_t(void);
trackpoint_t *new_trackpoint_t(void);

extern struct tcx_s tcxfile;
extern activity_t *current_activity;
extern lap_t *laps;
extern lap_t *current_lap;
extern track_t *current_track;
extern trackpoint_t *current_trackpoint;
