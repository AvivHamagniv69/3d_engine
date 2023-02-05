#include <stdlib.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <math.h>
#include <stdio.h>

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

void printEdge(struct def3DObj *ex, int eachEdge, int primaryAxis, int secondaryAxis, int howMuchPrimaryAxis, int howMuchSecondaryAxis, int whatIsPrimary) {
    double amtToAddTemp = 1;
    if (howMuchPrimaryAxis != 0 && howMuchSecondaryAxis != 0) {
        amtToAddTemp = ((double) howMuchPrimaryAxis)/((double)howMuchSecondaryAxis) * 10;
    }

    int amtToAdd = amtToAddTemp;
    if ((int) amtToAddTemp == 0) {
        amtToAdd = 10;
    }

    int counter = 10;

    while (primaryAxis < howMuchPrimaryAxis || primaryAxis > howMuchPrimaryAxis) {
        const short int Y_ISNT_ZERO = 0;

        if (whatIsPrimary == Y_ISNT_ZERO) {
            gotoxy((int) ex->edges[eachEdge].point1.x+secondaryAxis, (int) ex->edges[eachEdge].point1.y+primaryAxis);
        } else {
            gotoxy((int) ex->edges[eachEdge].point1.x+primaryAxis, (int) ex->edges[eachEdge].point1.y+secondaryAxis);
        }
        printf("%c", ex->lookEdges);

        if (howMuchSecondaryAxis > 0 && secondaryAxis < howMuchSecondaryAxis && counter % amtToAdd == 0) {
            secondaryAxis++;
        } else if (howMuchSecondaryAxis < 0 && secondaryAxis > howMuchSecondaryAxis && counter % amtToAdd == 0) {
            secondaryAxis--;
        }
        if (howMuchPrimaryAxis >= 0 && primaryAxis <= howMuchPrimaryAxis) {
            primaryAxis++;
        } else if (howMuchPrimaryAxis <= 0 && primaryAxis >= howMuchPrimaryAxis) {
            primaryAxis--;
        }
        counter+=10;
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
    gotoxy(0, 0);
    free(sqr.edges);
    return 0;
}
