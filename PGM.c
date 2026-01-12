
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    int rows;         
    int cols;          
    int max_gray;    
    int **pixels;   
} Image;
//Part1:Initialization 
Image* create_image(int cols, int rows, int max_gray) {
    Image *img=(Image*)malloc(sizeof(Image));
    img->rows=rows; 
    img->cols=cols; 
    img->max_gray = max_gray;

  
    img->pixels =(int**) malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        img->pixels[i] = (int*) malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            img->pixels[i][j] = 0;
        }
    }
    return img;
}
// Part 2: Copying and Information Retieval
Image*copy_image(Image *source) {

    Image*new_copy=create_image(source->cols, source->rows, source->max_gray);

    for (int i=0; i < source->rows; i++) {
        for (int j=0; j < source->cols; j++) {
            new_copy->pixels[i][j] = source->pixels[i][j];
        }
    }
    return new_copy;
}
int get_rows(Image *img) {
    return img->rows;
}

int get_cols(Image *img) {
    return img->cols;
}

int get_max_gray(Image *img) {
    return img->max_gray;
}
//Part3:Rotation and Negation
Image* negate_image(Image *img) {
    Image *neg = (Image*)malloc(sizeof(Image));
    neg->rows = img->rows;
    neg->cols = img->cols;
    neg->max_gray = img->max_gray;

    neg->pixels = (int**) malloc(neg->rows * sizeof(int*));
    for (int i = 0; i < neg->rows; i++) {
        neg->pixels[i] = (int*) malloc(neg->cols*sizeof(int));
        for (int j=0; j < neg->cols; j++) {
            neg->pixels[i][j] = img->max_gray-img->pixels[i][j];
        }
    }
    return neg;
}

void rotate_image(Image *img, float angle) {
    int **temp = (int**) malloc(img->rows * sizeof(int*));
    for (int i = 0; i < img->rows; i++) {
        temp[i]=(int*)calloc(img->cols, sizeof(int));
    }

    float cx = img->cols/2.0;
    float cy = img->rows/2.0;

    for (int i = 0; i < img->rows; i++) {
        for (int j = 0; j < img->cols; j++) {
            float x = j - cx;
            float y = i - cy;

            int old_j = (int)(x * cos(angle) + y * sin(angle) + cx);
            int old_i = (int)(-x * sin(angle) + y * cos(angle) + cy);

            if (old_i >= 0 && old_i < img->rows && old_j >= 0 && old_j < img->cols) {
                temp[i][j] = img->pixels[old_i][old_j];
            } else {
                temp[i][j] = 0;
            }
        }
    }

    for (int i = 0; i < img->rows; i++) {
        for (int j = 0; j < img->cols; j++) {
            img->pixels[i][j] =temp[i][j];
        }
        free(temp[i]);
    }
    free(temp);
}
int is_valid(Image *img, int r, int c) {
    if (r >= 0 && r < img->rows && c >= 0 && c < img->cols) {
        return 1;
    }
    return 0;
}

int get_pixel(Image *img, int r, int c) {
    if (is_valid(img, r, c)) {
        return img->pixels[r][c];
    }
    return 0;
}

void set_pixel(Image *img, int r, int c, int val) {
    if (is_valid(img, r, c)) {
        img->pixels[r][c] = val;
    }
}

void print_image(Image *img, const char *title) {
    printf("\n--- %s ---\n", title);
    printf("Dimensions: %d rows x %d cols | Max Gray: %d\n\n", img->rows, img->cols, img->max_gray);

    for (int i = 0; i < img->rows; i++) {
        for (int j = 0; j < img->cols; j++) {
            printf("%2d ", img->pixels[i][j]);
        }
        printf("\n");
    }
    printf("---------------------------\n");
}
//Part5:Encoding Decoding 
/*1. Strategy to represent characters in pixels:*
Since the PGM format uses gray levels (like 0-15 or 0-255), we can’t always just swap a pixel for an ASCII character. A character like 'A' (ASCII 65) wouldn't fit in a pixel if the image max gray is only 15.

* *The Plan:* Break each 8-bit character into four 2-bit chunks.
* *The Logic:* Each chunk will have a value between 0 and 3 ( possibilities). This small range will easily fit into any PGM gray level without exceeding the max_gray limit.

*2. Embedding mechanism and pixel count:*
To hide the message without ruining the image, we will modify only the *Least Significant Bits (LSB)*.

* *Mechanism:* For each pixel, we take the original value and clear out the last two bits, then "inject" a 2-bit chunk of our character.
* *Pixel Usage:* Since a character is 8 bits and we hide 2 bits per pixel, we will use exactly *4 pixels per character*.

*3. Determining the end and avoiding data overwrite:*

* *End of message:* After the last character of the text, we will encode a "Null Character" (ASCII 0). When decoding, once the program reconstructs a 0, it knows the message is finished.
* *Avoiding Overwrite:* Before starting the loop, we calculate if (message_length + 1) * 4 is greater than the total number of pixels (rows * cols). If it is, the program should print an error saying the image is too small for the message.

*4. Decoding steps:*

* Create a loop that visits pixels in groups of four.
* For each pixel, use the modulo operator (pixel % 4) or bitwise AND (pixel & 3) to extract the hidden 2 bits.
* Shift these bits into a single byte to rebuild the original 8-bit value.
* Convert the resulting byte into a char.
* Repeat until the reconstructed character is \0.

*5. Recognizing boundaries during decoding:*
The decoder needs to follow the same "path" as the encoder. We use a nested loop (rows then columns). The boundary is naturally defined by the img->rows and img->cols variables. We must ensure the column counter resets and the row counter increments correctly to stay within the allocated 2D array memory.

*6. Converting pixel values back to characters:*
This is done using bit-shifting. For example, if we have four 2-bit values (), the character is reconstructed as:
char c = (b1 << 6) | (b2 << 4) | (b3 << 2) | b4;
This takes the small pieces and "glues" them back into a full ASCII letter.

*7. Potential challenges and reliability:*

* *Lossy Edits:* If someone opens the PGM and saves it as a JPEG, the subtle bit changes will be lost or blurred, making the message unreadable.
* *Low Gray Levels:* If the image max_gray is only 1 (a binary image), we can only hide 1 bit per pixel, which doubles the number of pixels needed and makes the process slower.
* *Metadata changes:* If the image is resized or cropped, the "path" of pixels is broken, and the message is destroyed.*/






int main() {
    int cols=24, rows=7, max_g=15;
    Image* original=create_image(cols, rows, max_g);

    int motif[7][24]={
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 3, 3, 3, 3, 0, 0, 7, 7, 7, 7, 0, 0, 11, 11, 11, 11, 0, 0, 15, 15, 15, 15, 0},
        {0, 3, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 15, 0, 0, 15, 0},
        {0, 3, 3, 3, 0, 0, 0, 7, 7, 7, 0, 0, 0, 11, 11, 11, 0, 0, 0, 15, 15, 15, 15, 0},
        {0, 3, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 15, 0, 0, 0, 0},
        {0, 3, 0, 0, 0, 0, 0, 7, 7, 7, 7, 0, 0, 11, 11, 11, 11, 0, 0, 15, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            original->pixels[i][j]=motif[i][j];
        }
    }

    print_image(original, "1. IMAGE ORIGINALE");
    printf("\n--- Application de la fonction : copy_image ---\n");
    Image* copy = copy_image(original);
    print_image(copy, "RÉSULTAT : COPIE CONFORME");
    printf("\n--- Application de la fonction : negate_image (sur l'original) ---\n");
    Image* negative = negate_image(original);
    print_image(negative, "RÉSULTAT : IMAGE INVERSÉE (NÉGATIF)");
    printf("\n--- Application de la fonction : rotate_image (45 degres sur l'original) ---\n");
    Image* rotated = copy_image(original);
    rotate_image(rotated, 0.785);
    print_image(rotated, "RÉSULTAT : IMAGE ROTATÉE");
    printf("\n--- Application des fonctions : set_pixel & get_pixel ---\n");
    Image* modified=copy_image(original);
    set_pixel(modified,0,0,15);
    printf("Valeur lue au pixel (0,0) apres modification : %d\n", get_pixel(modified, 0, 0));
    print_image(modified, "RÉSULTAT : PIXEL (0,0) MODIFIÉ");

    return 0;
}