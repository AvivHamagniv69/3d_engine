#include <stdlib.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

// TODO: implement FOV.
// impplement system to display properly.

typedef struct point_struct {
    double x;
    double y;
    double z;
}point;

typedef struct edge_struct {
    point point1, point2;
}edge;

struct def3DObj {
    int isUsed;
    char lookPoints;
    char lookEdges;

    int sizePoints;
    point *points;

    int sizeEdge;
    edge *edges;
} ;

void printPointValues(point *p) {
    //printf("point x: %d, y: %d, z: %d \n", p->x, p->y, p->z);
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
        gotoxy((int) ex->edges[eachEdge].point1.x, (int) ex->edges[eachEdge].point1.y);
        printf("%c", ex->lookPoints);

        gotoxy((int) ex->edges[eachEdge].point2.x, (int) ex->edges[eachEdge].point2.y);
        printf("%c", ex->lookPoints);
    }
}

void printEdges(struct def3DObj *ex) {
    for (int eachEdge = 0; eachEdge < ex->sizeEdge; eachEdge++) {
        int howMuchX = ex->edges[eachEdge].point2.x - ex->edges[eachEdge].point1.x;
        int howMuchY = ex->edges[eachEdge].point2.y - ex->edges[eachEdge].point1.y;

        int x = 0;
        int y = 0;

        if (howMuchY != 0 && abs(howMuchY) >= abs(howMuchX)) {
                int amtToAdd = 1;

                if (howMuchY != 0 && howMuchX != 0) {
                    amtToAdd = ((double) howMuchY)/((double)howMuchX);
                }
                if (amtToAdd == 0) {
                    amtToAdd = 1;
                }

                int counter = 0;

                while (y < howMuchY || y > howMuchY) {
                    gotoxy((int) ex->edges[eachEdge].point1.x+x, (int) ex->edges[eachEdge].point1.y+y);

                    printf("%c", ex->lookEdges);

                    if (howMuchX > 0 && x < howMuchX && counter % amtToAdd == 0) {
                        x++;
                    } else if (howMuchX < 0 && x > howMuchX && counter % amtToAdd == 0) {
                        x--;
                    }
                    if (howMuchY > 0 && y < howMuchY) {
                        y++;
                    } else if (howMuchY < 0 && y > howMuchY) {
                        y--;
                    }
                    counter++;
                }
        }

        else if (howMuchX != 0 && abs(howMuchX) >= abs(howMuchY)) {
                int amtToAdd = 1;

                if (howMuchY != 0 && howMuchX != 0) {
                    amtToAdd = ((double) howMuchX)/((double)howMuchY);
                }
                if (amtToAdd == 0) {
                    amtToAdd = 1;
                }

                int counter = 0;

                while (x < howMuchX || x > howMuchX) {
                    gotoxy((int) ex->edges[eachEdge].point1.x+x, (int) ex->edges[eachEdge].point1.y+y);

                    printf("%c", ex->lookEdges);

                    if (howMuchX > 0 && x < howMuchX) {
                        x++;
                    } else if (howMuchX < 0 && x > howMuchX) {
                        x--;
                    }
                    if (howMuchY > 0 && y < howMuchY && counter % amtToAdd == 0) {
                        y++;
                    } else if (howMuchY < 0 && y > howMuchY && counter % amtToAdd == 0) {
                        y--;
                    }
                    counter++;
                }
        }
    }
}

void createEdges(struct def3DObj *ex, ...) {
    va_list args;
    va_start(args, ex);

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    point arr[ex->sizeEdge*2];
    for (int i = 0; i < ex->sizeEdge*2; i++) {
        point point = {
            va_arg(args, double),
            va_arg(args, double) * 2,
            va_arg(args, double)
        };

        if (point.z == 0) {
            point.z = 0.01;
        }
        point.x /= point.z;
        point.y /= point.z;
        point.z /= point.z;

        point.x *= w.ws_col;
        point.y *= w.ws_row;

        point.x += (w.ws_col/2);
        point.y += (w.ws_row/2);

        arr[i] = point;
    }
    va_end(args);

    ex->edges = malloc(sizeof(edge) * ex->sizeEdge);
    int indxEdge = 0;
    int indxPoint = 0;
    while (indxEdge < ex->sizeEdge) {
        ex->edges[indxEdge].point1 = arr[indxPoint];
        ex->edges[indxEdge].point2 = arr[indxPoint+1];
        indxEdge++;
        indxPoint += 2;
    }
}

void addToPoints(struct def3DObj *ex, ...) {
    va_list args;
    va_start(args, ex);

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    point arr[ex->sizeEdge*2];
    for (int i = 0; i < ex->sizeEdge*2; i++) {
        point point = {
            va_arg(args, double),
            va_arg(args, double),
            va_arg(args, double) * 2
        };

        point.x /= point.z;
        point.y /= point.z;
        point.z /= point.z;

        point.x *= w.ws_col;
        point.y *= w.ws_row;

        point.x += (w.ws_col/2);
        point.y += (w.ws_row/2);

        arr[i] = point;
    }
    va_end(args);

    int indxEdge = 0;
    int indxPoint = 0;
    while (indxEdge < ex->sizeEdge) {
        ex->edges[indxEdge].point1.x += arr[indxPoint].x;
        ex->edges[indxEdge].point1.y += arr[indxPoint].y;
        ex->edges[indxEdge].point1.z += arr[indxPoint].z;

        ex->edges[indxEdge].point2.x += arr[indxPoint+1].x;
        ex->edges[indxEdge].point2.y += arr[indxPoint+1].y;
        ex->edges[indxEdge].point2.z += arr[indxPoint+1].z;

        indxEdge++;
        indxPoint += 2;
    }
}

int main() {
    clear();
    struct def3DObj sqr;
    sqr.lookPoints = '*';

    sqr.sizeEdge = 12;
    sqr.lookEdges = '*';

    createEdges(&sqr,
        0.1, 0.1, 1.0,
        0.1, 0.1, 1.5,

        0.1, 0.2, 1.0,
        0.1, 0.2, 1.5,

        0.2, 0.1, 1.0,
        0.2, 0.1, 1.5,

        0.2, 0.2, 1.0,
        0.2, 0.2, 1.5,

        0.1, 0.1, 1.0,
        0.2, 0.1, 1.0,

        0.1, 0.1, 1.0,
        0.1, 0.2, 1.0,

        0.2, 0.2, 1.0,
        0.2, 0.1, 1.0,

        0.2, 0.2, 1.0,
        0.1, 0.2, 1.0,

        0.1, 0.1, 1.5,
        0.2, 0.1, 1.5,

        0.1, 0.1, 1.5,
        0.1, 0.2, 1.5,

        0.2, 0.2, 1.5,
        0.2, 0.1, 1.5,

        0.2, 0.2, 1.5,
        0.1, 0.2, 1.5
    );

    printPoints(&sqr);
    printEdges(&sqr);
    sleep(1);

    gotoxy(0, 0);

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    printf("%d \n", w.ws_col);
    printf("%d \n", w.ws_row);
    free(sqr.edges);
    return 0;
}
