#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>

int lap_count = 0;
double lap_distances[100];

double lap_distance(mxml_node_t *lap)
{
    mxml_node_t *distance;

    distance = mxmlFindElement(lap, lap, "DistanceMeters", NULL, NULL, MXML_DESCEND_FIRST);
    if (distance) {
        const char *s_dist;
        s_dist = mxmlGetText(distance, NULL);
        return atof(s_dist);
    }

    return (double)0.0;
}

void find_laps(mxml_node_t *tree)
{
    mxml_node_t *actone, *lap;
    int lap_count = 0;
    double total_dist = 0;

    actone = mxmlFindElement(tree, tree, "Activity", NULL, NULL, MXML_DESCEND);

    lap = actone;
    for(lap = mxmlFindElement(actone, tree, "Lap", NULL, NULL, MXML_DESCEND_FIRST);
        lap != NULL;
        lap = mxmlFindElement(lap, tree, "Lap", NULL, NULL, MXML_NO_DESCEND)
    )  {
        float l_dist;
        const char *lst;
        lap_count++;
        lst = mxmlElementGetAttr(lap, "StartTime");
        l_dist = lap_distance(lap);
        printf("Lap %d starts at %s, %.2f metres\n", lap_count, lst, l_dist);
        total_dist = total_dist + l_dist;
    }
    printf("Total dist: %.2f metres\n", total_dist);
}

int tag_ptr = -1;
char current_tag[1024][1024];
char *things[6] = { "CDATA", "Comment", "Data", "Directive", "Close", "Open" };

void
sax_cb(mxml_node_t *node, mxml_sax_event_t event, void *data)
{
    const char *node_name = mxmlGetElement(node);
    printf("| %-9s %p %p\n", things[event], node, data);

    if (event == MXML_SAX_DATA) {
        if (! strcmp(current_tag[tag_ptr], "DistanceMeters")) {
            const char *x = mxmlGetText(node, NULL);
            if (x) {
                printf("D %p %p %s\n", node, x, x);
            } else {
                printf("D %p %p\n", node, x);
            }

            /*
            double s_dist;
            s_dist = atof((char *)data);
            printf("L %.2f meters\n", s_dist);
            */
        }
    }

    if (event == MXML_SAX_ELEMENT_OPEN) {
        const char *x = mxmlGetText(node, NULL);
        const char *y = mxmlGetOpaque(node);
        printf("+ %s %p %p\n", node_name, x, y);
        tag_ptr++;
        strcpy(current_tag[tag_ptr], node_name);

        if (! strcmp(node_name, "DistanceMeters")) {
            printf("R %s\n", node_name);
            mxmlRetain(node);
        }
    }

    if (event == MXML_SAX_ELEMENT_CLOSE) {
        const char *x = mxmlGetText(node, NULL);
        const char *y = mxmlGetOpaque(node);
        printf("- %s %p %p\n", node_name, x, y);

        if (! strcmp(node_name, "DistanceMeters")) {
            printf("R %s\n", node_name);
            mxmlRetain(node);
        }

        tag_ptr--;
    }
}

int main(int argc, char **argv)
{
    FILE *fp;
    mxml_node_t *tree;

    /* set all the lap distances to a known invalid number */
    { int i; for(i=0; i<100; i++) { lap_distances[i] = -1.0; } }

    fp = fopen(argv[1], "r");
    tree = mxmlSAXLoadFile(NULL, fp, MXML_TEXT_CALLBACK, sax_cb, NULL);
    fclose(fp);

    return(0);
}
