#include <stdlib.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

// \x1B[2J]
#define PI 3.141592653589793
#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

// TODO:
/*
 * static inline
void drawCircle(const uint x, const uint y, const uint radius)
{
    const double rsq = (double)(radius*radius);
    uint yoff = radius;
    for(uint xoff = 0; xoff <= yoff; xoff++){
        const double yc = sqrt(rsq - (xoff+1)*(xoff+1));
        const double ym = (double)yoff - 0.5;
        // 8 sections of circle
        drawPixel(x+xoff, y+yoff);        // 1
        drawPixel(x-yoff, y+xoff);        // 2
        drawPixel(x-xoff, y-yoff);        // 3
        drawPixel(x+yoff, y-xoff);        // 4
        drawPixel(x-xoff, y+yoff);        // 5
        drawPixel(x-yoff, y-xoff);        // 6
        drawPixel(x+xoff, y-yoff);        // 7
        drawPixel(x+yoff, y+xoff);        // 8
        yoff -= yc <= ym;
    }
}
*/
/*
 * https://discord.com/channels/331718482485837825/331718539738087426/1079705665691451422
 * fix rendering issue with edges
 * fix problems with origin point
 * implement colors
 * make resizble arrays
 * make functions to resize those arrays
 * make linked lists to store 3d objects
 * make an array to store all cameras
 * make function to print all 3d objects
 * respect aspect ratio
 * fix memory leaks
 */

struct winsize w;
// WINDOW *create_newwin(int height, int width, int starty, int startx);
// void destroy_win(WINDOW *local_win);
static int activeCamera = 0;

typedef struct {
    double x;
    double y;
    double z;
} point;

typedef struct {
    point* arr;
    int length;
    int capacity;
} dynamicArrP;

typedef struct {
    point* point1;
    point* point2;
} edge;

typedef struct {
    edge* arr;
    int length;
    int capacity;
} dynamicArrE;

typedef struct {
    edge* edge1;
    edge* edge2;
    edge* edge3;
} triangle;

typedef struct {
    triangle* arr;
    int length;
    int capacity;
} dynamicArrT;

typedef struct {
    short int isUsed;
    char lookPoints;
    char lookEdges;
    char lookTriangles;
    point originPoint;

    dynamicArrP points;
    dynamicArrE edges;
    dynamicArrT triangles;
} obj3d;

typedef struct {
    point originPoint;
    unsigned short int frameCap;
    unsigned short int right; // right has to be bigger then left;
    unsigned short int bottom; // bottom has to be bigger then top (get your mind out of the gutter)
    unsigned short int left;
    unsigned short int top;

    double depth; // the fov of the camera
    double depthRadians;
    double focalLength;

    double nearClippingPlane; // objects closer to the camera then this value will not be rendered
    double farClippingPlane; // objects that are further to the camera then this value will not be rendered

    double xRotation;
    double yRotation;
    double zRotation;

    double xRotationRadians;
    double yRotationRadians;
    double zRotationRadians;

    double projectionMatrix[4][4];

    point zVector;
    point yVector;
    point xVector;

    point zNormallizedVector;
    point yNormallizedVector;
    point xNormallizedVector;

    point viewMatrix[3];
} camera;

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0); /* 0, 0 gives default characters
                            * for the vertical and horizontal
                            * lines*/
    wrefresh(local_win);   /* Show that box*/

    return local_win;
}

void destroy_win(WINDOW *local_win) {
    /* box(local_win, ' ', ' '); : This won't produce the desired
    * result of erasing the window. It will leave it's four corners
    * and so an ugly remnant of window.
    */
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    /* The parameters taken are
    * 1. win: the window on which to operate
    * 2. ls: character to be used for the left side of the window
    * 3. rs: character to be used for the right side of the window
    * 4. ts: character to be used for the top side of the window
    * 5. bs: character to be used for the bottom side of the window
    * 6. tl: character to be used for the top left corner of the window
    * 7. tr: character to be used for the top right corner of the window
    * 8. bl: character to be used for the bottom left corner of the window
    * 9. br: character to be used for the bottom right corner of the window
    */
    wrefresh(local_win);
    delwin(local_win);
}

point normallizeVector(point startOfVector, point endOfVector) {
    point normallizedVector = {
        endOfVector.x - startOfVector.x,
        endOfVector.y - startOfVector.y,
        endOfVector.z - startOfVector.z
    };
    double n = sqrt(pow(normallizedVector.x, 2) + pow(normallizedVector.y, 2) + pow(normallizedVector.z, 2));
    normallizedVector.x /= n;
    normallizedVector.y /= n;
    normallizedVector.z /= n;

    normallizedVector.x += startOfVector.x;
    normallizedVector.y += startOfVector.y;
    normallizedVector.z += startOfVector.z;

    return normallizedVector;
}

void createCamera(camera* camera, double x, double y, double z, unsigned short int left, unsigned short int top, unsigned short int right, unsigned short int bottom, double depth, double nearClippingPlane, double farClippingPlane, double xRotation, double yRotation, double zRotation) {
    // z is forward relative
    // x is right relative
    // y is down relative
    camera->originPoint.x = x;
    camera->originPoint.y = y;
    camera->originPoint.z = z;

    camera->right = right;
    camera->bottom = bottom;

    camera->depth = depth;
    camera->depthRadians = depth*PI/180;
    camera->focalLength = 1 / tan(camera->depthRadians / 2);

    camera->nearClippingPlane = nearClippingPlane;
    camera->farClippingPlane = farClippingPlane;

    camera->xRotation = xRotation;
    camera->yRotation = yRotation;
    camera->zRotation = zRotation;

    camera->xRotationRadians = xRotation*PI/180;
    camera->yRotationRadians = yRotation*PI/180;
    camera->zRotationRadians = zRotation*PI/180;

    camera->projectionMatrix[0][0] = camera->focalLength / (camera->bottom/camera->right);
    camera->projectionMatrix[0][1] = 0;
    camera->projectionMatrix[0][2] = 0;
    camera->projectionMatrix[0][3] = 0;

    camera->projectionMatrix[1][0] = 0;
    camera->projectionMatrix[1][1] = camera->focalLength;
    camera->projectionMatrix[1][2] = 0;
    camera->projectionMatrix[1][3] = 0;

    camera->projectionMatrix[2][0] = 0;
    camera->projectionMatrix[2][1] = 0;
    camera->projectionMatrix[2][2] = (farClippingPlane + nearClippingPlane) / (nearClippingPlane - farClippingPlane);
    camera->projectionMatrix[2][3] = (2 * farClippingPlane * nearClippingPlane) / (nearClippingPlane - farClippingPlane);

    camera->projectionMatrix[3][0] = 0;
    camera->projectionMatrix[3][1] = 0;
    camera->projectionMatrix[3][2] = -1;
    camera->projectionMatrix[3][3] = 0;

    camera->zVector.x = cos(camera->yRotationRadians) * cos(camera->xRotationRadians);
    camera->zVector.y = sin(camera->xRotationRadians);
    camera->zVector.z = sin(camera->yRotationRadians) * cos(camera->xRotationRadians);
    camera->zNormallizedVector = normallizeVector(camera->originPoint, camera->zVector);

    camera->xVector.x = camera->zVector.z;
    camera->xVector.y = camera->zVector.y;
    camera->xVector.z = -camera->zVector.x;
    camera->xNormallizedVector = normallizeVector(camera->originPoint, camera->xVector);

    camera->yVector.x = camera->xVector.y;
    camera->yVector.y = -camera->xVector.x;
    camera->yVector.z = camera->xVector.z;
    camera->xNormallizedVector = normallizeVector(camera->originPoint, camera->yVector);
}

point findOriginPointByGeometry(point arr[], int sizeOfArr) {
    /**
     * findOriginPointByGeometry(point array[], int sizeOfArray)
     * return originPoint
     * takes an array of all the points of a 3d object and the size of the array and returns the average position of all these points
     */
    point sumPoint = {0, 0, 0};
    for (int i = 0; i < sizeOfArr; i++) {
        sumPoint.x += arr[i].x;
        sumPoint.y += arr[i].y;
        sumPoint.z += arr[i].z;
    }
    point originPoint = {sumPoint.x/sizeOfArr, sumPoint.y/sizeOfArr, sumPoint.z/sizeOfArr};
    return originPoint;
}

point projectPoint(point pointToProject, camera camera) {
    // t = (ygiven - ay) / (by - ay)
    // given = a + t(b - a)
    double t = (camera.nearClippingPlane - pointToProject.z) / (camera.originPoint.z - pointToProject.z);
    point pointProjected = {
        pointToProject.x + t*(camera.originPoint.x - pointToProject.x),
        pointToProject.y + t*(camera.originPoint.y - pointToProject.y),
        pointToProject.z
    };

    pointProjected.x *= camera.depthRadians;
    pointProjected.y *= camera.depthRadians;

    pointProjected.x *= camera.right;
    pointProjected.y *= camera.bottom;

    pointProjected.x += camera.right/2;
    pointProjected.y += camera.bottom/2;

    return pointProjected;
}

void projectArr(point arr[], camera camera, int sizeOfArr) {
    for (int i = 0; i < sizeOfArr; i++) {
        arr[i] = projectPoint(arr[i], camera);
    }
}

void printPoint(point point, camera camera, char lookPoint) {
    point = projectPoint(point, camera);
    gotoxy((int) point.x, (int) point.y);
    printf("%c", lookPoint);
}

void printPoints(point arr[], int sizeOfArr, char lookPoint, camera camera) {
    for (int eachPoint = 0; eachPoint < sizeOfArr; eachPoint++) {
        printPoint(arr[eachPoint], camera, lookPoint);
    }
    fflush(stdout);
}

void _printEdges(obj3d obj3d, camera camera) {
    for (int i = 0; i < obj3d.edges.length; i++) {
        point point1 = {
            obj3d.edges.arr[i].point1->x,
            obj3d.edges.arr[i].point1->y,
            obj3d.edges.arr[i].point1->z
        };
        point point2 = {
            obj3d.edges.arr[i].point2->x,
            obj3d.edges.arr[i].point2->y,
            obj3d.edges.arr[i].point2->z
        };
        point1 = projectPoint(point1, camera);
        point2 = projectPoint(point2, camera);
        double m = (point2.y-point1.y)/(point2.x-point1.x);
        double xoff = point2.x - point1.x;
        double yoff = point2.y - point1.y;
        if (fabs(xoff) >= fabs(yoff)) {
            int x = 0;
            while (abs(x) < fabs(xoff)) {
                gotoxy((int) point1.x+x, (int) (point1.y+x*m));
                if (xoff < 0) {
                    x--;
                } else if (xoff > 0) {
                    x++;
                }
            }
        }
        else if (fabs(yoff) >= fabs(xoff)) {

        }
    }
}

void printEdges(obj3d obj3d, camera camera) {
    gotoxy(0, 0);
    for (int i = 0; i < obj3d.edges.length; i++) {
        point point1 = {
            obj3d.edges.arr[i].point1->x,
            obj3d.edges.arr[i].point1->y,
            obj3d.edges.arr[i].point1->z
        };
        point point2 = {
            obj3d.edges.arr[i].point2->x,
            obj3d.edges.arr[i].point2->y,
            obj3d.edges.arr[i].point2->z
        };
        point1 = projectPoint(point1, camera);
        point2 = projectPoint(point2, camera);
        double xoff = point2.x - point1.x;
        double yoff = point2.y - point1.y;

        if (fabs(xoff) >= fabs(yoff)) {
            int y = 0;
            double addOrSub;
            if (point2.y < point1.y) {
                double addOrSub = -1;
            } else {
                double addOrSub = 1;
            }
            for (int x = 1; x < (int) fabs(xoff); x++) {
                double r1 = sqrt(pow(point2.y - (point1.y+y+addOrSub), 2) + pow(point2.x - (point1.x+x) , 2));
                double r2 = sqrt(pow(point2.y - (point1.y+y), 2) + pow(point2.x - (point1.x+x) , 2));
                if (point2.y > point1.y && r1 > r2) {
                    y++;
                }
                else if (point2.y < point1.y) {
                    y--;
                }
                //printf(" %f, %f, %d, %d \n ",point1.x, point1.y, x, y);
                gotoxy((int)point1.x+x, (int)point1.y+y);
                printf("*");
            }
        }
        else if (fabs(yoff) >= fabs(xoff)) {
            int x = 0;
            double addOrSub;
            if (point2.y < point1.y) {
                double addOrSub = -1;
            } else {
                double addOrSub = 1;
            }
            for (int y = 1; y < (int) fabs(yoff); y++) {
                double r1 = sqrt(pow(point2.y - (point1.y+y), 2) + pow(point2.x - (point1.x+x+addOrSub) , 2));
                double r2 = sqrt(pow(point2.y - (point1.y+y), 2) + pow(point2.x - (point1.x+x) , 2));
                if (point2.y > point1.y && r1 > r2) {
                    x++;
                }
                else if (point2.y < point1.y) {
                    x--;
                }
                //printf(" %f, %f, %d, %d \n ",point1.x, point1.y, x, y);
                gotoxy((int)point1.x+x, (int)point1.y+y);
                printf("*");
            }
        }
    }
}

void __printEdges(obj3d *obj3d, camera camera) {
    for (int eachEdge = 0; eachEdge < obj3d->edges.length; eachEdge++) {
        point point1 = {
            obj3d->edges.arr[eachEdge].point1->x,
            obj3d->edges.arr[eachEdge].point1->y,
            obj3d->edges.arr[eachEdge].point1->z
        };
        point point2 = {
            obj3d->edges.arr[eachEdge].point2->x,
            obj3d->edges.arr[eachEdge].point2->y,
            obj3d->edges.arr[eachEdge].point2->z
        };
        point1 = projectPoint(point1, camera);
        point2 = projectPoint(point2, camera);

        int howMuchX = point2.x - point1.x;
        int howMuchY = point2.y - point1.y;

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
                    gotoxy((int) point1.x+x, (int) point1.y+y);
                    printf("%c\n", obj3d->lookEdges);

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

                    int tempAmtToAdd = (point2.y-point1.y)/(point2.x-point1.x);
                    if (tempAmtToAdd != 0) {
                        amtToAdd = tempAmtToAdd;
                    }
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
                    gotoxy((int) point1.x+x, (int) point1.y+y);
                    printf("%c\n", obj3d->lookEdges);

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
                    int tempAmtToAdd = (point2.x-point1.x)/(point2.y-point1.y);
                    if (tempAmtToAdd != 0) {
                        amtToAdd = tempAmtToAdd;
                    }
                }
        }
    }
    fflush(stdout);
}

void create3dobjParams(obj3d* obj3d, short int isUsed, int sizePoints, int sizeEdges, int sizeTriangles, char lookPoints, char lookEdges, char lookTriangles) {
    obj3d->isUsed = isUsed;
    obj3d->points.length = sizePoints;
    obj3d->points.capacity = sizePoints+1000;
    obj3d->edges.length = sizeEdges;
    obj3d->edges.capacity = sizeEdges+3000;
    obj3d->triangles.length = sizeTriangles;
    obj3d->triangles.capacity = sizeTriangles+3000;
    obj3d->lookPoints = lookPoints;
    obj3d->lookEdges = lookEdges;
    obj3d->lookTriangles = lookTriangles;
}

void createPoints(obj3d* obj3d, point arr[obj3d->points.length]) {
    obj3d->points.arr = (point*) malloc(sizeof(point) * obj3d->points.capacity);
    if (obj3d->points.arr == NULL) {
       printf("couldnt allocate memory!");
       exit(1);
    }
    for (int i = 0; i < obj3d->points.length; i++) {
        obj3d->points.arr[i] = arr[i];
    }
    obj3d->originPoint = findOriginPointByGeometry(obj3d->points.arr, obj3d->points.length);
}

void createEdges(obj3d *obj3d, edge arr[obj3d->edges.length]) {
    obj3d->edges.arr = (edge*) malloc(sizeof(edge) * obj3d->edges.capacity);
    if (obj3d->points.arr == NULL) {
        printf("couldnt allocate memory!");
        exit(1);
    }
    for (int i = 0; i < obj3d->edges.length; i++) {
        obj3d->edges.arr[i] = arr[i];
    }
}
void createFaces(obj3d *obj3d, triangle arr [obj3d->triangles.length]) {
    obj3d->triangles.arr = (triangle*) malloc(sizeof(triangle) * obj3d->triangles.capacity);
    if (obj3d->triangles.arr == NULL) {
        printf("couldnt allocate memory!");
        exit(1);
    }
    for (int i = 0; i < obj3d->triangles.length; i++) {
        obj3d->triangles.arr[i] = arr[i];
    }
}

void create3dobj(obj3d* obj3d, short int isUsed, char lookPoints, char lookEdges, char lookTriangles, int sizePoints, int sizeEdges, int sizeTriangles, ...) {
    create3dobjParams(obj3d, isUsed, sizePoints, sizeEdges, sizeTriangles, lookPoints, lookEdges, lookTriangles);
    va_list args;
    va_start(args, sizeTriangles);

    point arrP[obj3d->points.length];
    for (int i = 0; i < obj3d->points.length; i++) {
        arrP[i].x = va_arg(args, double);
        arrP[i].y = va_arg(args, double);
        arrP[i].z = va_arg(args, double);
    }
    createPoints(obj3d, arrP);

    edge arrE[obj3d->edges.length];
    for (int i = 0; i < obj3d->edges.length; i++) {
        arrE[i].point1 = &(obj3d->points.arr[va_arg(args, int)]);
        arrE[i].point2 = &(obj3d->points.arr[va_arg(args, int)]);
    }
    createEdges(obj3d, arrE);

    triangle arrT[obj3d->triangles.length];
    for (int i = 0; i < obj3d->edges.length; i++) {
        arrT[i].edge1 = &(obj3d->edges.arr[va_arg(args, int)]);
        arrT[i].edge2 = &(obj3d->edges.arr[va_arg(args, int)]);
        arrT[i].edge3 = &(obj3d->edges.arr[va_arg(args, int)]);
    }
    createFaces(obj3d, arrT);
}

void deletePoints(obj3d* obj3d) {
    free(obj3d->points.arr);
}
void deleteEdges(obj3d* obj3d) {
    free(obj3d->edges.arr);
}
void deleteFaces(obj3d* obj3d) {
    free(obj3d->triangles.arr);
}
void delete3DObj(obj3d* obj3d) {
    deletePoints(obj3d);
    deleteEdges(obj3d);
    deleteFaces(obj3d);
}

void rotateObjZWithoutRecalOriginPoint(obj3d* obj3d, double howMuchToRotate) {
    /*
     * rotaion matrix:
     * | cos(howMuchToRotate) * x, sin(howMuchToRotate) * x, 0 * x  |
     * | -sin(howMuchToRotate) * y, cos(howMuchToRotate) * y, 0 * y |
     * | 0 * z, 0 * z, 1 * z |
     */
    howMuchToRotate *= PI/180;
    double sinHowMuchToRotate = sin(howMuchToRotate);
    double cosHowMuchToRotate = cos(howMuchToRotate);
    for (int i = 0; i < obj3d->points.length; i++) {
        double xMinusOrigin = obj3d->points.arr[i].x - obj3d->originPoint.x;
        double yMinusOrigin = obj3d->points.arr[i].y - obj3d->originPoint.y;

        double x = xMinusOrigin;
        double y = yMinusOrigin;

        xMinusOrigin = cosHowMuchToRotate * x + (-sinHowMuchToRotate) * y;
        yMinusOrigin = sinHowMuchToRotate * x + cosHowMuchToRotate * y;

        xMinusOrigin += obj3d->originPoint.x;
        yMinusOrigin += obj3d->originPoint.y;

        obj3d->points.arr[i].x = xMinusOrigin;
        obj3d->points.arr[i].y = yMinusOrigin;
    }
}

void rotateObjz(obj3d* obj3d, double howMuchToRotate) {
    obj3d->originPoint = findOriginPointByGeometry(obj3d->points.arr, obj3d->points.length);
    rotateObjZWithoutRecalOriginPoint(obj3d, howMuchToRotate);
}

void rotateObjyWithoutRecalcOriginPoint(obj3d* obj3d, double howMuchToRotate) {
    /*
     * rotation matrix:
     * | cos(howMuchToRotate) * x, 0 * x, sin(howMuchToRotate) * x  |
     * | 0 * y, 1 * y, 0 * y |
     * | -sin(howMuchToRotate) * z, 0 * z, cos(howMuchToRotate) * z |
     */
    howMuchToRotate *= PI/180;
    double sinHowMuchToRotate = sin(howMuchToRotate);
    double cosHowMuchToRotate = cos(howMuchToRotate);
    for (int i = 0; i < obj3d->points.length; i++) {
        double xMinusOrigin = obj3d->points.arr[i].x - obj3d->originPoint.x;
        double zMinusOrigin = obj3d->points.arr[i].z - obj3d->originPoint.z;

        double x = xMinusOrigin;
        double z = zMinusOrigin;

        xMinusOrigin = cosHowMuchToRotate * x + (-sinHowMuchToRotate) * z;
        zMinusOrigin = sinHowMuchToRotate * x + cosHowMuchToRotate * z;

        xMinusOrigin += obj3d->originPoint.x;
        zMinusOrigin += obj3d->originPoint.z;

        obj3d->points.arr[i].x = xMinusOrigin;
        obj3d->points.arr[i].z = zMinusOrigin;
    }
}

void rotateObjy(obj3d* obj3d, double howMuchToRotate) {
    obj3d->originPoint = findOriginPointByGeometry(obj3d->points.arr, obj3d->points.length);
    rotateObjyWithoutRecalcOriginPoint(obj3d, howMuchToRotate);
}

void rotateObjxWithoutRecalcOriginPoint(obj3d* obj3d, double howMuchToRotate) {
    /*
     * rotation matrix:
     * | 1 * x, 0 * x, 0 * x  |
     * | 0 * y, cos(howMuchToRotate) * y, sin(howMuchToRotate) * y |
     * | 0 * z, -sin(cosHowMuchToRotate) * z, cos(howMuchToRotate) * z |
     */
    howMuchToRotate *= PI/180;
    double sinHowMuchToRotate = sin(howMuchToRotate);
    double cosHowMuchToRotate = cos(howMuchToRotate);
    for (int i = 0; i < obj3d->points.length; i++) {
        double yMinusOrigin = obj3d->points.arr[i].y - obj3d->originPoint.y;
        double zMinusOrigin = obj3d->points.arr[i].z - obj3d->originPoint.z;

        double y = yMinusOrigin;
        double z = zMinusOrigin;

        yMinusOrigin = cosHowMuchToRotate * y + (-sinHowMuchToRotate) * z;
        zMinusOrigin = sinHowMuchToRotate * y + cosHowMuchToRotate * z;

        yMinusOrigin += obj3d->originPoint.x;
        zMinusOrigin += obj3d->originPoint.z;

        obj3d->points.arr[i].y = yMinusOrigin;
        obj3d->points.arr[i].z = zMinusOrigin;
    }
}

void rotateObjx(obj3d* obj3d, double howMuchToRotate) {
    obj3d->originPoint = findOriginPointByGeometry(obj3d->points.arr, obj3d->points.length);
    rotateObjxWithoutRecalcOriginPoint(obj3d, howMuchToRotate);
}

void moveObjxWithoutRecalcOriginPoint(obj3d* obj3d, double howMuchToMove) {
    for (int i = 0; i < obj3d->points.length; i++) {
        obj3d->points.arr[i].x += howMuchToMove;
    }
}

void moveObjx(obj3d* obj3d, double howMuchToMove) {
    moveObjxWithoutRecalcOriginPoint(obj3d, howMuchToMove);
    obj3d->originPoint = findOriginPointByGeometry(obj3d->points.arr, obj3d->points.length);
}

void moveObjyWithoutRecalcOriginPoint(obj3d* obj3d, double howMuchToMove) {
    for (int i = 0; i < obj3d->points.length; i++) {
        obj3d->points.arr[i].y += howMuchToMove;
    }
}

void moveObjy(obj3d* obj3d, double howMuchToMove) {
    moveObjyWithoutRecalcOriginPoint(obj3d, howMuchToMove);
    obj3d->originPoint = findOriginPointByGeometry(obj3d->points.arr, obj3d->points.length);
}

void moveObjzWithoutRecalcOriginPoint(obj3d* obj3d, double howMuchToMove) {
    for (int i = 0; i < obj3d->points.length; i++) {
        obj3d->points.arr[i].z += howMuchToMove;
    }
}

void moveObjz(obj3d* obj3d, double howMuchToMove) {
    moveObjzWithoutRecalcOriginPoint(obj3d, howMuchToMove);
    obj3d->originPoint = findOriginPointByGeometry(obj3d->points.arr, obj3d->points.length);
}

int reallocArrP(dynamicArrP* dynamicArrP, size_t newSize) {
    int capacity = newSize+1000;
    dynamicArrP->arr = (point*) realloc(dynamicArrP->arr, sizeof(point) * capacity);
    if (dynamicArrP->arr == NULL) {
        return 1;
    }
    dynamicArrP->length = newSize;
    dynamicArrP->capacity = capacity;

    return 0;
}

int addElementP(dynamicArrP* dynamicArrP, size_t amtToAdd, ...) {
    va_list args;
    va_start(args, amtToAdd);

    point arr[amtToAdd];
    for (int i = 0; i < amtToAdd; i++) {
        point point = {
            va_arg(args, double),
            va_arg(args, double),
            va_arg(args, double)
        };
        arr[i] = point;
    }
    va_end(args);

    if (amtToAdd+dynamicArrP->length < dynamicArrP->capacity) {
        for (int i = 0; i < amtToAdd+dynamicArrP->length; i++) {
            dynamicArrP->arr[i+dynamicArrP->length] = arr[i];
        }
        dynamicArrP->length += amtToAdd;
        return 0;
    }

    int check = reallocArrP(dynamicArrP, dynamicArrP->length + amtToAdd);
    if (check == 1) {
        // reallocating has failed
        return 1;
    }
    for (int i = 0; i < amtToAdd; i++) {
        dynamicArrP->arr[dynamicArrP->length-amtToAdd+i] = arr[i];
    }
    return 0;
}

int removeElementP(dynamicArrP* dynamicArrP, size_t amtToRemove, ...) {
    va_list args;
    va_start(args, amtToRemove);

    int arr[amtToRemove];
    for (int i = 0; i < amtToRemove; i++) {
        arr[i] = va_arg(args, int);
    }
    va_end(args);

    for (int i = 0; i < dynamicArrP->length; i++) {
        for (int j = 0; j < amtToRemove; j++) {
            if (i == arr[j]) {
                for (int k = i; k < dynamicArrP->length-1; k++) {
                    dynamicArrP->arr[k] = dynamicArrP->arr[k+1];
                }
                dynamicArrP->arr[dynamicArrP->length].x = 0;
                dynamicArrP->arr[dynamicArrP->length].y = 0;
                dynamicArrP->arr[dynamicArrP->length].z = 0;
            }
        }

    }
    dynamicArrP->length -= amtToRemove;
    if (dynamicArrP->length+1000 < dynamicArrP->capacity) {
        int check = reallocArrP(dynamicArrP, dynamicArrP->length);
        if (check == 1) {
            return 1;
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int reallocArrE(dynamicArrE* dynamicArrE, size_t newSize) {
    int capacity = newSize+1000;
    dynamicArrE->arr = (edge*) realloc(dynamicArrE->arr, sizeof(edge) * capacity);
    if (dynamicArrE->arr == NULL) {
        return 1;
    }
    dynamicArrE->length = newSize;
    dynamicArrE->capacity = capacity;

    return 0;
}

int addElementE(dynamicArrP* dynamicArrP, dynamicArrE* dynamicArrE, int amtToAdd, ...) {
    va_list args;
    va_start(args, amtToAdd);

    edge arr[amtToAdd];
    for (int i = 0; i < amtToAdd; i++) {
        edge edge = {
            &(dynamicArrP->arr[va_arg(args, int)]),
            &(dynamicArrP->arr[va_arg(args, int)])
        };
        arr[i] = edge;
    }
    va_end(args);

    if (amtToAdd+dynamicArrE->length < dynamicArrE->capacity) {
        for (int i = 0; i < amtToAdd+dynamicArrE->length; i++) {
            dynamicArrE->arr[i+dynamicArrE->length] = arr[i];
        }
        dynamicArrE->length += amtToAdd;
        return 0;
    }

    int check = reallocArrE(dynamicArrE, dynamicArrE->length + amtToAdd);
    if (check == 1) {
        // reallocating has failed
        return 1;
    }
    for (int i = 0; i < amtToAdd; i++) {
        dynamicArrE->arr[dynamicArrE->length-amtToAdd+i] = arr[i];
    }
    return 0;
}

int removeElementE(dynamicArrE* dynamicArrE, size_t amtToRemove, ...) {
    va_list args;
    va_start(args, amtToRemove);

    int arr[amtToRemove];
    for (int i = 0; i < amtToRemove; i++) {
        arr[i] = va_arg(args, int);
    }
    va_end(args);

    for (int i = 0; i < dynamicArrE->length; i++) {
        for (int j = 0; j < amtToRemove; j++) {
            if (i == arr[j]) {
                for (int k = i; k < dynamicArrE->length-1; k++) {
                    dynamicArrE->arr[k] = dynamicArrE->arr[k+1];
                }
                dynamicArrE->arr[dynamicArrE->length].x = 0;
                dynamicArrE->arr[dynamicArrE->length].y = 0;
                dynamicArrE->arr[dynamicArrE->length].z = 0;
            }
        }

    }
    dynamicArrE->length -= amtToRemove;
    if (dynamicArrE->length+1000 < dynamicArrE->capacity) {
        int check = reallocArrP(dynamicArrP, dynamicArrP->length);
        if (check == 1) {
            return 1;
        }
    }
    return 0;
}

int main() {
    // z is forward relative
    // x is right relative
    // y is down relative
    cbreak();
    clear();
    ioctl(0, TIOCGWINSZ, &w);

    camera camera;
    createCamera(&camera, 0.0, 0.0, 0.5, 0, 0, w.ws_col, w.ws_row, 70, 0.1, 1000, 0, 0, 0);
    obj3d sqr;
    create3dobj(&sqr, 1, '*', '*', '*', 8, 12, 0,
        -0.1, -0.1, 1.0,
        0.1, -0.1, 1.0,
        -0.1, 0.1, 1.0,
        0.1, 0.1, 1.0,

        -0.1, -0.1, 1.2,
        0.1, -0.1, 1.2,
        -0.1, 0.1, 1.2,
        0.1, 0.1, 1.2,

        0,1,
        0,2,
        3,2,
        3,1,

        4,5,
        4,6,
        7,5,
        7,6,

        0,4,
        1,5,
        2,6,
        3,7
    );
    printPoints(sqr.points.arr, sqr.points.length, sqr.lookPoints, camera);
    //printEdges(sqr, camera);

    msleep(60);
    clear();
    while(1) {
        //rotateObjx(&sqr, 3);
        //rotateObjy(&sqr, 3);
        rotateObjz(&sqr, 1);
        //moveObjx(&sqr, 0.02);
        printPoints(sqr.points.arr, sqr.points.length, sqr.lookPoints, camera);
        //printEdges(sqr, camera);
        printPoint(sqr.originPoint, camera, 'X');

        point o = projectPoint(sqr.originPoint, camera);
        gotoxy(0, 0);
        //printf("%f, %f, %f\n", sqr.originPoint.x, sqr.originPoint.y, sqr.originPoint.z);
        //printf("%d, %d", w.ws_col, w.ws_row);

        msleep(60);
        clear();
    }

    gotoxy(0, 0);
    endwin();
    return 0;
}
