#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>

int lap_count = 0;
double lap_distances[100];
char sport[1024];
int tag_ptr = -1;
char current_tag[1024][1024];
char *things[6] = { "CDATA", "Comment", "Data", "Directive", "Close", "Open" };

void
sax_cb(mxml_node_t *node, mxml_sax_event_t event, void *data)
{
    const char *node_name = mxmlGetElement(node);

    if (event == MXML_SAX_DATA) {
        if (! strcmp(current_tag[tag_ptr], "DistanceMeters")) {
            if (! strcmp(current_tag[tag_ptr-1], "Lap")) {
                const char *x = mxmlGetText(node, NULL);
                lap_distances[lap_count] = atof(x);
                lap_count++;
            }
        }
    }

    if (event == MXML_SAX_ELEMENT_OPEN) {
        tag_ptr++;
        strcpy(current_tag[tag_ptr], node_name);

        if(! strcmp(node_name, "Activity")) {
            const char *t = mxmlElementGetAttr(node, "Sport");
            strcpy(sport, t);
        }
    }

    if (event == MXML_SAX_ELEMENT_CLOSE) {
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

    {
        int i;
        double t = 0.0;
        for(i=0; lap_distances[i] > 0; i++) {
            printf("Lap %d: %.2f metres\n", i+1, lap_distances[i]);
            t = t + lap_distances[i];
        }
        printf("%.2f %s\n", t, sport);
    }

    return(0);
}
