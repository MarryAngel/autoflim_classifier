#include "ift.h"

/*
iftMergeSequences ~/msc/2_datasets/brats_2023_std/t2f/ \
 ~/msc/2_datasets/brats_2023_std/t1c/ \
 ~/msc/2_datasets/brats_2023_std/merged/
*/

int main(int argc, char **argv) {
  timer *tstart;
  int memory_start, memory_end;
  memory_start = iftMemoryUsed();
  tstart = iftTic();

  if (argc != 4) {
    iftError(
      "Usage: iftCA <P1> <P2> <P3> \n"
      // t1c and merged folder must be in the same directory of the t2f folder
      "P1: t2f folder.\n"
      "P2: t1c folder.\n"
      "P3: merged folder.\n",
      "main"
    );
  }

  iftFileSet *fs = iftLoadFileSetFromDirBySuffix(argv[1], ".nii.gz", 1);
  iftMakeDir(argv[3]);
  char t2f_path[1024], t1c_path[1024], merged_path[1024],
        t2f_std_folder[512], t1c_std_folder[512],
        t2f_std_path[1024], t1c_std_path[1024];
  if (argv[1][strlen(argv[1])-1] == '/') {
    char temp[256]; // Or appropriate size
    strcpy(temp, argv[1]);
    temp[strlen(temp)-1] = '\0'; // Remove the last character
    sprintf(t2f_std_folder, "%s_bstd", temp);
  } else {
    sprintf(t2f_std_folder, "%s_bstd", argv[1]);
  }
  iftMakeDir(t2f_std_folder);
  if (argv[2][strlen(argv[2])-1] == '/') {
    char temp[256]; // Or appropriate size
    strcpy(temp, argv[2]);
    temp[strlen(temp)-1] = '\0'; // Remove the last character
    sprintf(t1c_std_folder, "%s_bstd", temp);
  } else {
    sprintf(t1c_std_folder, "%s_bstd", argv[2]);
  }
  iftMakeDir(t1c_std_folder);

  for (size_t i=0; i < fs->n; i++) {
    char *filename = iftFilename(fs->files[i]->path, "-t2f.nii.gz");
    sprintf(t2f_path, "%s/%s-t2f.nii.gz", argv[1], filename);
    sprintf(t1c_path, "%s/%s-t1c.nii.gz", argv[2], filename);
    sprintf(t2f_std_path, "%s/%s-t2f.mimg", t2f_std_folder, filename);
    sprintf(t1c_std_path, "%s/%s-t1c.mimg", t1c_std_folder, filename);
    sprintf(merged_path, "%s/%s-merged.mimg", argv[3], filename);

    printf("[INFO] Processing %s\n", t2f_path);
    printf("[INFO] Processing %s\n", t1c_path);

    iftImage *t2f_img = iftReadImageByExt(t2f_path);
    iftImage *t1c_img = iftReadImageByExt(t1c_path);
    iftMImage *t2f_img_std = iftCreateMImage(
      t2f_img->xsize, t2f_img->ysize, t2f_img->zsize, 1
    );
    iftMImage *t1c_img_std = iftCreateMImage(
      t1c_img->xsize, t1c_img->ysize, t1c_img->zsize, 1
    );
    iftMImage *merged = iftCreateMImage(
      t2f_img->xsize, t2f_img->ysize, t2f_img->zsize, 2
    );

    float t2f_mean = 0.0f, t2f_n = 0.0f, t2f_std = 0.0f,
          t1c_mean = 0.0f, t1c_std = 0.0f, t1c_n = 0.0f;
    #pragma omp parallel for reduction(+:t2f_mean,t2f_n,t1c_mean,t1c_n)
    for (size_t p=0; p < t2f_img->n; p++) {
      t2f_mean += (float) t2f_img->val[p];
      if (t2f_img->val[p] != 0) {
        t2f_n += 1.0;
      }
      t1c_mean += (float) t1c_img->val[p];
      if (t1c_img->val[p] != 0) {
        t1c_n += 1.0;
      }
    }
    t2f_mean /= t2f_n;
    t1c_mean /= t1c_n;
    #pragma omp parallel for reduction(+:t2f_std,t1c_std)
    for (size_t p=0; p < t2f_img->n; p++) {
      if (t2f_img->val[p] != 0) {
        t2f_std += powf((float)(t2f_img->val[p]) - t2f_mean, 2);
      }
      if (t1c_img->val[p] != 0) {
        t1c_std += powf((float)(t1c_img->val[p]) - t1c_mean, 2);
      }
    }
    t2f_std = sqrtf(t2f_std / t2f_n);
    t1c_std = sqrtf(t1c_std / t1c_n);


    #pragma omp parallel for
    // Normalize here (Save as MIMG both normalized images and merged ones)
    for (size_t p=0; p < t2f_img->n; p++) {
      merged->val[p][0] = ((float) (t2f_img->val[p]) - t2f_mean) / t2f_std;
      t2f_img_std->val[p][0] = ((float) (t2f_img->val[p]) - t2f_mean) / t2f_std;
      merged->val[p][1] = ((float) (t1c_img->val[p]) - t1c_mean) / t1c_std;
      t1c_img_std->val[p][0] = ((float) (t1c_img->val[p]) - t1c_mean) / t1c_std;
    }

    iftWriteMImage(merged, merged_path);
    iftWriteMImage(t2f_img_std, t2f_std_path);
    iftWriteMImage(t1c_img_std, t1c_std_path);

    iftDestroyImage(&t2f_img);
    iftDestroyImage(&t1c_img);
    iftDestroyMImage(&merged);
    iftDestroyMImage(&t2f_img_std);
    iftDestroyMImage(&t1c_img_std);
    iftFree(filename);
  }

  iftDestroyFileSet(&fs);

  puts("\nDone ...");
  puts(iftFormattedTime(iftCompTime(tstart, iftToc())));
  memory_end = iftMemoryUsed();
  iftVerifyMemory(memory_start, memory_end);

  return 0;
}