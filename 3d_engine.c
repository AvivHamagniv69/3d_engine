#include <stdlib.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

// TODO:
// implement a system to find origin point.
// implement a system to rotate a cube on the y axis.
// impplement system to display properly.
// implement FOV.

typedef struct point_struct {
    double x;
    double y;
    double z;
} point;

typedef struct edge_struct {
    point point1, point2;
} edge;

typedef struct triangle_by_edges_struct {
    edge edge1, edge2, edge3;
} triangleByEdges;

typedef struct triangle_by_points_struct {
    point point1, point2, point3;
} triangleByPoints;

typedef struct def3DObj {
    int isUsed;
    char lookPoints;
    char lookEdges;
    point originPoint;

    int sizePoints;
    point* points;

    int sizeEdge;
    edge* edges;

    int sizeTrianglesByEdges;
    triangleByEdges* trianglesByEdges;

    int sizeTrianglesByPoints;
    triangleByPoints* trianglesByPoints;
} obj3d;

void printPointValues(point *p) {
    //printf("point x: %d, y: %d, z: %d \n", p->x, p->y, p->z);
}

void printEdgeValues(edge *p) {
    printf("start of edge: ");
    printPointValues(&p->point1);
    printPointValues(&p->point2);
}

void printPointsAndEdges(obj3d *ex) {
    for (int i = 0; i < ex->sizeEdge; i++) {
        printEdgeValues(&ex->edges[i]);
    }
}

void printPoints(obj3d *ex) {
    for (int eachEdge = 0; eachEdge < ex->sizeEdge; eachEdge++) {
        gotoxy((int) ex->edges[eachEdge].point1.x, (int) ex->edges[eachEdge].point1.y);
        printf("%c", ex->lookPoints);

        gotoxy((int) ex->edges[eachEdge].point2.x, (int) ex->edges[eachEdge].point2.y);
        printf("%c", ex->lookPoints);
    }
    fflush(stdout);
}

void printEdges(obj3d *ex) {
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
    fflush(stdout);
}

void projectArr(obj3d *ex, point arr[ex->sizeEdge*2]) {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    for (int i = 0; i < ex->sizeEdge*2; i++) {
        if (arr[i].z == 0) {
            arr[i].z = 0.01;
        }
        arr[i].x /= arr[i].z;
        arr[i].y /= arr[i].z;

        arr[i].x *= w.ws_col;
        arr[i].y *= w.ws_row;

        arr[i].x += (w.ws_col/2);
        arr[i].y += (w.ws_row/2);
    }
}

void createEdges(obj3d *ex, ...) {
    va_list args;
    va_start(args, ex);

    point arr[ex->sizeEdge*2];
    for (int i = 0; i < ex->sizeEdge*2; i++) {
        point point = {
            va_arg(args, double),
            va_arg(args, double) * 2,
            va_arg(args, double)
        };
        arr[i] = point;
    }
    va_end(args);

    projectArr(ex, arr);
    ex->edges = malloc(sizeof(edge) * ex->sizeEdge);
    if (ex->edges == NULL) {
        printf("couldnt allocate memory");
        exit(1);
    }
    int indxEdge = 0;
    int indxPoint = 0;
    while (indxEdge < ex->sizeEdge) {
        ex->edges[indxEdge].point1 = arr[indxPoint];
        ex->edges[indxEdge].point2 = arr[indxPoint+1];
        indxEdge++;
        indxPoint += 2;
    }
}

point findOriginPointByGeometry(point arr[]) {
    point sumPoint = {0, 0, 0};
    for (int i = 0; i < sizeof(*arr); i++) {
        sumPoint.x += arr[i].x;
        sumPoint.y += arr[i].y;
        sumPoint.z += arr[i].z;
    }
    point originPoint = {sumPoint.x/sizeof(*arr), sumPoint.y/sizeof(*arr), sumPoint.z/sizeof(*arr)};
    return originPoint;
}

/*
point findCrossProduct(point a, point b) {
    point crossProduct = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b .y - a.y * b.x
    };
    return crossProduct;
}

double calcSurfaceAreaOfTriangle(point a, point b, point c) {
    point ab = {
        b.x - a.x,
        b.y - a.y,
        b.z - a.z
    };

    point ac = {
        c.x - a.x,
        c.y - a.y,
        c.z - a.z
    };

    point crossProduct = findCrossProduct(ab, ac);
    double surfaceArea = sqrt(pow(crossProduct.x, 2) + pow(crossProduct.y, 2) + pow(crossProduct.z, 2)) / 2;
    return surfaceArea;
}

void findLongestDistanceBetweenPointsInObj(point arr[], point* min, point* max) {
    *min = arr[0];
    *max = arr[0];
    double distance = sqrt(abs(pow(max->x - min->x, 2) + pow(max->y - min->y, 2) + pow(max->z - min->z, 2)));
    double distanceTemp;
    point minTemp;
    point maxTemp;

    for (int iterMin = 0; iterMin < sizeof(*arr); iterMin++) {
        minTemp = arr[iterMin];
        for (int iterMax = 0; iterMax < sizeof(*arr); iterMax++) {
            maxTemp = arr[iterMax];
            distanceTemp = sqrt(abs(pow(max->x - min->x, 2) + pow(max->y - min->y, 2) + pow(max->z - min->z, 2)));

            if (distanceTemp > distance) {
                distance = distanceTemp;
                *min = minTemp;
                *max = maxTemp;
            }
        }
    }
}

void findConvexHullOfUndefinedObj(point arr[], point arrToReturn[]) {
    if (sizeof(*arr) < 3) {
        return;
    }

    point* minPointInObj;
    point* maxPointInObj;
    findLongestDistanceBetweenPointsInObj(arr, minPointInObj, maxPointInObj);

}*/

int main() {
    clear();
    obj3d sqr;
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

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    free(sqr.edges);
    return 0;
}
