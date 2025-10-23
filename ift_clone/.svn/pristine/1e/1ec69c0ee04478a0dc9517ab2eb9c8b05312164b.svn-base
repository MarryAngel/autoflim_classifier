#include "ift.h"
#include<time.h>


#define MIN_DISTANCE_THRESHOLD 5.0
#define RADIUS 4.0
int main(int argc, char *argv[])
{
    if (argc != 5)
      iftError("Usage: iftDrawMntsSeeds P1 P2 P3 P4\n"
	       "P1: input folder with predicted seeds (-seeds.txt)\n"
	       "P2: input folder with label seeds (-seeds.txt)\n"
           "P3: input folder with images images (.png)\n"
           "P4: output folder with classification report and drawn minutiaes\n",	       
	       "main");

    char *out_dir = argv[4];
    iftMakeDir(out_dir);
    time_t tm;
    time(&tm);

    iftAdjRel *A = iftCircular(RADIUS);

    char filename[200];
    sprintf(filename, "%s/classification_report.txt", out_dir);

    FILE *classification_report = fopen(filename, "w");
    fprintf(classification_report, "Classification Report - %s\n", ctime(&tm));

    const char *predicted_seeds_path = argv[1];
    const char *label_seeds_path     = argv[2];
    const char *images_path          = argv[3];
    
    iftFileSet *pred_fs  = iftLoadFileSetFromDirBySuffix(predicted_seeds_path, "-seeds.txt", 1);
    iftFileSet *label_fs = iftLoadFileSetFromDirBySuffix(label_seeds_path, "-seeds.txt", 1);
    iftFileSet *imgs_fs  = iftLoadFileSetFromDirBySuffix(images_path, ".png", 1);

    int false_negative, false_positive;

    int npred   = pred_fs->n;
    int nlabel  = label_fs->n;

    float *accuracy = iftAllocFloatArray(npred);
    if (npred != nlabel){
        iftError("Error in iftMntReport.\n Number of predicted seeds must be equal to the number of label seeds\n", "main");
    }

    for (int i = 0; i < npred; i++){
        
        int true_positive = 0;
        char *basename = iftFilename(pred_fs->files[i]->path, "-seeds.txt");
        sprintf(filename, "%s/%s.png", out_dir, basename); // output name


        iftImage *img = iftReadImageByExt(imgs_fs->files[i]->path);

        printf("Image: %s\n", imgs_fs->files[i]->path);
        printf("pred: %s\n", pred_fs->files[i]->path);
        printf("label: %s\n", label_fs->files[i]->path);

        
        iftLabeledSet *pred_seeds  = iftReadSeeds(img, pred_fs->files[i]->path);
        iftLabeledSet *label_seeds = iftReadSeeds(img, label_fs->files[i]->path);
        iftLabeledSet *S3 = iftCopyLabeledSet(label_seeds);

        int nmnts_pred  = iftLabeledSetSize(pred_seeds);
        int nmnts_label = iftLabeledSetSize(label_seeds);

        if (!iftIsColorImage(img))
        {
            img->Cb = iftAlloc(img->n, sizeof *img->Cb);
            img->Cr = iftAlloc(img->n, sizeof *img->Cr);
            for (int i = 0; i < img->n; i++) {
                img->Cb[i] = 127;
                img->Cr[i] = 127;
            }
        }

        // Drawing pred minutiaes
        int norm = iftNormalizationValue(iftMaximumValue(img));
        iftColor pred, label;

        pred.val[0] = norm * 1;
        pred.val[1] = norm * 0;
        pred.val[2] = norm * 0;
        pred = iftRGBtoYCbCr(pred, norm);

        label.val[0] = norm * 0;
        label.val[1] = norm * 1;
        label.val[2] = norm * 0;
        label = iftRGBtoYCbCr(label, norm);

        iftLabeledSet *S = iftCopyLabeledSet(pred_seeds);

        for (; S != NULL; S = S->next) {
            iftVoxel u = iftGetVoxelCoord(img, S->elem);
            for (int i = 0; i < A->n; i++) {
                iftVoxel v = iftGetAdjacentVoxel(A, u, i);
                int p = iftGetVoxelIndex(img, v);
                if (iftValidVoxel(img, v)) {
                    img->val[p] = pred.val[0];
                    img->Cb[p]  = pred.val[1];
                    img->Cr[p]  = pred.val[2];
                }
            }        
        }

        // Drawing label minutias

        S = iftCopyLabeledSet(label_seeds);
        for (; S != NULL; S = S->next) {
            iftVoxel u = iftGetVoxelCoord(img, S->elem);
            for (int i = 0; i < A->n; i++) {
                iftVoxel v = iftGetAdjacentVoxel(A, u, i);
                int p = iftGetVoxelIndex(img, v);
                if (iftValidVoxel(img, v)) {
                    img->val[p] = label.val[0];
                    img->Cb[p]  = label.val[1];
                    img->Cr[p]  = label.val[2];
                }
            }        
        }
        iftDestroyLabeledSet(&S);
        iftWriteImageByExt(img, filename);

        // Identifying correct minutiaes

        iftLabeledSet *S1 = iftCopyLabeledSet(pred_seeds);
        for (; S1 != NULL; S1 = S1->next) {
            iftVoxel u = iftGetVoxelCoord(img, S1->elem);
            
            float dmin = IFT_INFINITY_FLT; // smallest distance between a predicted minutia and all label mnts
            int elem_min; //element in S2 with smallest distance
            float d;
            iftLabeledSet *S2 = iftCopyLabeledSet(label_seeds);
            for (; S2 != NULL; S2 = S2->next){
                iftVoxel v = iftGetVoxelCoord(img, S2->elem);
                d = sqrt((u.x - v.x)*(u.x - v.x) + (u.y - v.y)*(u.y - v.y));
                if (d < dmin){
                    dmin = d;
                    elem_min = S2->elem;
                }
            }
            if (dmin < MIN_DISTANCE_THRESHOLD){ // Found a true positive match
                iftRemoveLabeledSetElem(&pred_seeds, S1->elem);
                if (iftLabeledSetHasElement(S3, elem_min))
                    iftRemoveLabeledSetElem(&S3, elem_min);
                true_positive++;
            }
            
            iftDestroyLabeledSet(&S2);
        }

        false_positive = iftLabeledSetSize(pred_seeds); 
        false_negative = iftLabeledSetSize(S3);

        fprintf(classification_report, "------------------------\n");
        fprintf(classification_report, "Image %s, pred: %d, label: %d\n", basename, nmnts_pred, nmnts_label);
        
        accuracy[i] = true_positive * 100.0 / nmnts_pred;
        fprintf(classification_report, "\tAccuracy: %.2f%%\n", accuracy[i]);
        fprintf(classification_report, "\tTP: %d, FP: %d, FN: %d\n", true_positive, false_positive, false_negative);

        

        iftFree(basename);
        iftDestroyImage(&img);
        iftDestroyLabeledSet(&pred_seeds);
        iftDestroyLabeledSet(&S3);
        iftDestroyLabeledSet(&label_seeds);
        iftDestroyLabeledSet(&S1);
    }


    float mean_acc, std_acc;
    mean_acc = iftMean(accuracy, npred);
    std_acc  = iftStd(accuracy, npred);

    fprintf(classification_report, "------------------------\n");
    fprintf(classification_report, "General Statistics: \n");
    fprintf(classification_report, "Mean accuracy: %.2f, Std accuracy: %.2f", mean_acc, std_acc);

    iftFree(accuracy);
    fclose(classification_report);
    iftDestroyFileSet(&pred_fs);
    iftDestroyFileSet(&label_fs);
    iftDestroyFileSet(&imgs_fs);
}
