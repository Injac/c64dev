/* spriter.c: transforms a sprite GIF into ready-to-use assembler code
 *
 * Author:  Eli Wenig
 * Version: 0.8
 *
 */

#include <gif_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage ();
int in_array (int *, int);
const char *bin (int);
void world_is_a_fuck (int, int, int, int);
int DecodeHiResCharacterGraphics (GifFileType *);
int DecodeMultiColorCharacterGraphics (GifFileType *);
int DecodeHiResSpriteGraphics (GifFileType *, int);
int DecodeMultiColorSpriteGraphics (GifFileType *);

int main (int argc, char **argv)
{
    // inverse flag
    int rflag = 0, mflag = 0, cflag = 0, ch;
    while ((ch = getopt (argc, argv, "rmc")) != -1)
    {
        switch (ch)
        {
        case 'r':
            rflag = 1;
            break;
        case 'm':
            mflag = 1;
            break;
        case 'c':
            cflag = 1;
            break;
        default:
            // print error and exit
            usage ();
            return 1;
        }
    }

    // adjust args array post-getopt
    argc -= optind;
    argv += optind;

    if (argv[0] == NULL)
    {
        // print error and exit
        fputs ("File name missing\n", stderr);
        usage ();
        return 1;
    }

    char *fname = argv[0];

    // open the GIF file
    GifFileType *SpriteGif;
    SpriteGif = DGifOpenFileName (fname);

    // check for error
    if (SpriteGif == NULL)
    {
        // print error and exit
        PrintGifError ();
        exit (1);
    }

    if (DGifSlurp (SpriteGif) == GIF_ERROR)
    {
        // print error and exit
        PrintGifError ();
        exit (1);
    }

    int rc; // return code
    if (cflag && mflag)
    {
        rc = DecodeMultiColorCharacterGraphics (SpriteGif);
    }
    else if (cflag)
    {
        rc = DecodeHiResCharacterGraphics (SpriteGif);
    }
    else if (mflag)
    {
        rc = DecodeMultiColorSpriteGraphics (SpriteGif);
    }
    else
    {
        rc = DecodeHiResSpriteGraphics (SpriteGif, rflag);
    }

    // close the file object
    if (DGifCloseFile (SpriteGif) == GIF_ERROR)
    {
        // print error and exit
        PrintGifError ();
        return 1;
    }

    return rc;
}

int DecodeHiResCharacterGraphics (GifFileType *SpriteGif)
{
    // stub
    fputs ("ERROR: Not implemented.\n", stderr);
    return 1;
}

int DecodeMultiColorCharacterGraphics (GifFileType *SpriteGif)
{
    // character graphics
    int wwidth = 4; // wanted width
    if ((SpriteGif->SWidth % wwidth != 0) || (SpriteGif->SHeight % 21 != 0))
    {
        fprintf (stderr, "Error: Incorrect dimensions: %dx%d\n", SpriteGif->SWidth, SpriteGif->SHeight);
        return 1;
    }

    // iterate over the raster bits
    unsigned char *raster = SpriteGif->SavedImages->RasterBits;

    int wscale = SpriteGif->SWidth / wwidth;
    int hscale = SpriteGif->SHeight / 8;
    int i, j, m, h; // iterators
    int colors[4] = { -1, -1, -1, -1 }; // palette values to store
    int cur, k, l; // current color, array indices
    for (h = 0; h < hscale; h++)
    {
        for (m = 0; m < wscale; m++)
        {
            if (wscale > 1)
            {
                printf ("\n; character %x\n", m + h * 8);
            }
            for (i = 0; i < 32; i += 4)
            {
                for (j = 0; j < 4; j++)
                {
                    if (j % 4 == 0) fputs ("\n\t.byte %", stdout);
                    int index = (i * wscale + m * 4 + h * 8 * 4 * wscale) + j;
                    // world_is_a_fuck(wscale,index,4,8*hscale);
                    // fprintf(stderr,"index %d\n",index);
                    // usleep(100);
                    cur = raster[index];
                    k = in_array (colors, cur);
                    if (k == -1)
                    {
                        l = in_array (colors, -1);
                        if (l == -1)
                        {
                            fputs ("ERROR: more than 4 colors in the palette\n", stderr);
                            fputs ("Color cache: { ", stderr);
                            int z;
                            for (z = 0; z < 4; z++)
                            {
                                fprintf (stderr, "%d, ", colors[z]);
                            }
                            fputs ("}\n", stderr);
                            return 1;
                        }
                        colors[l] = cur;
                        printf (bin (l), stdout);
                    }
                    else
                    {
                        printf (bin (k), stdout);
                    }
                }
            }
        }
    }

    return 0;
}

int DecodeHiResSpriteGraphics (GifFileType *SpriteGif, int rflag)
{
    int wwidth = 24; // wanted width
    // make sure we have the correct number of pixels
    if ((SpriteGif->SWidth % wwidth != 0) && SpriteGif->SHeight != 21)
    {
        fprintf (stderr, "Error: Incorrect dimensions: %dx%d\n", SpriteGif->SWidth, SpriteGif->SHeight);
        return 1;
    }

    // iterate over the raster bits
    unsigned char *raster = SpriteGif->SavedImages->RasterBits;

    int wscale = SpriteGif->SWidth / wwidth;
    int i, j, row = 0; // iterators
    for (i = 0; i < 504 * wscale; i += 24)
    {
        printf ("\t; row %x\n", row++);
        fputs ("\t.byte %", stdout);
        for (j = 0; j < 8; j++)
            fputs ((raster[i + j] == 0) ? (rflag ? "1" : "0") : (rflag ? "0" : "1"), stdout);
        fputs ("\n\t.byte %", stdout);
        for (j = 8; j < 16; j++)
            fputs ((raster[i + j] == 0) ? (rflag ? "1" : "0") : (rflag ? "0" : "1"), stdout);
        fputs ("\n\t.byte %", stdout);
        for (j = 16; j < 24; j++)
            fputs ((raster[i + j] == 0) ? (rflag ? "1" : "0") : (rflag ? "0" : "1"), stdout);
        puts ("");
    }

    return 0;
}

int DecodeMultiColorSpriteGraphics (GifFileType *SpriteGif)
{
    int wwidth = 12; // wanted width
    // make sure we have the correct number of pixels
    if ((SpriteGif->SWidth % wwidth != 0) && SpriteGif->SHeight != 21)
    {
        fprintf (stderr, "Error: Incorrect dimensions: %dx%d\n", SpriteGif->SWidth, SpriteGif->SHeight);
        return 1;
    }

    // iterate over the raster bits
    unsigned char *raster = SpriteGif->SavedImages->RasterBits;

    int wscale = SpriteGif->SWidth / wwidth;
    int i, j, m = 0, row = 0; // iterators
    int colors[4] = { -1, -1, -1, -1 }; // palette values to store
    int cur, k, l; // current color, array indices
    for (m = 0; m < wscale; m++)
    {
        if (wscale > 1)
        {
            printf ("\n; sprite %x\n", m % 21);
        }
        for (i = 0; i < 252; i += 12)
        {
            printf ("\n\n\t; row %x", row++);
            for (j = 0; j < 12; j++)
            {
                if (j % 4 == 0) fputs ("\n\t.byte %", stdout);
                int index = (i * wscale + m * 12) + j;
                // world_is_a_fuck(wscale,index,12,21);
                // fprintf(stderr,"index %d\n",index);
                cur = raster[index];
                k = in_array (colors, cur);
                if (k == -1)
                {
                    l = in_array (colors, -1);
                    if (l == -1)
                    {
                        fputs ("ERROR: more than 4 colors in the palette\n", stderr);
                        fputs ("Color cache: { ", stderr);
                        int z;
                        for (z = 0; z < 4; z++)
                        {
                            fprintf (stderr, "%d, ", colors[z]);
                        }
                        fputs ("}\n", stderr);
                        return 1;
                    }
                    colors[l] = cur;
                    printf (bin (l), stdout);
                }
                else
                {
                    printf (bin (k), stdout);
                }
            }
        }

        fputs ("\n\t; padding byte\n\t.byte $00\n", stdout);
    }
    fputs ("\n", stdout);

    return 0;
}

void world_is_a_fuck (int a, int b, int w, int h)
{
    fprintf (stderr, "\033[H");
    int i, j;
    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w * a; j++)
        {
            if (b == (i * w * a) + j)
            {
                fputs ("O", stderr);
            }
            else
            {
                fputs ("o", stderr);
            }
        }
        fputs ("\n", stderr);
    }
    usleep (10000);
}

void usage ()
{
    // print usage message
    fputs ("Usage: ./spriter [-r] [-m] [-c] [filename]", stderr);
}

int in_array (int *haystack, int needle)
{
    int k = 3; // iterator
    do
    {
        if (haystack[k] == needle) return k;
    } while (--k >= 0);
    return -1;
}

const char *bin (int a)
{
    switch (a)
    {
    case 0:
        return "11";
    case 1:
        return "10";
    case 2:
        return "01";
    case 3:
        return "00";
    default:
        return "";
    }
}
