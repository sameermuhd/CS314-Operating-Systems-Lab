#include <bits/stdc++.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct pixel {
    int r, g, b;
};

struct graypixel {
    int val;
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

void convertToGreyscale(vector<vector<pixel>> &inputImage, int height, int width, int pid,
                        int pipefds[2]) {
    if (pid > 0) return;

    struct graypixel grayPixel[1];

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int greyScaleValue = (0.2989 * inputImage[i][j].r) + (0.5870 * inputImage[i][j].g) +
                                 (0.1140 * inputImage[i][j].b);

            grayPixel[0].val = greyScaleValue;
            write(pipefds[1], grayPixel, sizeof(struct graypixel));
        }
    }

    exit(0);
}

void imageEdgeDectection(vector<vector<pixel>> &outputImage, int height, int width, int pid,
                         int pipefds[2], char *imgType, int pixelMax, FILE *outFile) {
    if (pid > 0) return;

    pixel imgHor[height][width];
    pixel imgVer[height][width];
    pixel imgMag[height][width];

    vector<vector<pixel>> inputImage(height, vector<pixel>(width));

    graypixel grayPixel[0];

    /// magnitude
    int maxR = -200;
    int minR = 2000;
    int maxG = -200;
    int minG = 2000;
    int maxB = -200;
    int minB = 2000;

    int currentR, currentG, currentB;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            read(pipefds[0], grayPixel, sizeof(struct graypixel));

            inputImage[i][j].r = grayPixel[0].val;
            inputImage[i][j].g = grayPixel[0].val;
            inputImage[i][j].b = grayPixel[0].val;

            if (i == 0 || i == 1 || j == 0 || j == 1) {
                continue;
            }

            {
                i = i - 1;
                j = j - 1;

                currentR = inputImage[i - 1][j - 1].r + 2 * inputImage[i - 1][j].r +
                           inputImage[i - 1][j + 1].r - inputImage[i + 1][j - 1].r -
                           2 * inputImage[i + 1][j].r - inputImage[i + 1][j + 1].r;
                currentG = inputImage[i - 1][j - 1].g + 2 * inputImage[i - 1][j].g +
                           inputImage[i - 1][j + 1].g - inputImage[i + 1][j - 1].g -
                           2 * inputImage[i + 1][j].g - inputImage[i + 1][j + 1].g;
                currentB = inputImage[i - 1][j - 1].b + 2 * inputImage[i - 1][j].b +
                           inputImage[i - 1][j + 1].b - inputImage[i + 1][j - 1].b -
                           2 * inputImage[i + 1][j].b - inputImage[i + 1][j + 1].b;
                imgHor[i][j].r = currentR;
                imgHor[i][j].g = currentG;
                imgHor[i][j].b = currentB;

                currentR = inputImage[i - 1][j - 1].r + 2 * inputImage[i][j - 1].r +
                           inputImage[i + 1][j - 1].r - inputImage[i - 1][j + 1].r -
                           2 * inputImage[i][j + 1].r - inputImage[i + 1][j + 1].r;
                currentG = inputImage[i - 1][j - 1].g + 2 * inputImage[i][j - 1].g +
                           inputImage[i + 1][j - 1].g - inputImage[i - 1][j + 1].g -
                           2 * inputImage[i][j + 1].g - inputImage[i + 1][j + 1].g;
                currentB = inputImage[i - 1][j - 1].b + 2 * inputImage[i][j - 1].b +
                           inputImage[i + 1][j - 1].b - inputImage[i - 1][j + 1].b -
                           2 * inputImage[i][j + 1].b - inputImage[i + 1][j + 1].b;
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

                i = i + 1;
                j = j + 1;
            }
        }
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

    int pipefds[2];
    int pipeDesc1;

    pipeDesc1 = pipe(pipefds);

    if (pipeDesc1 == -1) perror("pipe");

    vector<vector<pixel>> greyScaleImage(height, vector<pixel>(width));
    vector<vector<pixel>> edgeImage(height, vector<pixel>(width));

    auto startTime = chrono::high_resolution_clock::now();

    convertToGreyscale(image, height, width, fork(), pipefds);
    imageEdgeDectection(edgeImage, height, width, fork(), pipefds, imgType, pixelMax, outFile);

    while (wait(NULL) > 0)
        ;

    auto stopTime = chrono::high_resolution_clock::now();
    auto timeTaken = chrono::duration_cast<microseconds>(stopTime - startTime);
    cout << "Time Taken: " << timeTaken.count() << " microseconds\n";

    return 0;
}
