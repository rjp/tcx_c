#include <mxml.h>
#include <stdio.h>

void find_laps(mxml_node_t *tree)
{
    mxml_node_t *actone, *lap;
    int lap_count = 0;

    actone = mxmlFindElement(tree, tree, "Activity", NULL, NULL, MXML_DESCEND);

    lap = actone;
    for(lap = mxmlFindElement(actone, tree, "Lap", NULL, NULL, MXML_DESCEND_FIRST);
        lap != NULL;
        lap = mxmlFindElement(lap, tree, "Lap", NULL, NULL, MXML_NO_DESCEND)
    )  {
        const char *lst;
        lap_count++;
        lst = mxmlElementGetAttr(lap, "StartTime");
        printf("Lap %d starts at %s\n", lap_count, lst);
    }
}

int main(void)
{
    FILE *fp;
    mxml_node_t *tree;

    fp = fopen("/Users/rjp/dunwich.tcx", "r");
    tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    fclose(fp);

    find_laps(tree);

    return(0);
}
