#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char **argv)
{
    FILE *fp;
    mxml_node_t *tree;

    fp = fopen(argv[1], "r");
    tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    fclose(fp);

    find_laps(tree);

    return(0);
}
