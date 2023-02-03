#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <math.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

// TODO: implement FOV.
// impplement system to display properly.

int cmprModulIntDouble(int a, double b) {
    
}

typedef struct point_struct {
    int x;
    int y;
    int z;
}point;

typedef struct edge_struct {
    point point1, point2;
}edge;

struct def3DObj {
    int isUsed;
    char lookPoints;
    char lookEdges;

    int sizeEdge;
    edge *edges;
};

void printPointValues(point *p) {
    printf("point x: %d, y: %d, z: %d \n", p->x, p->y, p->z);
}

void printEdgeValues(edge *p) {
    printf("start of edge: ");
    printPointValues(&p->point1);
    printPointValues(&p->point2);
}

void printPointsAndEdges(struct def3DObj *ex) {
    for (int i = 0; i < ex->sizeEdge; i++) {
        printEdgeValues(&ex->edges[i]);
    }
}

void printPoints(struct def3DObj *ex) {
    for (int eachEdge = 0; eachEdge < ex->sizeEdge; eachEdge++) {
        gotoxy(ex->edges[eachEdge].point1.x, ex->edges[eachEdge].point1.y);
        printf("%c", ex->lookPoints);

        gotoxy(ex->edges[eachEdge].point2.x, ex->edges[eachEdge].point2.y);
        printf("%c", ex->lookPoints);
    }
}

void printEdge(struct def3DObj *ex, int eachEdge, int primaryAxis, int secondaryAxis, int howMuchPrimaryAxis, int howMuchSecondaryAxis, int whatIsPrimary) {
    double amtToAddFull = 1.0;
    double temp = amtToAddFull;
    double amtToAddDec;
    if (howMuchPrimaryAxis != 0 && howMuchSecondaryAxis != 0) {
        amtToAddFull = howMuchPrimaryAxis/howMuchSecondaryAxis;
    }
    amtToAddFull = fmod(amtToAddFull, amtToAddDec);
    amtToAddDec = temp-amtToAddFull;

    int counter = 1;

    while (primaryAxis < howMuchPrimaryAxis || primaryAxis > howMuchPrimaryAxis) {
        const short int Y_ISNT_ZERO = 0;

        if (whatIsPrimary == Y_ISNT_ZERO) {
            gotoxy(ex->edges[eachEdge].point1.x+secondaryAxis, ex->edges[eachEdge].point1.y+primaryAxis);
        } else {
            gotoxy(ex->edges[eachEdge].point1.x+primaryAxis, ex->edges[eachEdge].point1.y+secondaryAxis);
        }
        printf("%c", ex->lookEdges);

        if (howMuchSecondaryAxis > 0 && secondaryAxis < howMuchSecondaryAxis && counter % amtToAddFull == 0) {
            secondaryAxis++;
        } else if (howMuchSecondaryAxis < 0 && secondaryAxis > howMuchSecondaryAxis && counter % amtToAddFull == 0) {
            secondaryAxis--;
        }
        if (howMuchPrimaryAxis > 0 && primaryAxis < howMuchPrimaryAxis) {
            primaryAxis++;
        } else if (howMuchPrimaryAxis < 0 && primaryAxis > howMuchPrimaryAxis) {
            primaryAxis--;
        }
        counter += 1 + amtToAddDec;
    }
}

void printEdges(struct def3DObj *ex) {
    for (int eachEdge = 0; eachEdge < ex->sizeEdge; eachEdge++) {
        int howMuchX = ex->edges[eachEdge].point2.x - ex->edges[eachEdge].point1.x;
        int howMuchY = ex->edges[eachEdge].point2.y - ex->edges[eachEdge].point1.y;

        int x = 0;
        int y = 0;

        if (howMuchY != 0) {printEdge(ex, eachEdge, y, x, howMuchY, howMuchX, 0);}
        else if (howMuchX != 0) {printEdge(ex, eachEdge, x, y, howMuchX, howMuchY, 1);}
    }
}

void createEdges(struct def3DObj *ex, ...) {
    va_list args;
    va_start(args, ex);

    point arr[ex->sizeEdge*2];
    for (int i = 0; i < ex->sizeEdge*2; i++) {
        point point = {va_arg(args, int), va_arg(args, int), va_arg(args, int)};
        arr[i] = point;
    }
    va_end(args);

    point projectedArr[ex->sizeEdge*2];
    for (int allcArr1 = 0; allcArr1 < ex->sizeEdge*2; allcArr1++) {
        projectedArr[allcArr1].x = arr[allcArr1].x/arr[allcArr1].z;
        projectedArr[allcArr1].y = arr[allcArr1].y/arr[allcArr1].z;
        projectedArr[allcArr1].z = arr[allcArr1].z/arr[allcArr1].z;
    }

    ex->edges = malloc(sizeof(edge) * ex->sizeEdge);
    int indxEdge = 0;
    int indxPoint = 0;
    while (indxEdge < ex->sizeEdge) {
        ex->edges[indxEdge].point1 = projectedArr[indxEdge];
        ex->edges[indxEdge].point2 = projectedArr[indxEdge+1];
        indxEdge++;
        indxPoint += 2;
    }
}

int main() {
    clear();
    struct def3DObj sqr;
    sqr.lookPoints = '*';

    sqr.sizeEdge = 1;
    sqr.lookEdges = '*';

    createEdges(&sqr, 10, 10, 1, 25, 20, 1);
    printPointsAndEdges(&sqr);
    printPoints(&sqr);
    printEdges(&sqr);

    gotoxy(100, 0);

    free(sqr.edges);
    return 0;
}
