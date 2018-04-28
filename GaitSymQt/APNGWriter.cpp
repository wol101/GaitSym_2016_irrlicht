#include "APNGWriter.h"

#include <QImage>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QMessageBox>
#include <QtEndian>

#include <vector>

#pragma pack(1) // force structures to be packed

struct PNGSignature {
    quint8 name[8];
};
struct IHDR {
    quint32 chunkLength;
    quint8 chunkType[4];
    quint32 width;
    quint32 height;
    quint8 bitDepth;
    quint8 colourType;
    quint8 compressionMethod;
    quint8 filterMethod;
    quint8 interlaceMethod;
    quint32 crc;
};
struct IDAT {
    quint32 chunkLength;
    quint8 chunkType[4];
    const quint8 *chunkData;
    quint32 crc;
};
struct IEND {
    quint32 chunkLength;
    quint8 chunkType[4];
    quint32 crc;
};
struct tEXt {
    quint32 chunkLength;
    quint8 chunkType[4];
    quint8 keyword[8];
    quint8 nullSeparator;
    quint8 text[8];
    quint32 crc;
};
struct acTL {
    quint32 chunkLength;
    quint8 chunkType[4];
    quint32 numFrames;
    quint32 numPlays;
    quint32 crc;
};
struct fcTL {
    quint32 chunkLength;
    quint8 chunkType[4];
    quint32 sequenceNumber;
    quint32 width;
    quint32 height;
    quint32 xOffset;
    quint32 yOffset;
    quint16 delayNumerator;
    quint16 delayDenominator;
    quint8 disposeOp;
    quint8 blendOp;
    quint32 crc;
};
struct fdAT {
    quint32 chunkLength;
    quint8 chunkType[4];
    quint32 sequenceNumber;
    const quint8 *chunkData;
    quint32 crc;
};

#pragma options align=reset // back to the default packing

APNGWriter::APNGWriter()
{

}


// write an APNG file
void APNGWriter::WriteAPNG(const char *pathname, quint32 width, quint32 height, const unsigned char *rgb, bool deleteExisting)
{
    if (deleteExisting)
    {
        if (QFileInfo(pathname).exists())
        {
            if (QFile::remove(pathname) == false)
            {
                QMessageBox::warning(0, "Snapshot Error", QString("Could not delete '%1'\n").arg(pathname), "Click button to return to simulation");
                return;
            }
        }
    }

    QImage image(rgb, width, height, width * 3, QImage::Format_RGB888);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG"); // this saves the PNG file data to a QByteArray

    PNGSignature signature;
    IHDR ihdr;
    std::vector<IDAT> idatVector;
    IEND iend = {/*chunkLength*/ qToBigEndian(0u), /*chunkType*/ {'I','E','N','D'}, /*crc*/ 0};
    iend.crc = qToBigEndian(PNGCRC32(reinterpret_cast<const quint8 *>(&iend) + 4, sizeof(iend) - 8)); /*the CRC is taken of the data and the 4 chunk type letters, not the length*/

    DecodePNG(reinterpret_cast<const quint8 *>(ba.constData()), ba.size(), &signature, &ihdr, &idatVector, &iend);

    if (QFileInfo(pathname).exists() == false)
    {
        // create a new APNG file
        QFile file(pathname);
        file.open(QIODevice::WriteOnly);
        // signature
        file.write(reinterpret_cast<const char *>(&signature), sizeof(signature));

        // IHDR
        file.write(reinterpret_cast<const char *>(&ihdr), sizeof(ihdr));
        qDebug() << "ihdr.chunkLength = " << qFromBigEndian<quint32>(ihdr.chunkLength) << "\n"
                 << "ihdr.chunkType = " << QString(QByteArray((const char *)ihdr.chunkType, sizeof(ihdr.chunkType))) << "\n"
                 << "ihdr.width = " << qFromBigEndian<quint32>(ihdr.width) << "\n"
                 << "ihdr.height = " << qFromBigEndian<quint32>(ihdr.height) << "\n"
                 << "ihdr.bitDepth = " << ihdr.bitDepth << "\n"
                 << "ihdr.colourType = " << ihdr.colourType << "\n"
                 << "ihdr.compressionMethod = " << ihdr.compressionMethod << "\n"
                 << "ihdr.filterMethod = " << ihdr.filterMethod << "\n"
                 << "ihdr.interlaceMethod = " << ihdr.interlaceMethod << "\n"
                 << "ihdr.crc = " << QString("0x%1").arg(qFromBigEndian<quint32>(ihdr.crc), 8, 16, QLatin1Char( '0' )) << "\n";

        // tEXt
        tEXt text = {
            /*chunkLength*/ qToBigEndian(quint32(sizeof(text) - 12)),
            /*chunkType*/ {'t','E','X','t'},
            /*keyword*/ {'l','a','s','t','_','s','e','q'},
            /*nullSeparator*/ 0,
            /*text*/ {'0','0','0','0','0','0','0','1'},
            /*crc*/ 0
        };
        text.crc = qToBigEndian(PNGCRC32(reinterpret_cast<const quint8 *>(&text) + 4, sizeof(text) - 8)); /*the CRC is taken of the data and the 4 chunk type letters, not the length*/

        file.write(reinterpret_cast<const char *>(&text), sizeof(text));

        // acTL
        acTL actl = {
            /*chunkLength*/ qToBigEndian(quint32(sizeof(actl) - 12)),
            /*chunkType*/ {'a','c','T','L'},
            /*numFrames*/ qToBigEndian(1u),
            /*numPlays*/ qToBigEndian(0u),
            /*crc*/ 0
        };
        actl.crc = qToBigEndian(PNGCRC32(reinterpret_cast<const quint8 *>(&actl) + 4, sizeof(actl) - 8)); /*the CRC is taken of the data and the 4 chunk type letters, not the length*/
        file.write(reinterpret_cast<const char *>(&actl), sizeof(actl));

        // fcTL
        fcTL fctl = {
            /*fctl.chunkLength*/ qToBigEndian(quint32(sizeof(fctl) - 12)),
            /*fctl.chunkType*/ {'f','c','T','L'},
            /*fctl.sequenceNumber*/ qToBigEndian(0u),
            /*fctl.width*/ qToBigEndian(width),
            /*fctl.height*/ qToBigEndian(height),
            /*fctl.xOffset*/ qToBigEndian(0u),
            /*fctl.yOffset*/ qToBigEndian(0u),
            /*fctl.delayNumerator*/ qToBigEndian(quint16(40)),
            /*fctl.delayDenominator*/ qToBigEndian(quint16(1000)), // 1000 makes the numerator work in ms
            /*fctl.disposeOp*/ 0,
            /*fctl.blendOp*/ 0,
            /*fctl.crc*/ 0
        };
        fctl.crc = qToBigEndian(PNGCRC32(reinterpret_cast<const quint8 *>(&fctl) + 4, sizeof(fctl) - 8));
        file.write(reinterpret_cast<const char *>(&fctl), sizeof(fctl));
        qDebug() << "fctl.chunkLength = " << qFromBigEndian<quint32>(fctl.chunkLength) << "\n"
        << "fctl.chunkType = " << QString(QByteArray((const char *)fctl.chunkType, sizeof(fctl.chunkType))) << "\n"
        << "fctl.sequenceNumber = " << qFromBigEndian<quint32>(fctl.sequenceNumber) << "\n"
        << "fctl.width = " << qFromBigEndian<quint32>(fctl.width) << "\n"
        << "fctl.height = " << qFromBigEndian<quint32>(fctl.height) << "\n"
        << "fctl.xOffset = " << qFromBigEndian<quint32>(fctl.xOffset) << "\n"
        << "fctl.yOffset = " << qFromBigEndian<quint32>(fctl.yOffset) << "\n"
        << "fctl.delayNumerator = " << qFromBigEndian<quint16>(fctl.delayNumerator) << "\n"
        << "fctl.delayDenominator = " << qFromBigEndian<quint16>(fctl.delayDenominator) << "\n"
        << "fctl.disposeOp = " << fctl.disposeOp << "\n"
        << "fctl.blendOp = " << fctl.blendOp << "\n"
        << "fctl.crc = " << QString("0x%1").arg(qFromBigEndian<quint32>(fctl.crc), 8, 16, QLatin1Char( '0' )) << "\n";


        // because this is the first frame we just use the IDAT without converting it to fdAT
        IDAT idat;
        for (unsigned int i = 0; i < idatVector.size(); i++)
        {
            idat = idatVector[i];
            file.write(reinterpret_cast<const char *>(&idat), sizeof(idat.chunkLength) + sizeof(idat.chunkType));
            file.write(reinterpret_cast<const char *>(idat.chunkData), qFromBigEndian<quint32>(idat.chunkLength));
            file.write(reinterpret_cast<const char *>(&idat.crc), sizeof(idat.crc));
        }

        // IEND
        file.write(reinterpret_cast<const char *>(&iend), sizeof(iend));

        file.close();
    }
    else
    {
        // open existing APNG file
        QFile file(pathname);
        file.open(QIODevice::ReadWrite);

        // read signature
        PNGSignature signatureFile;
        file.read(reinterpret_cast<char *>(&signatureFile), sizeof(signatureFile));
        if (memcmp(&signatureFile, &signature, sizeof(signatureFile)))
        {
            QMessageBox::warning(0, "Movie Error", QString("PNG signature does not match '%1'\n").arg(pathname), "Click button to return to simulation");
            return;
        }

        // read IHDR
        IHDR ihdrFile;
        file.read(reinterpret_cast<char *>(&ihdrFile), sizeof(ihdrFile));
        if (memcmp(&ihdrFile, &ihdr, sizeof(ihdrFile)))
        {
            QMessageBox::warning(0, "Movie Error", QString("PNG IHDR chunk does not match '%1'\n").arg(pathname), "Click button to return to simulation");
            return;
        }

        // read tEXt
        tEXt text;
        qint64 textPos = file.pos();
        file.read(reinterpret_cast<char *>(&text), sizeof(text));
        if (memcmp(text.chunkType, "tEXt", sizeof(text.chunkType)) || memcmp(text.keyword, "last_seq", sizeof(text.keyword)))
        {
            QMessageBox::warning(0, "Movie Error", QString("PNG tEXt chunk has wrong format '%1'\n").arg(pathname), "Click button to return to simulation");
            return;
        }
        bool ok;
        quint32 lastSequenceNumber = QString(QByteArray((const char *)text.text, sizeof(text.text))).toUInt(&ok, 16);

        // read the acTL
        acTL actl;
        qint64 actlPos = file.pos();
        file.read(reinterpret_cast<char *>(&actl), sizeof(actl));
        if (memcmp(actl.chunkType, "acTL", sizeof(actl.chunkType)))
        {
            QMessageBox::warning(0, "Movie Error", QString("PNG acTL chunk has wrong type '%1'\n").arg(pathname), "Click button to return to simulation");
            return;
        }

        // write the new acTL
        file.seek(actlPos);
        actl.numFrames = qToBigEndian(qFromBigEndian<quint32>(actl.numFrames) + 1);
        actl.crc = qToBigEndian(PNGCRC32(reinterpret_cast<const quint8 *>(&actl) + 4, sizeof(actl) - 8)); /*the CRC is taken of the data and the 4 chunk type letters, not the length*/
        file.write(reinterpret_cast<const char *>(&actl), sizeof(actl));

        // now append the new frame data
        file.seek(file.size() - sizeof(IEND));

        // fcTL
        fcTL fctl = {
            /*fctl.chunkLength*/ qToBigEndian(quint32(sizeof(fctl) - 12)),
            /*fctl.chunkType*/ {'f','c','T','L'},
            /*fctl.sequenceNumber*/ qToBigEndian(lastSequenceNumber++),
            /*fctl.width*/ qToBigEndian(width),
            /*fctl.height*/ qToBigEndian(height),
            /*fctl.xOffset*/ qToBigEndian(0u),
            /*fctl.yOffset*/ qToBigEndian(0u),
            /*fctl.delayNumerator*/ qToBigEndian(quint16(40)),
            /*fctl.delayDenominator*/ qToBigEndian(quint16(1000)), // 1000 makes the numerator work in ms
            /*fctl.disposeOp*/ 0,
            /*fctl.blendOp*/ 0,
            /*fctl.crc*/ 0
        };
        fctl.crc = qToBigEndian(PNGCRC32(reinterpret_cast<const quint8 *>(&fctl) + 4, sizeof(fctl) - 8));
        file.write(reinterpret_cast<const char *>(&fctl), sizeof(fctl));
        qDebug() << "fctl.chunkLength = " << qFromBigEndian<quint32>(fctl.chunkLength) << "\n"
        << "fctl.chunkType = " << QString(QByteArray((const char *)fctl.chunkType, sizeof(fctl.chunkType))) << "\n"
        << "fctl.sequenceNumber = " << qFromBigEndian<quint32>(fctl.sequenceNumber) << "\n"
        << "fctl.width = " << qFromBigEndian<quint32>(fctl.width) << "\n"
        << "fctl.height = " << qFromBigEndian<quint32>(fctl.height) << "\n"
        << "fctl.xOffset = " << qFromBigEndian<quint32>(fctl.xOffset) << "\n"
        << "fctl.yOffset = " << qFromBigEndian<quint32>(fctl.yOffset) << "\n"
        << "fctl.delayNumerator = " << qFromBigEndian<quint16>(fctl.delayNumerator) << "\n"
        << "fctl.delayDenominator = " << qFromBigEndian<quint16>(fctl.delayDenominator) << "\n"
        << "fctl.disposeOp = " << fctl.disposeOp << "\n"
        << "fctl.blendOp = " << fctl.blendOp << "\n"
        << "fctl.crc = " << QString("0x%1").arg(qFromBigEndian<quint32>(fctl.crc), 8, 16, QLatin1Char( '0' )) << "\n";


        // because this is not the first frame we must convert the IDAT to fdAT
        IDAT idat;
        for (unsigned int i = 0; i < idatVector.size(); i++)
        {
            idat = idatVector[i];
            int fdatSize = qFromBigEndian<quint32>(idat.chunkLength) + 16;
            quint8 *fdatData = new quint8[fdatSize];
            *reinterpret_cast<quint32 *>(&fdatData[0]) = qToBigEndian(qFromBigEndian<quint32>(idat.chunkLength) + 4);
            memcpy(&fdatData[4], "fdAT", 4);
            *reinterpret_cast<quint32 *>(&fdatData[8]) = qToBigEndian(lastSequenceNumber++);
            memcpy(&fdatData[12], idat.chunkData, qFromBigEndian<quint32>(idat.chunkLength));
            quint32 crc = qToBigEndian(PNGCRC32(reinterpret_cast<const quint8 *>(fdatData) + 4, fdatSize - 8));
            memcpy(&fdatData[fdatSize - 4], &crc, 4);
            file.write(reinterpret_cast<const char *>(fdatData), fdatSize);
            delete [] fdatData;
        }

        // IEND
        file.write(reinterpret_cast<const char *>(&iend), sizeof(iend));

        // write the new tEXt
        file.seek(textPos);
        memcpy(&text.text, QString("%1").arg(lastSequenceNumber, 8, 16, QLatin1Char( '0' )).toUtf8().constData(), sizeof(text.text));
        text.crc = qToBigEndian(PNGCRC32(reinterpret_cast<const quint8 *>(&text) + 4, sizeof(text) - 8)); /*the CRC is taken of the data and the 4 chunk type letters, not the length*/
        file.write(reinterpret_cast<const char *>(&text), sizeof(text));

        file.close();

    }
}

int APNGWriter::DecodePNG(const quint8 *data, quint32 length, PNGSignature *signature, IHDR *ihdr, std::vector<IDAT> *idatVector, IEND *iend)
{
    quint32 index = 0;
    quint32 chunkLength;
    quint8 chunkType[4];
    IDAT idat;
    // signature
    if (index + sizeof(PNGSignature) > length) return __LINE__;
    memcpy(signature, &data[index], sizeof(PNGSignature));
    index += sizeof(PNGSignature);
    // header chunk
    if (index + sizeof(IHDR) > length) return __LINE__;
    memcpy(ihdr, &data[index], sizeof(IHDR));
    index += sizeof(IHDR);
    // look for the data chunk and the end chunk
    while (index < length)
    {
       chunkLength = *reinterpret_cast<const quint32 *>(&data[index]);
       index += sizeof(quint32);
       memcpy(chunkType, &data[index], sizeof(chunkType));
       index += sizeof(chunkType);
       if (memcmp(chunkType, "IDAT", sizeof(chunkType)) == 0)
       {
           idat.chunkLength = chunkLength;
           memcpy(&idat.chunkType, &chunkType, sizeof(idat.chunkType));
           idat.chunkData = &data[index];
           index += qFromBigEndian<quint32>(chunkLength);
           idat.crc = *reinterpret_cast<const quint32 *>(&data[index]);
           idatVector->push_back(idat);
           index += 4;
       }
       else if (memcmp(chunkType, "IEND", sizeof(chunkType)) == 0)
       {
           memcpy(iend, &data[index], sizeof(IEND));
           break;
       }
       else
       {
           index += qFromBigEndian<quint32>(chunkLength) + 4; // this includes skipping over the crc
       }
    }
    return 0;
}

/*Return the CRC of the bytes buf[0..len-1].*/
quint32 APNGWriter::PNGCRC32(const quint8 *data, size_t length)
{
    const quint32 crc32Table[256] =
    {
                 0u, 1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
         249268274u, 2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
         498536548u, 1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
         325883990u, 1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
         997073096u, 1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
         901097722u, 1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
         651767980u, 1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
         671266974u, 1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
        1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
        2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
        1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
        1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
        1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
        1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
        1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
        1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
        3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
        3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
        4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
        4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
        3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
        3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
        3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
        3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
        2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
        2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
        2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
        2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
        2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
        2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
        3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
        3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
    };
    quint32 r = 0xffffffffu;
    size_t i;
    for (i = 0; i < length; ++i)
    {
        r = crc32Table[(r ^ data[i]) & 0xff] ^ (r >> 8);
    }
    return r ^ 0xffffffffu;
}

