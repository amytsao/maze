#include "tga.h"
#include <stdio.h>

bool TgaImage::loadTGA(const char *filename)
{
    FILE *file;
    unsigned char type[4];
    unsigned char info[6];

    file = fopen(filename, "rb");

    if (!file)
        return false;

    fread (&type, sizeof (char), 3, file);
    fseek (file, 12, SEEK_SET);
    fread (&info, sizeof (char), 6, file);

    //image type either 2 (color) or 3 (greyscale)
    if (type[1] != 0 || (type[2] != 2 && type[2] != 3))
    {
        fclose(file);
        return false;
    }

    width = info[0] + info[1] * 256;
    height = info[2] + info[3] * 256;
    byteCount = info[4] / 8;

    if (byteCount != 3 && byteCount != 4) {
        fclose(file);
        return false;
    }

    long imageSize = width * height * byteCount;

    //allocate memory for image data
    data = new unsigned char[imageSize];

    //read in image data
    fread(data, sizeof(unsigned char), imageSize, file);

    //close file
    fclose(file);

    return true;
}
