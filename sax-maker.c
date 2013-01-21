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
    const char *node_name = mxmlGetElement(node);

    if (event == MXML_SAX_DATA) {
        const char *x = mxmlGetText(node, NULL);
        sprintf(chain, "%s:%s", current_tag[tag_ptr-1], current_tag[tag_ptr]);

        /* Activity_t */
        if(! strcmp(chain, "Activity:Id")) {
            current_activity->date = strdup(x);
        }

        if (! strcmp(current_tag[tag_ptr], "DistanceMeters")) {
            if (! strcmp(current_tag[tag_ptr-1], "Lap")) {
                current_lap->distance = atof(x);
            }
            if (! strcmp(current_tag[tag_ptr-1], "Trackpoint")) {
                current_trackpoint->distance = atof(x);
            }
        }

        /* ActivityLap_t */
        if(! strcmp(chain, "Lap:TotalTimeSeconds")) {
            current_lap->seconds = atof(x);
        }
        if(! strcmp(chain, "Lap:Calories")) {
            current_lap->calories = atoi(x);
        }
        if(! strcmp(chain, "Lap:TotalTimeSeconds")) {
            current_lap->seconds = atof(x);
        }
        if(! strcmp(chain, "Lap:MaximumSpeed")) {
            current_lap->speed_max = atof(x);
        }
        if(! strcmp(chain, "AverageHeartRateBpm:Value")) {
            current_lap->bpm_avg = atoi(x);
        }
        if(! strcmp(chain, "MaximumHeartRateBpm:Value")) {
            current_lap->bpm_max = atoi(x);
        }
        if(! strcmp(chain, "Lap:Intensity")) {
            current_lap->intensity = strdup(x);
        }
        if(! strcmp(chain, "Lap:Cadence")) {
            current_lap->cadence = atoi(x);
        }
        if (! strcmp(chain, "Trackpoint:Time")) {
            current_trackpoint->time = strdup(x);
        }
    }

    if (event == MXML_SAX_ELEMENT_OPEN) {
        tag_ptr++;
        strcpy(current_tag[tag_ptr], node_name);
        sprintf(chain, "%s:%s", current_tag[tag_ptr-1], current_tag[tag_ptr]);

        if(! strcmp(chain, "Activities:MultiSportSession")) {
            tcxfile.multisport = 1;
        }
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
                printf("Lap %d: %.2f metres, %d tracks, %d points, max=%.2f bpm avg=%d max=%d intensity=%s\n",
                    lc+1, i->distance, i->c_tracks, i->c_points,
                    i->speed_max, i->bpm_avg, i->bpm_max, i->intensity);
                td = td + i->distance;
                tt = tt + i->seconds;
                calories += i->calories;
                lc++;
            }
            printf("%.2f %.2f %s %d %s\n", td, tt, j->type, calories, j->date);
            printf("LP %p %s\n", j, j->last_point->time);
        }
    }

    return(0);
}
