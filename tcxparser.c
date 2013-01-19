#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "tcxparser.h"

struct tcx_s tcxfile; /* NOT A POINTER */
activity_t *current_activity = NULL;
lap_t *current_lap = NULL, *laps = NULL;
track_t *current_track = NULL;
trackpoint_t *current_trackpoint = NULL;

/* error-checked malloc */
void *safe_malloc(size_t size)
{
    void *tmp;
    tmp = (void *)malloc(size);
    if (!tmp) {
        exit(99);
    }
    memset(tmp, 0, size);
    return tmp;
}

/* constructors */
trackpoint_t *new_trackpoint_t(void)
{
    trackpoint_t *tmp = (trackpoint_t *)safe_malloc(sizeof(trackpoint_t));
    return tmp;
}

track_t *new_track_t(void)
{
    track_t *tmp = (track_t *)safe_malloc(sizeof(track_t));
    return tmp;
}

lap_t *new_lap_t(void)
{
    lap_t *tmp = (lap_t *)safe_malloc(sizeof(lap_t));
    return tmp;
}

activity_t *new_activity_t(void)
{
    activity_t *tmp = (activity_t *)safe_malloc(sizeof(activity_t));
    return tmp;
}

tcx_t *new_tcx_t(void)
{
    tcx_t *tmp = (tcx_t *)safe_malloc(sizeof(tcx_t));
    return tmp;
}

/* makers */
void add_lap(lap_t *lap)
{
    if (current_lap != NULL) {
        current_lap->next = lap;
    } else {
        current_activity->laps = lap;
    }
    current_lap = lap;
    current_track = NULL;
    current_trackpoint = NULL;
}

void add_trackpoint(trackpoint_t *tmp)
{
    current_lap->c_points++;
    current_track->c_points++;
    if (current_trackpoint != NULL) {
        current_trackpoint->next = tmp;
    } else {
        current_track->points = tmp;
    }
    current_trackpoint = tmp;
}

void add_track(track_t *tmp)
{
    if (current_track != NULL) {
        current_track->next = tmp;
    } else {
        current_lap->tracks = tmp;
    }
    current_track = tmp;
    current_trackpoint = NULL;
    current_lap->c_tracks++;
}

void add_activity(activity_t *tmp)
{
    if (current_activity != NULL) {
        current_activity->next = tmp;
    } else {
        tcxfile.activities = tmp;
    }
    current_activity = tmp;
    current_lap = NULL;
    current_track = NULL;
    current_trackpoint = NULL;
}

