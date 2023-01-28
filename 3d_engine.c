#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

struct def3DObj {
    int isUsed;
    char lookPoints;
    char lookEdges;

    int* arrPoints;
    int* arrEdges;
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

int main() {
    clear();

    struct def3DObj square;
    square.arrPoints = malloc(sizeof(int[1][1]));
    square.arrEdges = malloc(sizeof(int[1][1][1]));

    free(square.arrPoints);
    free(square.arrEdges);
    return 0;
}
