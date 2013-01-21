#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "tcxparser.h"

#define TAG(q,y,z,v) if (! strcmp( chain, #q ) ) { y->z = v; }
#define SETIF_F(x,y,z) if(! isnan(x->y)) { x->has.z = 1; }
#define SETIF_P(x,y,z) if(x->y >= 0) { x->has.z = 1; }

extern tcx_t tcxfile;
extern lap_t *laps;

char sport[1024];
int tag_ptr = 0;
char current_tag[1024][1024];
char *things[6] = { "CDATA", "Comment", "Data", "Directive", "Close", "Open" };
char chain[1024];

void
sax_cb(mxml_node_t *node, mxml_sax_event_t event, void *data)
{
    char tmp[1024];
    const char *node_name = tmp;
    const char *element = mxmlGetElement(node);

    if (element) {
        strncpy(tmp, mxmlGetElement(node), 1024);
        if (strchr(tmp, ':')) { /* BLOODY NAMESPACES */
            strsep((char **)&node_name, ":");
        }
    }

    if (event == MXML_SAX_DATA) {
        const char *x = mxmlGetText(node, NULL);
        sprintf(chain, "%s:%s", current_tag[tag_ptr-1], current_tag[tag_ptr]);

        /* Activity_t */
        TAG(Activity:Id, current_activity, date, strdup(x));

        /* ActivityLap_t */
        TAG(Lap:TotalTimeSeconds, current_lap, seconds, atof(x));
        TAG(Lap:Calories, current_lap, calories, atoi(x));
        TAG(Lap:TotalTimeSeconds, current_lap, seconds, atof(x));
        TAG(Lap:MaximumSpeed, current_lap, speed_max, atof(x));
        TAG(AverageHeartRateBpm:Value, current_lap, bpm_avg, atoi(x));
        TAG(MaximumHeartRateBpm:Value, current_lap, bpm_max, atoi(x));
        TAG(Lap:Intensity, current_lap, intensity, strdup(x));
        TAG(Lap:Cadence, current_lap, cadence, atoi(x));
        TAG(LX:AvgWatts, current_lap, watts_avg, atof(x));
        TAG(LX:MaxWatts, current_lap, watts_max, atoi(x));
        TAG(FatCalories:Value, current_lap, fat_calories, atof(x));

        if (! strcmp(current_tag[tag_ptr], "DistanceMeters")) {
            if (! strcmp(current_tag[tag_ptr-1], "Lap")) {
                current_lap->distance = atof(x);
            }
            if (! strcmp(current_tag[tag_ptr-1], "Trackpoint")) {
                current_trackpoint->distance = atof(x);
            }
        }


        /* Trackpoint_t */
        TAG(Trackpoint:Time, current_trackpoint, time, strdup(x));
        TAG(Position:LatitudeDegrees, current_trackpoint, latitude, atof(x));
        TAG(Position:LongitudeDegrees, current_trackpoint, longitude, atof(x));
        TAG(Trackpoint:Cadence, current_trackpoint, cadence, atoi(x));
        TAG(Trackpoint:Time, current_trackpoint, time, strdup(x));
        TAG(Trackpoint:AltitudeMeters, current_trackpoint, altitude, atof(x));
        TAG(HeartRateBpm:Value, current_trackpoint, bpm, atoi(x));
        TAG(TPX:Speed, current_trackpoint, speed, atof(x));
        TAG(TPX:Watts, current_trackpoint, watts, atof(x));
    }

    if (event == MXML_SAX_ELEMENT_OPEN) {
        tag_ptr++;
        strcpy(current_tag[tag_ptr], node_name);
        sprintf(chain, "%s:%s", current_tag[tag_ptr-1], current_tag[tag_ptr]);
//        printf("<%s>\n", node_name);

        if(! strcmp(chain, "Activities:MultiSportSession")) {
            tcxfile.multisport = 1;
        }
        /* Ignore our parent here */
        if(! strcmp(node_name, "Activity")) {
            const char *t = mxmlElementGetAttr(node, "Sport");
            activity_t *tmp = new_activity_t();
            add_activity(tmp);
            tmp->type = strdup(t);
        }

        if(! strcmp(node_name, "Lap")) {
            lap_t *tmp = new_lap_t();
            add_lap(tmp);
        }

        /* TODO refactor all this into the add_* functions */
        if (! strcmp(node_name, "Trackpoint")) {
            trackpoint_t *tmp = (trackpoint_t *)new_trackpoint_t();
            add_trackpoint(tmp);
        }

        if (! strcmp(node_name, "Track")) {
            track_t *tmp = (track_t *)new_track_t();
            add_track(tmp);
        }
    }

    if (event == MXML_SAX_ELEMENT_CLOSE) {
        tag_ptr--;
        sprintf(chain, "%s:%s", current_tag[tag_ptr], node_name);

        if(! strcmp(chain, "Track:Trackpoint")) {
            current_activity->last_point = current_trackpoint;
            SETIF_F(current_trackpoint, latitude, geo);
            SETIF_F(current_trackpoint, altitude, altitude);
            SETIF_F(current_trackpoint, distance, distance);
            SETIF_F(current_trackpoint, speed, speed);
            SETIF_P(current_trackpoint, watts, watts);
        }
    }
}

int main(int argc, char **argv)
{
    FILE *fp;
    mxml_node_t *tree;

    fp = fopen(argv[1], "r");
    tree = mxmlSAXLoadFile(NULL, fp, MXML_TEXT_CALLBACK, sax_cb, NULL);
    fclose(fp);

    {
        activity_t *j = tcxfile.activities;

printf("Multisport? %s\n", tcxfile.multisport ? "Yes" : "No");

        for(j=tcxfile.activities; j; j=j->next) {
            int lc = 0;
            double td = 0.0, tt = 0.0;
            int calories = 0;
            lap_t *i;
            printf("Activity %s %p\n", j->type, j->laps);
            for(i=j->laps; i; i = i->next) {
                track_t *tr;
                printf("Lap %d: %.2f metres, %d tracks, %d points, max=%.2f bpm avg=%d max=%d intensity=%s\n",
                    lc+1, i->distance, i->c_tracks, i->c_points,
                    i->speed_max, i->bpm_avg, i->bpm_max, i->intensity);
                td = td + i->distance;
                tt = tt + i->seconds;
                calories += i->calories;
                lc++;

                for(tr=i->tracks; tr; tr=tr->next) {
                    trackpoint_t *tp;
                    for(tp=tr->points; tp; tp=tp->next) {
                        if (tp->has.geo) {
                            fprintf(stderr, "GPS,%f,%f,",tp->latitude,tp->longitude);
                        } else {
                            fprintf(stderr, "GPS,BOGUS,BOGUS,");
                        }
                        fprintf(stderr, "%d,%s,%s,", lc, tp->time, j->type);
                        if (tp->has.watts) {
                            fprintf(stderr, "%d", tp->watts);
                        }
                        fprintf(stderr, "\n");
                    }
                }
            }
            printf("%.2f %.2f %s %d %s\n", td, tt, j->type, calories, j->date);
            printf("LP %p %s\n", j, j->last_point->time);
        }
    }

    return(0);
}
