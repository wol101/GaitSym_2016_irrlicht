#ifndef APNGWRITER_H
#define APNGWRITER_H

#include <qglobal.h>
#include <vector>

struct PNGSignature;
struct IHDR;
struct IEND;
struct IDAT;

class APNGWriter
{
public:
    APNGWriter();

    static void WriteAPNG(const char *pathname, quint32 width, quint32 height, const unsigned char *rgb, bool deleteExisting);
    static int DecodePNG(const quint8 *data, quint32 length, PNGSignature *signature, IHDR *ihdr, std::vector<IDAT> *idatVector, IEND *iend);
    static quint32 PNGCRC32(const quint8 *data, size_t length);
};

#endif // APNGWRITER_H
