#include <bits/stdc++.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;

#define SEM_NAME "/semLock"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

struct pixel {
    int r, g, b;
};

void printImage(char *imgType, int width, int height, int pixelMax, vector<vector<pixel>> &image,
                FILE *output) {
    fprintf(output, "%s\n", imgType);
    fprintf(output, "%d %d\n", width, height);
    fprintf(output, "%d\n", pixelMax);
    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            fprintf(output, "%d ", image[i][j].r);
            fprintf(output, "%d ", image[i][j].g);
            fprintf(output, "%d ", image[i][j].b);
        }
        fprintf(output, "\n");
    }
}

void convertToGreyscale(key_t shmKey2, key_t shmKey1, int pid, int height, int width) {
    if (pid > 0) return;

    int shmid = shmget(shmKey1, sizeof(struct pixel) * height * width, 0666 | IPC_CREAT);
    pixel *shmImage = (struct pixel *)shmat(shmid, NULL, 0);

    int shmid2 = shmget(shmKey2, sizeof(int) * 2, 0666 | IPC_CREAT);
    int *greyProgress = (int *)shmat(shmid2, NULL, 0);

    sem_t *semL = sem_open(SEM_NAME, O_RDWR);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            sem_wait(semL);
            struct pixel current = shmImage[i * width + j];

            int greyScaleValue = (0.2989 * current.r) + (0.5870 * current.g) + (0.1140 * current.b);

            current.r = greyScaleValue;
            current.g = greyScaleValue;
            current.b = greyScaleValue;

            shmImage[i * width + j] = current;

            greyProgress[0] = i + 1;
            greyProgress[1] = j + 1;

            sem_post(semL);
        }
    }
    exit(0);
}

void imageEdgeDectection(key_t shmKey2, key_t shmKey1, int pid, int height, int width,
                         char *imgType, int pixelMax, FILE *outFile) {
    if (pid > 0) return;

    auto ind_ = [&width](int i, int j) { return i * width + j; };

    int shmid = shmget(shmKey1, sizeof(struct pixel) * height * width, 0666 | IPC_CREAT);
    pixel *inputImage = (struct pixel *)shmat(shmid, NULL, 0);

    int shmid2 = shmget(shmKey2, sizeof(int) * 2, 0666 | IPC_CREAT);
    int *greyProgress = (int *)shmat(shmid2, NULL, 0);

    vector<vector<pixel>> outputImage(height, vector<pixel>(width));

    sem_t *semL = sem_open(SEM_NAME, O_RDWR);

    bool workedOnPixel = false;

    pixel imgHor[height][width];
    pixel imgVer[height][width];
    pixel imgMag[height][width];

    /// magnitude
    int maxR = -200;
    int minR = 2000;
    int maxG = -200;
    int minG = 2000;
    int maxB = -200;
    int minB = 2000;

    int currentR, currentG, currentB;

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            sem_wait(semL);

            if (j + 1 <= greyProgress[1] && i + 1 <= greyProgress[0]) {
                currentR = inputImage[ind_(i - 1, j - 1)].r + 2 * inputImage[ind_(i - 1, j)].r +
                           inputImage[ind_(i - 1, j + 1)].r - inputImage[ind_(i + 1, j - 1)].r -
                           2 * inputImage[ind_(i + 1, j)].r - inputImage[ind_(i + 1, j + 1)].r;
                currentG = inputImage[ind_(i - 1, j - 1)].g + 2 * inputImage[ind_(i - 1, j)].g +
                           inputImage[ind_(i - 1, j + 1)].g - inputImage[ind_(i + 1, j - 1)].g -
                           2 * inputImage[ind_(i + 1, j)].g - inputImage[ind_(i + 1, j + 1)].g;
                currentB = inputImage[ind_(i - 1, j - 1)].b + 2 * inputImage[ind_(i - 1, j)].b +
                           inputImage[ind_(i - 1, j + 1)].b - inputImage[ind_(i + 1, j - 1)].b -
                           2 * inputImage[ind_(i + 1, j)].b - inputImage[ind_(i + 1, j + 1)].b;
                imgHor[i][j].r = currentR;
                imgHor[i][j].g = currentG;
                imgHor[i][j].b = currentB;

                currentR = inputImage[ind_(i - 1, j - 1)].r + 2 * inputImage[ind_(i, j - 1)].r +
                           inputImage[ind_(i + 1, j - 1)].r - inputImage[ind_(i - 1, j + 1)].r -
                           2 * inputImage[ind_(i, j + 1)].r - inputImage[ind_(i + 1, j + 1)].r;
                currentG = inputImage[ind_(i - 1, j - 1)].g + 2 * inputImage[ind_(i, j - 1)].g +
                           inputImage[ind_(i + 1, j - 1)].g - inputImage[ind_(i - 1, j + 1)].g -
                           2 * inputImage[ind_(i, j + 1)].g - inputImage[ind_(i + 1, j + 1)].g;
                currentB = inputImage[ind_(i - 1, j - 1)].b + 2 * inputImage[ind_(i, j - 1)].b +
                           inputImage[ind_(i + 1, j - 1)].b - inputImage[ind_(i - 1, j + 1)].b -
                           2 * inputImage[ind_(i, j + 1)].b - inputImage[ind_(i + 1, j + 1)].b;
                imgVer[i][j].r = currentR;
                imgVer[i][j].g = currentG;
                imgVer[i][j].b = currentB;

                imgMag[i][j].r = sqrt(pow(imgHor[i][j].r, 2) + pow(imgVer[i][j].r, 2));
                if (imgMag[i][j].r > maxR) maxR = imgMag[i][j].r;
                if (imgMag[i][j].r < minR) minR = imgMag[i][j].r;

                imgMag[i][j].g = sqrt(pow(imgHor[i][j].g, 2) + pow(imgVer[i][j].g, 2));
                if (imgMag[i][j].g > maxG) maxG = imgMag[i][j].g;
                if (imgMag[i][j].g < minG) minG = imgMag[i][j].g;

                imgMag[i][j].b = sqrt(pow(imgHor[i][j].b, 2) + pow(imgVer[i][j].b, 2));
                if (imgMag[i][j].b > maxB) maxB = imgMag[i][j].b;
                if (imgMag[i][j].b < minB) minB = imgMag[i][j].b;

                workedOnPixel = true;
            } else {
                workedOnPixel = false;
            }
            sem_post(semL);

            if (!workedOnPixel) j -= 1;
        }
        if (!workedOnPixel) i -= 1;
    }

    int diffR = maxR - minR;
    int diffG = maxG - minG;
    int diffB = maxB - minB;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float normalR = (imgMag[i][j].r - minR) / (diffR * 1.0);
            imgMag[i][j].r = normalR * 255;

            float normalG = (imgMag[i][j].g - minG) / (diffG * 1.0);
            imgMag[i][j].g = normalG * 255;

            float normalB = (imgMag[i][j].b - minB) / (diffB * 1.0);
            imgMag[i][j].b = normalB * 255;
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            outputImage[i][j] = imgMag[i][j];
        }
    }

    printImage(imgType, width, height, pixelMax, outputImage, outFile);

    exit(0);
}

int main(int argv, char *argc[]) {
    if (argv != 3) {
        cout << "Usage: ./a.out <path-to-original-image>.ppm <path-to-transformed-image>\n";
        exit(1);
    }
    FILE *inFile = fopen(argc[1], "r");
    FILE *outFile = fopen(argc[2], "w");

    if (inFile == NULL) {
        cout << "Input File not available\n";
        exit(1);
    }

    char imgType[6];
    int width, height;
    int pixelMax;
    fscanf(inFile, "%s", imgType);
    fscanf(inFile, "%d %d", &width, &height);
    fscanf(inFile, "%d", &pixelMax);

    vector<vector<pixel>> image(height, vector<pixel>(width));

    int red, green, blue;

    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            fscanf(inFile, "%d %d %d", &red, &green, &blue);
            image[i][j].r = red;
            image[i][j].g = green;
            image[i][j].b = blue;
        }
    }

    fclose(inFile);

    key_t shmKey1 = 0x1234;
    int shmid = shmget(shmKey1, sizeof(struct pixel) * height * width, 0666 | IPC_CREAT);
    pixel *shmImage = (struct pixel *)shmat(shmid, NULL, 0);

    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            shmImage[(i * width) + j] = image[i][j];
        }
    }

    key_t shmKey2 = 0x1235;
    int shmid2 = shmget(shmKey1, sizeof(int) * 2, 0666 | IPC_CREAT);
    int *greyProgress = (int *)shmat(shmid2, NULL, 0);

    sem_t *binarySemaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, SEM_PERMS, 1);

    auto startTime = chrono::high_resolution_clock::now();

    convertToGreyscale(shmKey2, shmKey1, fork(), height, width);
    imageEdgeDectection(shmKey2, shmKey1, fork(), height, width, imgType, pixelMax, outFile);

    while (wait(NULL) > 0)
        ;

    auto stopTime = chrono::high_resolution_clock::now();
    auto timeTaken = chrono::duration_cast<microseconds>(stopTime - startTime);
    cout << "Time Taken: " << timeTaken.count() << " microseconds\n";

    return 0;
}
