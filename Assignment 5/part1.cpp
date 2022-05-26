#include <bits/stdc++.h>

#include <chrono>
#include <vector>

using namespace std;
using namespace std::chrono;

typedef struct pixel {
    int r, g, b;
} pixel;

void imageEdgeDectection(vector<vector<pixel>>& inputImage, vector<vector<pixel>>& outputImage,
                         int height, int width) {
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
        }
    }

    int diffR = maxR - minR;
    int diffG = maxG - minG;
    int diffB = maxB - minB;

    // cout << maxR << " " << maxG << " " << maxB << "\n";

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
}

void convertToGreyscale(vector<vector<pixel>>& inputImage, vector<vector<pixel>>& outputImage,
                        int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int greyScaleValue = (0.2989 * inputImage[i][j].r) + (0.5870 * inputImage[i][j].g) +
                                 (0.1140 * inputImage[i][j].b);
            outputImage[i][j].r = greyScaleValue;
            outputImage[i][j].g = greyScaleValue;
            outputImage[i][j].b = greyScaleValue;
        }
    }
}

void printImage(char* imgType, int width, int height, int pixelMax, vector<vector<pixel>>& image,
                FILE* output) {
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

int main(int argv, char* argc[]) {
    if (argv != 3) {
        cout << "Usage: ./a.out <path-to-original-image>.ppm <path-to-transformed-image>\n";
        exit(1);
    }
    FILE* inFile = fopen(argc[1], "r");
    FILE* outFile = fopen(argc[2], "w");

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

    vector<vector<pixel>> greyScaleImage(height, vector<pixel>(width));
    vector<vector<pixel>> edgeImage(height, vector<pixel>(width));

    auto startTime = chrono::high_resolution_clock::now();

    convertToGreyscale(image, image, height, width);
    imageEdgeDectection(image, edgeImage, height, width);

    printImage(imgType, width, height, pixelMax, edgeImage, outFile);

    auto stopTime = chrono::high_resolution_clock::now();
    auto timeTaken = chrono::duration_cast<microseconds>(stopTime - startTime);
    cout << "Time Taken: " << timeTaken.count() << " microseconds\n";

    return 0;
}