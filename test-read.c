#include <mxml.h>
#include <stdio.h>

int main(void)
{
    FILE *fp;
    mxml_node_t *tree;

    fp = fopen("/Users/rjp/dunwich.tcx", "r");
    tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    fclose(fp);
}
