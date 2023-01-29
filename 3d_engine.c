#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

struct def3DObj {
    int isUsed;
    char lookPoints;
    char lookEdges;

    int sizePoint1;
    int **points;

    int sizeEdge;
    int ***edges;
};

void printArrsForDebugging(struct def3DObj *ex) {
    for (int i = 0; i < ex->sizePoint1; i++) {
        printf("we are at array %d: \n", i);
        printf("point x is %d: \n", ex->points[i][0]);
        printf("point y is %d: \n", ex->points[i][1]);
    }

    for (int i = 0; i < ex->sizeEdge; i++) {
        printf("we are at edge %d: \n", i);
        for (int j = 0; j < 2; j++) {
            printf("this is point: %d \n", ex->edges[i][j][0]);
            printf("this is point: %d \n", ex->edges[i][j][1]);
        }
    }
}

void printPoints(struct def3DObj *ex) {
    for (int printPoints = 0; printPoints < ex->sizePoint1; printPoints++) {
        gotoxy(ex->points[printPoints][0], ex->points[printPoints][1]);
        printf("%c", ex->lookPoints);
    }
}

void printEdge(struct def3DObj *ex, int eachEdge, int primaryAxis, int secondaryAxis, int howMuchPrimaryAxis, int howMuchSecondaryAxis, int whatIsPrimary) {
    while (primaryAxis < howMuchPrimaryAxis || primaryAxis > howMuchPrimaryAxis) {
        if (whatIsPrimary == 0) {
            gotoxy(ex->edges[eachEdge][0][0]+secondaryAxis, ex->edges[eachEdge][0][1]+primaryAxis);
        } else {
            gotoxy(ex->edges[eachEdge][0][0]+primaryAxis, ex->edges[eachEdge][0][1]+secondaryAxis);
        }

        printf("%c", ex->lookEdges);

        if (howMuchSecondaryAxis > 0 && secondaryAxis < howMuchSecondaryAxis) {
            secondaryAxis++;
        } else if (howMuchSecondaryAxis < 0 && secondaryAxis > howMuchSecondaryAxis) {
            secondaryAxis--;
        }
        if (howMuchPrimaryAxis > 0 && primaryAxis < howMuchPrimaryAxis) {
            primaryAxis++;
        } else if (howMuchPrimaryAxis < 0 && primaryAxis > howMuchPrimaryAxis) {
            primaryAxis--;
        }
    }
}

void printEdges(struct def3DObj *ex) {
    for (int eachEdge = 0; eachEdge < ex->sizeEdge; eachEdge++) {
        int howMuchX = ex->edges[eachEdge][1][0] - ex->edges[eachEdge][0][0];
        int howMuchY = ex->edges[eachEdge][1][1] - ex->edges[eachEdge][0][1];

        int x = 0;
        int y = 0;

        if (howMuchY != 0) {printEdge(ex, eachEdge, y, x, howMuchY, howMuchX, 0);}
        else if (howMuchX != 0) {printEdge(ex, eachEdge, x, y, howMuchX, howMuchY, 1);}
    }
}

void createPoints(struct def3DObj *ex, ...) {
    va_list args;
    va_start(args, ex);

    ex->points = (int**) malloc(sizeof(int*) * ex->sizePoint1);
    for (int allcArr1 = 0; allcArr1 < ex->sizePoint1; allcArr1++) {
        ex->points[allcArr1] = (int*) malloc(sizeof(int) * 2);

        for (int allcArr2 = 0; allcArr2 < 2; allcArr2++) {
            ex->points[allcArr1][allcArr2] = va_arg(args, int);
        }
    }

    va_end(args);
}

void createEdges(struct def3DObj *ex, ...) {
    va_list args;
    va_start(args, ex);

    ex->edges = (int***) malloc(sizeof(int**) * ex->sizeEdge);
    for (int allEdges = 0; allEdges < ex->sizeEdge; allEdges++) {
        ex->edges[allEdges] = (int**) malloc(sizeof(int*) * 2);

        for (int pointers = 0; pointers < 2; pointers++) {
            int* pointer = ex->points[va_arg(args, int)];
            ex->edges[allEdges][pointers] = pointer;
        }
    }

    va_end(args);
}

void freePoints(struct def3DObj *ex) {
    for (int freeP = 0; freeP < ex->sizePoint1; freeP++) {
        free(ex->points[freeP]);
    }
    free(ex->points);
}

void freeEdges(struct def3DObj *ex) {
    for (int freeFirst = 0; freeFirst < ex->sizeEdge; freeFirst++) {
        for (int freeSecond = 0; freeSecond < 2; freeSecond++) {

        }
    }
}

int main() {
    clear();
    struct def3DObj sqr;
    sqr.sizePoint1 = 3;
    sqr.lookPoints = '*';

    sqr.sizeEdge = 3;
    sqr.lookEdges = '*';

    createPoints(&sqr, 10, 10, 10, 20, 20, 20);
    createEdges(&sqr, 0, 1, 1, 2, 2, 0);

    //printArrsForDebugging(&sqr);
    printPoints(&sqr);
    printEdges(&sqr);

    gotoxy(0, 0);

    freePoints(&sqr);
    free(sqr.edges);
    return 0;
}
