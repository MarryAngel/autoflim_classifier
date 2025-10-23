#include "ift.h"
#include<string.h>

/* 

Author: André Igor Nóbrega da Silva
Date:   16-11-2022
Brief:  Converts a MImages representing activations after convolutions with a bank of kernels to a seeds file.
        These seeds are the minutia detected at the point of maximum intensity.

*/

int main(int argc, char *argv[])
{
    timer *tstart;

    if (argc != 3)
      iftError("Usage: iftConvertMImageToSeeds P1 P2 P3\n"
	       "P1: input folder with mimages activations\n"
	       "P2: output folder with seeds files (-seeds.txt)\n",	       
	       "main");
    
    tstart = iftTic();
    void *trash = malloc(1);                 
    struct mallinfo info;   
    int MemDinInicial, MemDinFinal;
    free(trash); 
    info = mallinfo();
    MemDinInicial = info.uordblks;

    char *input_dir = argv[1];
    char *out_dir   = argv[2];
    iftMakeDir(out_dir);

    iftFileSet *fs   = iftLoadFileSetFromDirBySuffix(input_dir,".mimg", 1);
    int nimages      = fs->n;

    for (int i = 0; i < nimages; i++){
        char *basename = iftFilename(fs->files[i]->path,".mimg");
        
        // Load mimage
        iftMImage *image = iftReadMImage(fs->files[i]->path);
        iftImage  *base  = iftCreateImage(image->xsize, image->ysize, image->zsize);
        
        // output seeds file
        char filename[200];
        sprintf(filename, "%s/%s-seeds.txt", out_dir, basename);

        printf("Seed %d: %s\n", i, filename);
        

        float *max = iftAllocFloatArray(image->m);
        int *xmax  = iftAllocIntArray(image->m);
        int *ymax  = iftAllocIntArray(image->m);
        iftVoxel u;

        for (int b = 0; b < image->m; b++){
            max[b] = IFT_INFINITY_FLT_NEG;
            for (int i = 0; i < image->n; i++){
                if (image->val[i][b] > max[b]){
                    // max value
                    max[b] = image->val[i][b];

                    // coord of max value: mnt location
                    u = iftGetVoxelCoord(base, i);
                    xmax[b] = u.x;
                    ymax[b] = u.y; 
                }
            }
        }

        // Writing seeds file
        FILE *seeds_file = fopen(filename, "w");

        // header
        fprintf(seeds_file, "%lu %d %d\n", image->m, image->xsize, image->ysize);

        // writting mnt locations
        for (int b = 0; b < image->m; b++){
            fprintf(seeds_file, "%d %d %d %d\n", xmax[b], ymax[b], -1, 1);
        }

        fclose(seeds_file);
        iftDestroyMImage(&image);
        iftDestroyImage(&base);
        iftFree(max);
        iftFree(xmax);
        iftFree(ymax);
        iftFree(basename);
    }

    iftDestroyFileSet(&fs);
    printf("Done ... %s\n", iftFormattedTime(iftCompTime(tstart, iftToc())));

    /* ---------------------------------------------------------- */

    info = mallinfo();
    MemDinFinal = info.uordblks;
    if (MemDinInicial!=MemDinFinal)
        printf("\n\nDinamic memory was not completely deallocated (%d, %d)\n",
        MemDinInicial,MemDinFinal);   

    return (0);
}
