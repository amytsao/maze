#ifndef _TGA_H_
#define _TGA_H_

class TgaImage
{
public:
    TgaImage()
    {
        data = (unsigned char*)0;
        width = 0;
        height = 0;
        byteCount = 0;
    }

    ~TgaImage() { delete[] data; data = 0; }

    bool loadTGA(const char* filename);

    int width;
    int height;
    unsigned char byteCount;
    unsigned char* data;
};

#endif