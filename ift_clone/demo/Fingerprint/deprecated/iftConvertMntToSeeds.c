#include "ift.h"
#include<string.h>

/* 

Author: André Igor Nóbrega da Silva
Date:   07-11-2022
Brief:  Converts every mnt (minutia) file from a given folder to seeds format accepted by FLIM.

*/

int main(int argc, char *argv[])
{
    timer *tstart;

    if (argc != 4)
      iftError("Usage: iftConvertMntToSeeds P1 P2 P3\n"
	       "P1: input folder with minutia files (.mnt)\n"
	       "P2: output folder with seeds files (-seeds.txt)\n"	       
	       "P3: output a list of individuals to use their position in the list as seed label (list_of_individuals.txt)",	       
	       "main");
    
    tstart = iftTic();
    void *trash = malloc(1);                 
    struct mallinfo info;   
    int MemDinInicial, MemDinFinal;
    free(trash); 
    info = mallinfo();
    MemDinInicial = info.uordblks;

    // Load mnt files from folder
    iftFileSet  *fs_seeds = iftLoadFileSetFromDirBySuffix(argv[1], ".mnt", 1);
    iftMakeDir(argv[2]);

    // Create a list of individuos 

    int *id    = iftAllocIntArray(fs_seeds->n);
    int *index = iftAllocIntArray(fs_seeds->n);
    char basename[200];
    for(int i = 0; i < fs_seeds->n; i++){
      FILE *input_file = fopen(fs_seeds->files[i]->path, "r");
      // Read first header of .mnt file. First line: basename
      fscanf(input_file, "%s", basename);
      // get individuo number from basename
      id[i]    = atoi(strtok(basename, "-"));
      index[i] = i;
    }
    // Sort the ids 
    iftBucketSort(id, index, fs_seeds->n, IFT_INCREASING);



    // Count the number of labels and create the list of individuals
    int nlabels=1;
    for(int i = 1; i < fs_seeds->n; i++){
      if (id[index[i]] != id[index[i-1]]){
	nlabels++;
      }
    }
    int *person = iftAllocIntArray(nlabels);
    person[0]   = id[index[0]];
    for(int i = 1, j = 1; i < fs_seeds->n; i++){
      if (id[index[i]] != id[index[i-1]]){
	person[j] = id[index[i]];
	j = j+1;
      }
    }
    
    iftFree(id);
    iftFree(index);
    
    // Save the list of individuals

    FILE *out_file = fopen(argv[3],"w");
    fprintf(out_file,"%d\n",nlabels);
    for (int j=0; j < nlabels; j++)
      fprintf(out_file,"%d\n",person[j]); 
    fclose(out_file);
    
    // Generate the labeled seed files

    char seeds_filename[200];
    for(int i = 0; i < fs_seeds->n; i++){
      FILE *input_file = fopen(fs_seeds->files[i]->path, "r");
      // Read first header of .mnt file. First line: basename
      fscanf(input_file, "%s", basename);
      // get individuo number from basename
      char *Basename = iftCopyString(basename);
      int id_number = atoi(strtok(basename, "-"));
      // assign a label from the list of individuals
      int label=IFT_NIL;
      for (int j=0; j < nlabels; j++)
	if (person[j]==id_number)
	  label=j+1;

      // opening output .mnt file with correct filename
      sprintf(seeds_filename, "%s/%s%s", argv[2], Basename, "-seeds.txt");
      FILE *output_file = fopen(seeds_filename, "w");
        
      // Second line of header: number of minutias and image size
      int n_mnts, x_size, y_size;
      fscanf(input_file, "%d %d %d", &n_mnts, &x_size, &y_size);

      // writing header of output file
      fprintf(output_file, "%d %d %d\n", n_mnts, y_size, x_size); // read as y_size x_size
        
      // writing every minutia to seeds file
      for(int j = 0; j <= n_mnts; j++){
	int x, y;
	float theta;
	fscanf(input_file, "%d %d %f", &x, &y, &theta);
	fprintf(output_file, "%d %d %d %d\n", x, y, -1, label);
      }
      iftFree(Basename);
      fclose(input_file);
      fclose(output_file);
    }

    iftFree(person);
    iftDestroyFileSet(&fs_seeds);

    printf("Done ... %s\n", iftFormattedTime(iftCompTime(tstart, iftToc())));

    /* ---------------------------------------------------------- */

    info = mallinfo();
    MemDinFinal = info.uordblks;
    if (MemDinInicial!=MemDinFinal)
        printf("\n\nDinamic memory was not completely deallocated (%d, %d)\n",
        MemDinInicial,MemDinFinal);   

    return (0);
}
