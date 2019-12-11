#ifndef LAUTIFF2PDFOBJECT_H
#define LAUTIFF2PDFOBJECT_H

#include <QObject>

#ifndef Q_OS_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#endif

namespace libtiff
{
#include "tiffconf.h"
#include "tiffio.h"
}

#ifndef HAVE_GETOPT
extern int getopt(int, char **, char *);
#endif

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS	0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE	1
#endif

#define TIFF2PDF_MODULE "tiff2pdf"

#define PS_UNIT_SIZE	72.0F

#ifdef Q_OS_WIN
#define snprintf _snprintf
#endif

/* This type is of PDF color spaces. */
typedef enum {
    T2P_CS_BILEVEL = 0x01,	/* Bilevel, black and white */
    T2P_CS_GRAY = 0x02,	/* Single channel */
    T2P_CS_RGB = 0x04,	/* Three channel tristimulus RGB */
    T2P_CS_CMYK = 0x08,	/* Four channel CMYK print inkset */
    T2P_CS_LAB = 0x10,	/* Three channel L*a*b* color space */
    T2P_CS_PALETTE = 0x1000,/* One of the above with a color map */
    T2P_CS_CALGRAY = 0x20,	/* Calibrated single channel */
    T2P_CS_CALRGB = 0x40,	/* Calibrated three channel tristimulus RGB */
    T2P_CS_ICCBASED = 0x80	/* ICC profile color specification */
} t2p_cs_t;

/* This type is of PDF compression types.  */
typedef enum {
    T2P_COMPRESS_NONE = 0x00
#ifdef CCITT_SUPPORT
    , T2P_COMPRESS_G4 = 0x01
#endif
#if defined(JPEG_SUPPORT) || defined(OJPEG_SUPPORT)
    , T2P_COMPRESS_JPEG = 0x02
#endif
#ifdef ZIP_SUPPORT
    , T2P_COMPRESS_ZIP = 0x04
#endif
} t2p_compress_t;

/* This type is whether TIFF image data can be used in PDF without transcoding. */
typedef enum {
    T2P_TRANSCODE_RAW = 0x01, /* The raw data from the input can be used without recompressing */
    T2P_TRANSCODE_ENCODE = 0x02 /* The data from the input is perhaps unencoded and reencoded */
} t2p_transcode_t;

/* This type is of information about the data samples of the input image. */
typedef enum {
    T2P_SAMPLE_NOTHING = 0x0000, /* The unencoded samples are normal for the output colorspace */
    T2P_SAMPLE_ABGR_TO_RGB = 0x0001, /* The unencoded samples are the result of ReadRGBAImage */
    T2P_SAMPLE_RGBA_TO_RGB = 0x0002, /* The unencoded samples are contiguous RGBA */
    T2P_SAMPLE_RGBAA_TO_RGB = 0x0004, /* The unencoded samples are RGBA with premultiplied alpha */
    T2P_SAMPLE_YCBCR_TO_RGB = 0x0008,
    T2P_SAMPLE_YCBCR_TO_LAB = 0x0010,
    T2P_SAMPLE_REALIZE_PALETTE = 0x0020, /* The unencoded samples are indexes into the color map */
    T2P_SAMPLE_SIGNED_TO_UNSIGNED = 0x0040, /* The unencoded samples are signed instead of unsignd */
    T2P_SAMPLE_LAB_SIGNED_TO_UNSIGNED = 0x0040, /* The L*a*b* samples have a* and b* signed */
    T2P_SAMPLE_PLANAR_SEPARATE_TO_CONTIG = 0x0100 /* The unencoded samples are separate instead of contiguous */
} t2p_sample_t;

/* This type is of error status of the T2P struct. */
typedef enum {
    T2P_ERR_OK = 0, /* This is the value of t2p->t2p_error when there is no error */
    T2P_ERR_ERROR = 1 /* This is the value of t2p->t2p_error when there was an error */
} t2p_err_t;

/* This struct defines a logical page of a TIFF. */
typedef struct {
    libtiff::tdir_t page_directory;
    libtiff::uint32 page_number;
    libtiff::ttile_t page_tilecount;
    libtiff::uint32 page_extra;
} T2P_PAGE;

/* This struct defines a PDF rectangle's coordinates. */
typedef struct {
    float x1;
    float y1;
    float x2;
    float y2;
    float mat[9];
} T2P_BOX;

/* This struct defines a tile of a PDF.  */
typedef struct {
    T2P_BOX tile_box;
} T2P_TILE;

/* This struct defines information about the tiles on a PDF page. */
typedef struct {
    libtiff::ttile_t tiles_tilecount;
    libtiff::uint32 tiles_tilewidth;
    libtiff::uint32 tiles_tilelength;
    libtiff::uint32 tiles_tilecountx;
    libtiff::uint32 tiles_tilecounty;
    libtiff::uint32 tiles_edgetilewidth;
    libtiff::uint32 tiles_edgetilelength;
    T2P_TILE *tiles_tiles;
} T2P_TILES;

/* This struct is the context of a function to generate PDF from a TIFF. */
typedef struct {
    t2p_err_t t2p_error;
    T2P_PAGE *tiff_pages;
    T2P_TILES *tiff_tiles;
    libtiff::tdir_t tiff_pagecount;
    libtiff::uint16 tiff_compression;
    libtiff::uint16 tiff_photometric;
    libtiff::uint16 tiff_fillorder;
    libtiff::uint16 tiff_bitspersample;
    libtiff::uint16 tiff_samplesperpixel;
    libtiff::uint16 tiff_planar;
    libtiff::uint32 tiff_width;
    libtiff::uint32 tiff_length;
    float tiff_xres;
    float tiff_yres;
    libtiff::uint16 tiff_orientation;
    libtiff::toff_t tiff_dataoffset;
    libtiff::tsize_t tiff_datasize;
    libtiff::uint16 tiff_resunit;
    libtiff::uint16 pdf_centimeters;
    libtiff::uint16 pdf_overrideres;
    libtiff::uint16 pdf_overridepagesize;
    float pdf_defaultxres;
    float pdf_defaultyres;
    float pdf_xres;
    float pdf_yres;
    float pdf_defaultpagewidth;
    float pdf_defaultpagelength;
    float pdf_pagewidth;
    float pdf_pagelength;
    float pdf_imagewidth;
    float pdf_imagelength;
    int pdf_image_fillpage; /* 0 (default: no scaling, 1:scale imagesize to pagesize */
    T2P_BOX pdf_mediabox;
    T2P_BOX pdf_imagebox;
    libtiff::uint16 pdf_majorversion;
    libtiff::uint16 pdf_minorversion;
    libtiff::uint32 pdf_catalog;
    libtiff::uint32 pdf_pages;
    libtiff::uint32 pdf_info;
    libtiff::uint32 pdf_palettecs;
    libtiff::uint16 pdf_fitwindow;
    libtiff::uint32 pdf_startxref;
#define TIFF2PDF_FILEID_SIZE 33
    char pdf_fileid[TIFF2PDF_FILEID_SIZE];
#define TIFF2PDF_DATETIME_SIZE 17
    char pdf_datetime[TIFF2PDF_DATETIME_SIZE];
#define TIFF2PDF_CREATOR_SIZE 512
    char pdf_creator[TIFF2PDF_CREATOR_SIZE];
#define TIFF2PDF_AUTHOR_SIZE 512
    char pdf_author[TIFF2PDF_AUTHOR_SIZE];
#define TIFF2PDF_TITLE_SIZE 512
    char pdf_title[TIFF2PDF_TITLE_SIZE];
#define TIFF2PDF_SUBJECT_SIZE 512
    char pdf_subject[TIFF2PDF_SUBJECT_SIZE];
#define TIFF2PDF_KEYWORDS_SIZE 512
    char pdf_keywords[TIFF2PDF_KEYWORDS_SIZE];
    t2p_cs_t pdf_colorspace;
    libtiff::uint16 pdf_colorspace_invert;
    libtiff::uint16 pdf_switchdecode;
    libtiff::uint16 pdf_palettesize;
    unsigned char *pdf_palette;
    int pdf_labrange[4];
    t2p_compress_t pdf_defaultcompression;
    libtiff::uint16 pdf_defaultcompressionquality;
    t2p_compress_t pdf_compression;
    libtiff::uint16 pdf_compressionquality;
    libtiff::uint16 pdf_nopassthrough;
    t2p_transcode_t pdf_transcode;
    t2p_sample_t pdf_sample;
    libtiff::uint32 *pdf_xrefoffsets;
    libtiff::uint32 pdf_xrefcount;
    libtiff::tdir_t pdf_page;
#ifdef OJPEG_SUPPORT
    libtiff::tdata_t pdf_ojpegdata;
    libtiff::uint32 pdf_ojpegdatalength;
    libtiff::uint32 pdf_ojpegiflength;
#endif
    float tiff_whitechromaticities[2];
    float tiff_primarychromaticities[6];
    float tiff_referenceblackwhite[2];
    float *tiff_transferfunction[3];
    int pdf_image_interpolate;	/* 0 (default) : do not interpolate,
                       1 : interpolate */
    libtiff::uint16 tiff_transferfunctioncount;
    libtiff::uint32 pdf_icccs;
    libtiff::uint32 tiff_iccprofilelength;
    libtiff::tdata_t tiff_iccprofile;

    /* fields for custom read/write procedures */
    FILE *outputfile;
    int outputdisable;
    libtiff::tsize_t outputwritten;
} T2P;

/* These functions are called by main. */

int tiff2pdf_match_paper_size(float *, float *, char *);

/* These functions are used to generate a PDF from a TIFF. */

#ifdef __cplusplus
extern "C" {
#endif

T2P *t2p_init(void);
void t2p_validate(T2P *);
libtiff::tsize_t t2p_write_pdf(T2P *, libtiff::TIFF *, libtiff::TIFF *);
void t2p_free(T2P *);

#ifdef __cplusplus
}
#endif

inline t2p_cs_t operator|(t2p_cs_t a, t2p_cs_t b)
{
    return static_cast<t2p_cs_t>(static_cast<int>(a) | static_cast<int>(b));
}
inline t2p_cs_t operator|=(const t2p_cs_t &lhs, const t2p_cs_t &rhs)
{
    return static_cast<t2p_cs_t>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
inline t2p_cs_t operator^(t2p_cs_t a, t2p_cs_t b)
{
    return static_cast<t2p_cs_t>(static_cast<int>(a) ^ static_cast<int>(b));
}
inline t2p_cs_t operator^=(const t2p_cs_t &lhs, const t2p_cs_t &rhs)
{
    return static_cast<t2p_cs_t>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}
inline t2p_sample_t operator|(t2p_sample_t a, t2p_sample_t b)
{
    return static_cast<t2p_sample_t>(static_cast<int>(a) | static_cast<int>(b));
}

void t2p_read_tiff_init(T2P *, libtiff::TIFF *);
int t2p_cmp_t2p_page(const void *, const void *);
void t2p_read_tiff_data(T2P *, libtiff::TIFF *);
void t2p_read_tiff_size(T2P *, libtiff::TIFF *);
void t2p_read_tiff_size_tile(T2P *, libtiff::TIFF *, libtiff::ttile_t);
int t2p_tile_is_right_edge(T2P_TILES, libtiff::ttile_t);
int t2p_tile_is_bottom_edge(T2P_TILES, libtiff::ttile_t);
int t2p_tile_is_edge(T2P_TILES, libtiff::ttile_t);
int t2p_tile_is_corner_edge(T2P_TILES, libtiff::ttile_t);
libtiff::tsize_t t2p_readwrite_pdf_image(T2P *, libtiff::TIFF *, libtiff::TIFF *);
libtiff::tsize_t t2p_readwrite_pdf_image_tile(T2P *, libtiff::TIFF *, libtiff::TIFF *, libtiff::ttile_t);
#ifdef OJPEG_SUPPORT
int t2p_process_ojpeg_tables(T2P *, libtiff::TIFF *);
#endif
#ifdef JPEG_SUPPORT
int t2p_process_jpeg_strip(unsigned char *, libtiff::tsize_t *, unsigned char *, libtiff::tsize_t *, libtiff::tstrip_t, libtiff::uint32);
#endif
void t2p_tile_collapse_left(libtiff::tdata_t, libtiff::tsize_t, libtiff::uint32, libtiff::uint32, libtiff::uint32);
void t2p_write_advance_directory(T2P *, libtiff::TIFF *);
libtiff::tsize_t t2p_sample_planar_separate_to_contig(T2P *, unsigned char *, unsigned char *, libtiff::tsize_t);
libtiff::tsize_t t2p_sample_realize_palette(T2P *, unsigned char *);
libtiff::tsize_t t2p_sample_abgr_to_rgb(libtiff::tdata_t, libtiff::uint32);
libtiff::tsize_t t2p_sample_rgba_to_rgb(libtiff::tdata_t, libtiff::uint32);
libtiff::tsize_t t2p_sample_rgbaa_to_rgb(libtiff::tdata_t, libtiff::uint32);
libtiff::tsize_t t2p_sample_lab_signed_to_unsigned(libtiff::tdata_t, libtiff::uint32);
libtiff::tsize_t t2p_write_pdf_header(T2P *, libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_obj_start(libtiff::uint32, libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_obj_end(libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_name(unsigned char *, libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_string(char *, libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_stream(libtiff::tdata_t, libtiff::tsize_t, libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_stream_start(libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_stream_end(libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_stream_dict(libtiff::tsize_t, libtiff::uint32,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_stream_dict_start(libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_stream_dict_end(libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_stream_length(libtiff::tsize_t,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_catalog(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_info(T2P *,  libtiff::TIFF *,  libtiff::TIFF *);
void t2p_pdf_currenttime(T2P *);
void t2p_pdf_tifftime(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_pages(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_page(libtiff::uint32, T2P *,  libtiff::TIFF *);
void t2p_compose_pdf_page(T2P *);
void t2p_compose_pdf_page_orient(T2P_BOX *, libtiff::uint16);
void t2p_compose_pdf_page_orient_flip(T2P_BOX *, libtiff::uint16);
libtiff::tsize_t t2p_write_pdf_page_content(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xobject_stream_dict(libtiff::ttile_t, T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xobject_cs(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_transfer(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_transfer_dict(T2P *,  libtiff::TIFF *, libtiff::uint16);
libtiff::tsize_t t2p_write_pdf_transfer_stream(T2P *,  libtiff::TIFF *, libtiff::uint16);
libtiff::tsize_t t2p_write_pdf_xobject_calcs(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xobject_icccs(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xobject_icccs_dict(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xobject_icccs_stream(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xobject_cs_stream(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xobject_decode(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xobject_stream_filter(libtiff::ttile_t, T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_xreftable(T2P *,  libtiff::TIFF *);
libtiff::tsize_t t2p_write_pdf_trailer(T2P *,  libtiff::TIFF *);

class LAUTiff2PdfObject : public QObject
{
    Q_OBJECT

public:
    explicit LAUTiff2PdfObject(QString inputFileString, QString outputFileString, QObject *parent = nullptr);
    ~LAUTiff2PdfObject()
    {
        ;
    }

signals:

public slots:

};

#endif // LAUTIFF2PDFOBJECT_H
