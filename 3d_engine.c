#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

struct def3DObj {
    int isUsed;
    char lookPoints;
    char lookEdges;

    int sizePoint1;
    int sizePoint2;
    int **points;

    int sizeEdge;
    int *edges;
};

void printObj(int arrPoints[][2], int sizeOfArrPoints, int* arrEdges[][2][2], int sizeOfArrEdges, char lookPoints, char lookEdges) {
    for (int printPoints = 0; printPoints < sizeOfArrPoints; printPoints++) {
        gotoxy(arrPoints[printPoints][0], arrPoints[printPoints][1]);
        printf("%c", lookPoints);
    }

    for (int printEdges = 0; printEdges < sizeOfArrEdges; printEdges++) {
        // arrEdges[edge][points that connect the edge][pointers to the coordinets of the points connecting the edge]
        long int howMuchX = abs(*arrEdges[printEdges][1][0] - *arrEdges[printEdges][0][0]);
        long int howMuchY = abs(*arrEdges[printEdges][1][1] - *arrEdges[printEdges][0][1]);

        int x = 0;
        for (int y = 0; y < howMuchY; y++) {
            gotoxy(*arrEdges[printEdges][0][0]+x, *arrEdges[printEdges][0][1]+y);
            printf("%c", lookEdges);

            if (x < howMuchX) {
                x++;
            }
        }
    }
}

void printPoints(struct def3DObj *ex) {
    for (int printPoints = 0; printPoints < ex->sizePoint1; printPoints++) {
        gotoxy(ex->points[printPoints][0], ex->points[printPoints][1]);
        printf("%c", ex->lookPoints);
    }
}

void createPoints(struct def3DObj *ex, ...) {
    va_list args;
    va_start(args, ex);

    ex->points = (int**) malloc(sizeof(int*) * ex->sizePoint1);
    for (int allcArr1 = 0; allcArr1 < ex->sizePoint1; allcArr1++) {
        ex->points[allcArr1] = (int*) malloc(sizeof(int) * ex->sizePoint2);

        for (int allcArr2 = 0; allcArr2 < ex->sizePoint2; allcArr2++) {
            ex->points[allcArr1][allcArr2] = va_arg(args, int);
        }
    }

    va_end(args);
}

void createEdges(struct def3DObj *ex, ...) {
    va_list args;
    va_start(args, ex);

    ex->edges = (int*) malloc(sizeof(int) * ex->sizeEdge);
    for (int allcArr = 0; allcArr < ex->sizeEdge; allcArr++) {
        ex->edges[allcArr] = 
    }
}

int main() {
    clear();
    struct def3DObj sqr;
    sqr.sizePoint1 = 3;
    sqr.sizePoint2 = 2;
    sqr.lookPoints = '*';

    createPoints(&sqr, 10, 10, 10, 20, 20, 20);

    printPoints(&sqr);


    free(sqr.points);
    return 0;
}
