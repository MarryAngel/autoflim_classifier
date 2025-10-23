#include "ift.h"

typedef enum _dist_type {
   EUCLIDEAN, 
   COSINE
} Dist_Type;

void iftFindPrototypes(iftDataSet *Z, iftFileSet *fs, iftCSV *prototypes, double percentagePrototypes, int *nprototypes) {

	/* Calculates the number of prototypes that each class*/
	int *nprototypes_per_class = iftAllocIntArray(Z->nclasses + 1);
	for (int c = 1; c <= Z->nclasses; c++) {
		int nsamples = 0;
		for (int s = 0; s < Z->nsamples; s++) {
			if (Z->sample[s].truelabel == c)
				nsamples++;
		}
		nprototypes_per_class[c] = (int)(nsamples * percentagePrototypes);
	}
	
	/* Mark the images listed as prototypes */
	for (int row=0; row < prototypes->nrows; row++) {
		char *basename1 = iftBasename(prototypes->data[row][1]);
		for (int i=0; i < fs->n; i++) {
			char *basename2 = iftFilename(fs->files[i]->path, iftFileExt(fs->files[i]->path));
			if (strcmp(basename1, basename2) == 0) {
				iftAddSampleStatus(&(Z->sample[i]), IFT_PROTOTYPE);
				// printf("Sample %d is prototype %s from class %d\n", i, basename2, Z->sample[i].truelabel);
			}
			iftFree(basename2);
		}
		iftFree(basename1);
	}

	/*Recalculate the number of prototypes to be added for each class,
	subtracting the number of already marked prototypes.*/
	for (int c = 1; c <= Z->nclasses; c++) {
		int marked = 0;
		for (int s = 0; s < Z->nsamples; s++) {
			if (Z->sample[s].truelabel == c && iftHasSampleStatus(Z->sample[s], IFT_PROTOTYPE)) {
				marked++;
			}
		}
		nprototypes_per_class[c] -= marked;
		if (nprototypes_per_class[c] < 0)
			nprototypes_per_class[c] = 0;
	}
	
	// Mark the remaining prototypes randomly for each class
	for (int c = 1; c <= Z->nclasses; c++) {
		int marked = 0;
		while (marked < nprototypes_per_class[c]) {
			int s = iftRandomInteger(0, Z->nsamples - 1);
			if (Z->sample[s].truelabel == c && !iftHasSampleStatus(Z->sample[s], IFT_PROTOTYPE)) {
				iftAddSampleStatus(&Z->sample[s], IFT_PROTOTYPE);
				marked++;
			}
		}
	}
	iftFree(nprototypes_per_class);

	/* Print the number of prototypes per class and count the total prototypes */
	for (int c = 1; c <= Z->nclasses; c++) {
		int count = 0;
		for (int s = 0; s < Z->nsamples; s++) {
			if (Z->sample[s].truelabel == c && iftHasSampleStatus(Z->sample[s], IFT_PROTOTYPE)) {
				count++;
			}
		}
		// printf("Class %d has %d prototypes.\n", c, count);
		*nprototypes += count;
	}
}

int main(int argc, char *argv[]){

	timer *tstart = NULL;

    if (argc != 5) {
        iftError("Usage: iftPrototypeEvaluation <P1> <P2> <P3> <P4>\n"
                 "P1: DataSet with training samples (.zip).\n"
                 "P2: Prototypes (.csv file with FLIM's training images).\n"
                 "P3: Output file with the misclassified images (.csv)\n"
                 "P4: Distance type (0: Cosine, 1: Euclidean)\n",
                 "main");
	}

    tstart = iftTic();

    iftDataSet *Z    = iftReadDataSet(argv[1]);
	iftCSV *prototypes = iftReadCSV(argv[2],'/');
    iftFileSet *fs   = NULL;
	
    /* Verify the dataset has a file set */
    if (Z->ref_data_type == IFT_REF_DATA_FILESET){
    	fs = (iftFileSet *)Z->ref_data;

    	if (Z->nsamples != fs->n){
			iftError("Dataset and file set do not have the same number of samples","main");
		}
	}else{
		iftError("Dataset does not have a file set. Cannot proceed with prototype evaluation.", "main");
	}
	
	/* Identify prototypes in the dataset based on the CSV file and 
	randomly select additional prototypes for each class according 
	to the specified percentage. */

	double percentagePrototypes = 0.5; 
	int nprototypes = 0;
	iftFindPrototypes(Z, fs, prototypes, percentagePrototypes, &nprototypes);
	iftDestroyCSV(&prototypes);
	
	printf("Total number of prototypes to be evaluated: %d\n", nprototypes);
	
	int *prototype_sample = iftAllocIntArray(nprototypes);
	iftMatrix *P          = iftCreateMatrix(nprototypes,Z->nfeats);

	float *nerrors=iftAllocFloatArray(Z->nclasses+1);
	int   *nsamples_per_class = iftCountSamplesPerClassDataSet(Z);

	/* Set the chosen distance type using the Dist_Type enum */
	Dist_Type DIST_TYPE = atoi(argv[4]);
	if (DIST_TYPE < 0 || DIST_TYPE > 1) {
		iftError("Invalid distance type. Use 0 for Cosine or 1 for Euclidean.", "main");
	}
	DIST_TYPE = (DIST_TYPE == 0) ? COSINE : EUCLIDEAN;

	if (DIST_TYPE == COSINE) {
		printf("Using Cosine distance.\n");

		/* Normalize the dataset by Z-score */
		Z = iftNormalizeDataSetByZScore(Z, NULL, 0.001);

		printf("Normalizing dataset by Z-score.\n");
		printf("Number of samples: %d, Number of features: %d\n", Z->nsamples, Z->nfeats);

		/* Normalize the feature vector of each sample, including
 			prototypes that are copied to matrix P. */
	
		int p=0; 
		for (int s=0; s < Z->nsamples; s++){
			
			float norm=0.0;
			for (int f=0; f < Z->nfeats; f++)
				norm += Z->sample[s].feat[f]*Z->sample[s].feat[f];
			norm = sqrtf(norm);

			if (!iftAlmostZero(norm))
				for (int f=0; f < Z->nfeats; f++)
					Z->sample[s].feat[f] = Z->sample[s].feat[f]/norm;
			
			if (iftHasSampleStatus(Z->sample[s],IFT_PROTOTYPE)){
				prototype_sample[p]=s;
				for (int f=0; f < Z->nfeats; f++) 
					iftMatrixElem(P, p, f) = Z->sample[s].feat[f];
				p++;
			}

		}

		/* Compute cosine similarity between all samples and the prototypes */

		iftMatrix *S = iftMultMatrices(Z->data,P);

		/* Verify if the most similar samples is from the same class
		and count the errors. */

		FILE *fp = fopen(argv[3],"w");

		fprintf(fp,"Misclassified Image; Certainty;\n");

		for (int s = 0; s < S->nrows; s++){ /* samples */
			
			float max_simil1=  IFT_INFINITY_FLT_NEG;
			float max_simil2=  IFT_INFINITY_FLT_NEG;
			int   p1        =  IFT_NIL;
			int   p2        =  IFT_NIL;
			Z->sample[s].weight = 0;

			/* Encontrar o protótipo mais similar para a amostra */
			for (int col = 0; col < S->ncols; col++){
				if (iftMatrixElem(S, col, s) > max_simil1){
					max_simil1 = iftMatrixElem(S, col, s);
					p1 = col;
				}
			} 

			/*Encontrar o segundo protótipo mais similar de classe diferente*/
			int t1 = prototype_sample[p1];

			for (int col = 0; col < S->ncols; col++){
				/*compara com max_simil1*/
				if (iftMatrixElem(S, col, s) > max_simil2){
					int t2 = prototype_sample[col];
					if (Z->sample[t1].truelabel != Z->sample[t2].truelabel){
						max_simil2 = iftMatrixElem(S, col, s);
						p2 = col;
					}
				}
			}

			// printf("Sample %d: Prototype %d (class %d) - Similarity: %f, Second Prototype %d (class %d) - Similarity: %f\n",
			// 	s, p1, Z->sample[t1].truelabel, max_simil1, p2, Z->sample[prototype_sample[p2]].truelabel, max_simil2);

			Z->sample[s].weight = max_simil1 / (max_simil1 + max_simil2);

			/*Gravar no arquivo*/

			if (Z->sample[t1].truelabel != Z->sample[s].truelabel){
				nerrors[Z->sample[s].truelabel]++;
				fprintf(fp,"%s;%f;\n",fs->files[s]->path,Z->sample[s].weight);
			}

			// /*mostrar amostras e protótipos que são da mesma classe e a sua certeza*/
			// if (Z->sample[t1].truelabel == Z->sample[s].truelabel){
			// 	printf("Sample %d is correctly classified as class %d with certainty %f\n",
			// 		s, Z->sample[t1].truelabel, Z->sample[s].weight);
			// }
			
		}


	fclose(fp);
	iftDestroyMatrix(&S);
	}

	else if (DIST_TYPE == EUCLIDEAN) {
		printf("Using Euclidean distance.\n");

		/*Reduzindo a dimensionalidade com TSNE*/
		int perplexity = 50;
		int max_iter   = 2000;
		int ndim=2;
		iftDataSet *Z2 = iftDimReductionByTSNE(Z, ndim, perplexity, max_iter);
		
		/*Separar os protótipos*/
		iftMatrix *P          = iftCreateMatrix(nprototypes,Z2->nfeats);
		
		int p=0; 
		for (int s=0; s < Z2->nsamples; s++){
			if (iftHasSampleStatus(Z2->sample[s],IFT_PROTOTYPE)){
				prototype_sample[p]=s;
				for (int f=0; f < Z2->nfeats; f++) 
					iftMatrixElem(P, p, f) = Z2->sample[s].feat[f];
				p++;
			}
		}

		/*Calcular a distância euclidiana das amostras com o protótipo*/
		iftMatrix *D = iftCreateMatrix(P->ncols, Z2->nsamples);

		for (int s = 0; s < Z2->nsamples; s++){
			
			for (int p = 0; p < P->ncols; p++){
				float dist = 0.0;
				for (int f=0; f < Z2->nfeats; f++){
					float diff = Z2->sample[s].feat[f] - iftMatrixElem(P, p, f);
					dist += diff * diff;
				}
				iftMatrixElem(D, p, s) = sqrtf(dist);
			}

		}

		FILE *fp = fopen(argv[3],"w");

		fprintf(fp,"Misclassified Image; Certainty;\n");

		for (int s = 0; s < Z2->nsamples; s++){
			float min_dist1=  IFT_INFINITY_FLT;
			float min_dist2=  IFT_INFINITY_FLT;
			int   p1        =  IFT_NIL;
			int   p2        =  IFT_NIL;
			Z->sample[s].weight = 0;

			/*Encontrar o protótipo de menor distância para aquela amostra*/
			for (int col = 0; col < D->ncols; col++){
				if (iftMatrixElem(D, col, s) < min_dist1){
					min_dist1 = iftMatrixElem(D, col, s);
					p1 = col;
				}
			}

			/*Encontrar o segundo protótipo de menor distância de classe diferente*/
			int t1 = prototype_sample[p1];
			for (int col = 0; col < D->ncols; col++){
				/*compara com min_dist1*/
				if (iftMatrixElem(D, col, s) < min_dist2){
					int t2 = prototype_sample[col];
					if (Z2->sample[t1].truelabel != Z2->sample[t2].truelabel){
						min_dist2 = iftMatrixElem(D, col, s);
						p2 = col;
					}
				}
			}

			Z2->sample[s].weight = min_dist1 / (min_dist1 + min_dist2);

			/*Gravar no arquivo*/
			if (Z2->sample[t1].truelabel != Z2->sample[s].truelabel){
				nerrors[Z2->sample[s].truelabel]++;
				fprintf(fp,"%s;%f;\n",fs->files[s]->path,Z2->sample[s].weight);
			}
		}

		fclose(fp);
		iftDestroyDataSet(&Z2);
		iftDestroyMatrix(&D);
	}
	
	iftDestroyMatrix(&P);
	iftFree(prototype_sample);

	printf("Accuracy per class: \n");

	for (int c=1; c <= Z->nclasses; c++)	
		printf("Class %d: %f\n",c,1.0 - nerrors[c]/nsamples_per_class[c]);
	
	iftFree(nsamples_per_class);
	iftDestroyDataSet(&Z);
    
    printf("Done ... %s\n", iftFormattedTime(iftCompTime(tstart, iftToc())));

    return 0;	
}
	

// #include "ift.h"

// typedef enum _dist_type {
//    EUCLIDEAN, 
//    COSINE
// } Dist_Type;

// #define DIST_TYPE COSINE /* Euclidean: 0 Cosine: 1 */

// int main(int argc, char *argv[]){

// 	printf("Starting iftPrototypeEvaluation...\n");

// 	timer *tstart = NULL;

//     if (argc != 4) {
//         iftError("Usage: iftPrototypeEvaluation <P1> <P2> <P3>\n"
//                  "P1: DataSet with training samples (.zip).\n"
//                  "P2: Prototypes (.csv file with FLIM's training images).\n"
//                  "P3: Output file with the misclassified images (.csv)\n",
//                  "main");
// 	}

//     tstart = iftTic();

//     iftDataSet *Z    = iftReadDataSet(argv[1]);
//     iftFileSet *fs   = NULL;

//     /* Verify the dataset has a file set */
    
//     if (Z->ref_data_type == IFT_REF_DATA_FILESET){
//     	fs = (iftFileSet *)Z->ref_data;

//     	if (Z->nsamples != fs->n){
// 			iftError("Dataset and file set do not have the same number of samples","main");
// 		}

// 		/* for (int i=0; i < fs->n; i++) */
// 		/* 	printf("%s\n",fs->files[i]->path); */

//     	iftCSV *prototypes = iftReadCSV(argv[2],'/');

// 		/* for (int row=0; row < prototypes->nrows; row++) */
// 		/* 	printf("%s\n",prototypes->data[row][1]); */
		
// 		/* Find the prototypes (training images) in the dataset and set
// 		their status accordingly. Sample twice the number of impurity
// 		prototypes. */
      
//     	for (int row=0; row < prototypes->nrows; row++){

// 			char *basename1 = iftBasename(prototypes->data[row][1]);
			
// 			for (int i=0; i < fs->n; i++){

// 	  			char *basename2 = iftFilename(fs->files[i]->path, iftFileExt(fs->files[i]->path));
// 				//printf("Basename1: %s, Basename2: %s\n", basename1, basename2);

// 				if (strcmp(basename1,basename2)==0){
// 					iftAddSampleStatus(&Z->sample[i], IFT_PROTOTYPE);
// 					printf("Sample %d is prototype %s from class %d\n",
// 						i,basename2,Z->sample[i].truelabel);
// 				}
// 				iftFree(basename2);
// 			}
		
// 			iftFree(basename1);
// 		}

//     	int nimpurities=0;

//     	while(nimpurities != 2*prototypes->nrows){

// 			int s = iftRandomInteger(0, Z->nsamples-1);

// 			if (Z->sample[s].truelabel == 9){
// 				iftAddSampleStatus(&Z->sample[s], IFT_PROTOTYPE);
// 				nimpurities++;  
// 			}
//     	}

// 		int  nprototypes      = prototypes->nrows + nimpurities;
// 		int *prototype_sample = iftAllocIntArray(nprototypes);
// 		iftMatrix *P          = iftCreateMatrix(nprototypes,Z->nfeats);
// 		iftDestroyCSV(&prototypes);

// 		float *nerrors=iftAllocFloatArray(Z->nclasses+1);
// 		int   *nsamples_per_class = iftCountSamplesPerClassDataSet(Z);

//       	if (DIST_TYPE==COSINE) {
      
// 			/* Normalize the feature vector of each sample, including
// 			prototypes that are copied to matrix P. */
	
// 			int p=0; /* prototype index */
// 			for (int s=0; s < Z->nsamples; s++){

// 				float norm=0.0;
// 				for (int f=0; f < Z->nfeats; f++)
// 					norm += Z->sample[s].feat[f]*Z->sample[s].feat[f];
// 				norm = sqrtf(norm);
// 				if (!iftAlmostZero(norm))
// 					for (int f=0; f < Z->nfeats; f++)
// 						Z->sample[s].feat[f] = Z->sample[s].feat[f]/norm;
				
// 				if (iftHasSampleStatus(Z->sample[s],IFT_PROTOTYPE)){
// 					prototype_sample[p]=s;
// 					for (int f=0; f < Z->nfeats; f++) 
// 						iftMatrixElem(P, p, f) = Z->sample[s].feat[f];
// 					p++;
// 				}

// 			}
	      
// 			/* Compute cosine similarity between all samples and the prototypes */
	
// 			iftMatrix *S = iftMultMatrices(Z->data,P);

// 			/* Verify if the most similar samples is from the same class
// 			and count the errors. */

// 			FILE *fp = fopen(argv[3],"w");
	
// 			fprintf(fp,"Misclassified Image; Certainty;\n");

// 			for (int s = 0; s < S->nrows; s++){ /* samples */
// 				float max_simil1=  IFT_INFINITY_FLT_NEG;
// 				float max_simil2=  IFT_INFINITY_FLT_NEG;
// 				int   p1        =  IFT_NIL;
// 				int   p2        =  IFT_NIL;
// 				Z->sample[s].weight = 0;

// 				/* Encontrar o protótipo mais similar para a amostra */
// 				for (int col = 0; col < S->ncols; col++){
// 					if (iftMatrixElem(S, col, s) > max_simil1){
// 						max_simil1 = iftMatrixElem(S, col, s);
// 						p1 = col;
// 					}
// 				} 

// 				/*Encontrar o segundo protótipo mais similar de classe diferente*/
// 				int t1 = prototype_sample[p1];

// 				for (int col = 0; col < S->ncols; col++){
// 					/*compara com max_simil1*/
// 					if (iftMatrixElem(S, col, s) > max_simil2){
// 						int t2 = prototype_sample[col];
// 						if (Z->sample[t1].truelabel != Z->sample[t2].truelabel){
// 							max_simil2 = iftMatrixElem(S, col, s);
// 							p2 = col;
// 						}
// 					}
// 				}

// 				Z->sample[s].weight = max_simil1 / (max_simil1 + max_simil2);


// 				// /*O que foi implementado na terça [24/06]*/
// 				// for (int col = 0; col < S->ncols; col++){ /* prototypes */
// 				// 	if (iftMatrixElem(S, col, s)>max_simil1){
// 				// 		if (p1 == IFT_NIL){
// 				// 			max_simil1 = iftMatrixElem(S, col, s);
// 				// 			p1         = col;
// 				// 		}else if (p2 == IFT_NIL){
// 				// 			int t1 = prototype_sample[p1];
// 				// 			if (Z->sample[t1].truelabel != Z->sample[s].truelabel){
// 				// 				max_simil2 = max_simil1;
// 				// 				p2 = p1;
// 				// 				Z->sample[s].weight =  iftMatrixElem(S, col, s) / (iftMatrixElem(S, col, s) + max_simil2);
// 				// 			}
// 				// 			max_simil1 = iftMatrixElem(S, col, s);
// 				// 			p1         = col;
// 				// 		}
						
// 				// 		else {
// 				// 			int t1 = prototype_sample[p1];
// 				// 			if (Z->sample[t1].truelabel != Z->sample[s].truelabel){
// 				// 				max_simil2 = max_simil1;
// 				// 				p2 = p1;
// 				// 			}
// 				// 			max_simil1 = iftMatrixElem(S, col, s);
// 				// 			p1         = col;
// 				// 			Z->sample[s].weight =  iftMatrixElem(S, col, s) / (iftMatrixElem(S, col, s) + max_simil2);
							
// 				// 		}
// 				// 	}
// 				// }

// 				/*normalizar entre 0 e 1*/

// 				// int t1 = prototype_sample[p1];
// 				if (Z->sample[t1].truelabel != Z->sample[s].truelabel){
// 					nerrors[Z->sample[s].truelabel]++;
// 					fprintf(fp,"%s;%f;\n",fs->files[s]->path,Z->sample[s].weight);
// 				}

// 			}


// 		fclose(fp);
// 		iftDestroyMatrix(&S);

//       	} else { /* EUCLIDEAN */

// 			int perplexity = 50;
// 			int max_iter   = 2000;
// 			// Z2d  = iftDimReductionByTSNE(Z, 2, perplexity, max_iter);

// 			// /* Normalize the feature vector of each sample, including
// 			// prototypes that are copied to matrix P. */
// 			// for (int s=0; s < Z->nsamples; s++){
// 			// 	float norm=0.0;
// 			// 	for (int f=0; f < Z->nfeats; f++)
// 			// 		norm += Z->sample[s].feat[f]*Z->sample[s].feat[f];
// 			// 	norm = sqrtf(norm);
// 			// 	if (!iftAlmostZero(norm))
// 			// 		for (int f=0; f < Z->nfeats; f++)
// 			// 			Z->sample[s].feat[f] = Z->sample[s].feat[f]/norm;
// 			// }

// 			// /* Compute euclidean distance between all samples and the prototypes */
// 			// iftMatrix *S = iftCreateMatrix(P->nrows, Z->nsamples);
// 			// for (int s = 0; s < Z->nsamples; s++)
// 			// 	for (int p = 0; p < P->nrows; p++){
// 			// 		float dist = 0.0;
// 			// 		for (int f = 0; f < Z->nfeats; f++)
// 			// 			dist += (Z->sample[s].feat[f] - iftMatrixElem(P, p, f)) * (Z->sample[s].feat[f] - iftMatrixElem(P, p, f));
// 			// 		iftMatrixElem(S, p, s) = sqrtf(dist);
// 			// 	}
			

// 		}

//       	iftDestroyMatrix(&P);
//       	iftFree(prototype_sample);

//       	printf("Accuracy per class: \n");
//       	for (int c=1; c <= Z->nclasses; c++)	
// 			printf("Class %d: %f\n",c,1.0 - nerrors[c]/nsamples_per_class[c]);

//       	iftFree(nsamples_per_class);
// 	}

//     iftDestroyDataSet(&Z);
    
//     printf("Done ... %s\n", iftFormattedTime(iftCompTime(tstart, iftToc())));

//     return 0;
// }

// // #include "ift.h"

// // typedef enum _dist_type {
// //    EUCLIDEAN, 
// //    COSINE
// // } Dist_Type;

// // //#define DIST_TYPE COSINE /* Euclidean: 0 Cosine: 1 */
// // #define DIST_TYPE EUCLIDEAN

// // int main(int argc, char *argv[]){

// // 	timer *tstart = NULL;

// //     if (argc != 4) {
// //         iftError("Usage: iftPrototypeEvaluation <P1> <P2> <P3>\n"
// //                  "P1: DataSet with training samples (.zip).\n"
// //                  "P2: Prototypes (.csv file with FLIM's training images).\n"
// //                  "P3: Output file with the misclassified images (.csv)\n",
// //                  "main");
// // 	}

// //     tstart = iftTic();

// //     iftDataSet *Z    = iftReadDataSet(argv[1]);
// //     iftFileSet *fs   = NULL;

// //     /* Verify the dataset has a file set */
    
// //     if (Z->ref_data_type == IFT_REF_DATA_FILESET){
// //     	fs = (iftFileSet *)Z->ref_data;

// //     	if (Z->nsamples != fs->n){
// // 			iftError("Dataset and file set do not have the same number of samples","main");
// // 		}

// // 		/* for (int i=0; i < fs->n; i++) */
// // 		/* 	printf("%s\n",fs->files[i]->path); */

// //     	iftCSV *prototypes = iftReadCSV(argv[2],'/');

// // 		/* for (int row=0; row < prototypes->nrows; row++) */
// // 		/* 	printf("%s\n",prototypes->data[row][1]); */
		
// // 		/* Find the prototypes (training images) in the dataset and set
// // 		their status accordingly. Sample twice the number of impurity
// // 		prototypes. */
      
// //     	for (int row=0; row < prototypes->nrows; row++){

// // 			char *basename1 = iftBasename(prototypes->data[row][1]);
			
// // 			for (int i=0; i < fs->n; i++){

// // 	  			char *basename2 = iftFilename(fs->files[i]->path, iftFileExt(fs->files[i]->path));
				
// // 				if (strcmp(basename1,basename2)==0){
// // 					iftAddSampleStatus(&Z->sample[i], IFT_PROTOTYPE);
// // 					printf("Sample %d is prototype %s from class %d\n",
// // 						i,basename2,Z->sample[i].truelabel);
// // 				}
// // 				iftFree(basename2);
// // 			}
		
// // 			iftFree(basename1);
// // 		}

// //     	int nimpurities=0;

// // 		/*Percorrer todas as classes e separar 50% das amostras como protótipo*/
// // 		// for (int c=0; c < Z->nclasses; c++){
// // 		// 	int nsamples_per_class = ;
// // 		// 	int nproto = nsamples_per_class / 2;
// // 		// 	for (int p=0; p < nproto; p++){
// // 		// 		int s = iftRandomInteger(0, nclass-1);
// // 		// 		iftAddSampleStatus(&Z->sample[s], IFT_PROTOTYPE);
// // 		// 	}
// // 		// }

// //     	while(nimpurities != 2*prototypes->nrows){

// // 			int s = iftRandomInteger(0, Z->nsamples-1);

// // 			if (Z->sample[s].truelabel == 9){
// // 				iftAddSampleStatus(&Z->sample[s], IFT_PROTOTYPE);
// // 				nimpurities++;  
// // 			}
// //     	}

// // 		int  nprototypes      = prototypes->nrows + nimpurities;
// // 		int *prototype_sample = iftAllocIntArray(nprototypes);
// // 		iftMatrix *P          = iftCreateMatrix(nprototypes,Z->nfeats);
// // 		iftDestroyCSV(&prototypes);

// // 		float *nerrors=iftAllocFloatArray(Z->nclasses+1);
// // 		int   *nsamples_per_class = iftCountSamplesPerClassDataSet(Z);

// // 		Z = iftNormalizeDataSetByZScore(Z, NULL, 0.001);
		
// //       	if (DIST_TYPE==COSINE) {
      
// // 			printf("Usando distância cosseno\n");

// // 			/* Normalize the feature vector of each sample, including
// // 			prototypes that are copied to matrix P. */
	
// // 			int p=0; /* prototype index */
// // 			for (int s=0; s < Z->nsamples; s++){

// // 				float norm=0.0;
// // 				for (int f=0; f < Z->nfeats; f++)
// // 					norm += Z->sample[s].feat[f]*Z->sample[s].feat[f];
// // 				norm = sqrtf(norm);

// // 				if (!iftAlmostZero(norm))
// // 					for (int f=0; f < Z->nfeats; f++)
// // 						Z->sample[s].feat[f] = Z->sample[s].feat[f]/norm;
				
// // 				if (iftHasSampleStatus(Z->sample[s],IFT_PROTOTYPE)){
// // 					prototype_sample[p]=s;
// // 					for (int f=0; f < Z->nfeats; f++) 
// // 						iftMatrixElem(P, p, f) = Z->sample[s].feat[f];
// // 					p++;
// // 				}

// // 			}
	      
// // 			/* Compute cosine similarity between all samples and the prototypes */
	
// // 			iftMatrix *S = iftMultMatrices(Z->data,P);

// // 			/* Verify if the most similar samples is from the same class
// // 			and count the errors. */

// // 			FILE *fp = fopen(argv[3],"w");
	
// // 			fprintf(fp,"Misclassified Image; Certainty;\n");

// // 			for (int s = 0; s < S->nrows; s++){ /* samples */
// // 				float max_simil1=  IFT_INFINITY_FLT_NEG;
// // 				float max_simil2=  IFT_INFINITY_FLT_NEG;
// // 				int   p1        =  IFT_NIL;
// // 				int   p2        =  IFT_NIL;
// // 				Z->sample[s].weight = 0;

// // 				/* Encontrar o protótipo mais similar para a amostra */
// // 				for (int col = 0; col < S->ncols; col++){
// // 					if (iftMatrixElem(S, col, s) > max_simil1){
// // 						max_simil1 = iftMatrixElem(S, col, s);
// // 						p1 = col;
// // 					}
// // 				} 

// // 				/*Encontrar o segundo protótipo mais similar de classe diferente*/
// // 				int t1 = prototype_sample[p1];

// // 				for (int col = 0; col < S->ncols; col++){
// // 					/*compara com max_simil1*/
// // 					if (iftMatrixElem(S, col, s) > max_simil2){
// // 						int t2 = prototype_sample[col];
// // 						if (Z->sample[t1].truelabel != Z->sample[t2].truelabel){
// // 							max_simil2 = iftMatrixElem(S, col, s);
// // 							p2 = col;
// // 						}
// // 					}
// // 				}

// // 				Z->sample[s].weight = max_simil1 / (max_simil1 + max_simil2);

// // 				/*normalizar entre 0 e 1*/

// // 				/*Gravar no arquivo*/
// // 				if (Z->sample[t1].truelabel != Z->sample[s].truelabel){
// // 					nerrors[Z->sample[s].truelabel]++;
// // 					fprintf(fp,"%s;%f;\n",fs->files[s]->path,Z->sample[s].weight);
// // 				}

// // 			}


// // 		fclose(fp);
// // 		iftDestroyMatrix(&S);
		

// //       	} else { /* EUCLIDEAN */

// // 			printf("Usando distância euclidiana\n");

// // 			/*Reduzindo a dimensionalidade com TSNE*/
// // 			int perplexity = 50;
// // 			int max_iter   = 2000;
// // 			int ndim=2;
// // 			iftDataSet *Z2 = iftDimReductionByTSNE(Z, ndim, perplexity, max_iter);
			
// // 			/*Separar os protótipos*/
// // 			iftMatrix *P          = iftCreateMatrix(nprototypes,Z2->nfeats);
// // 			printf("Dimensão de P: %d x %d\n",P->nrows,P->ncols);
			
// // 			int p=0; 
// // 			for (int s=0; s < Z2->nsamples; s++){
// // 				if (iftHasSampleStatus(Z2->sample[s],IFT_PROTOTYPE)){
// // 					prototype_sample[p]=s;
// // 					for (int f=0; f < Z2->nfeats; f++) 
// // 						iftMatrixElem(P, p, f) = Z2->sample[s].feat[f];
// // 					p++;
// // 				}
// // 			}

// // 			/*Calcular a distância euclidiana das amostras com o protótipo*/
// // 			iftMatrix *D = iftCreateMatrix(P->ncols, Z2->nsamples);

// // 			for (int s = 0; s < Z2->nsamples; s++){
				
// // 				for (int p = 0; p < P->ncols; p++){
// // 					float dist = 0.0;
// // 					for (int f=0; f < Z2->nfeats; f++){
// // 						float diff = Z2->sample[s].feat[f] - iftMatrixElem(P, p, f);
// // 						dist += diff * diff;
// // 					}
// // 					iftMatrixElem(D, p, s) = sqrtf(dist);
// // 				}

// // 			}

// // 			FILE *fp = fopen(argv[3],"w");
	
// // 			fprintf(fp,"Misclassified Image; Certainty;\n");

// // 			for (int s = 0; s < Z2->nsamples; s++){
// // 				float min_dist1=  IFT_INFINITY_FLT;
// // 				float min_dist2=  IFT_INFINITY_FLT;
// // 				int   p1        =  IFT_NIL;
// // 				int   p2        =  IFT_NIL;
// // 				Z->sample[s].weight = 0;

// // 				/*Encontrar o protótipo de menor distância para aquela amostra*/
// // 				for (int col = 0; col < D->ncols; col++){
// // 					if (iftMatrixElem(D, col, s) < min_dist1){
// // 						min_dist1 = iftMatrixElem(D, col, s);
// // 						p1 = col;
// // 					}
// // 				}

// // 				/*Encontrar o segundo protótipo de menor distância de classe diferente*/
// // 				int t1 = prototype_sample[p1];
// // 				for (int col = 0; col < D->ncols; col++){
// // 					/*compara com min_dist1*/
// // 					if (iftMatrixElem(D, col, s) < min_dist2){
// // 						int t2 = prototype_sample[col];
// // 						if (Z2->sample[t1].truelabel != Z2->sample[t2].truelabel){
// // 							min_dist2 = iftMatrixElem(D, col, s);
// // 							p2 = col;
// // 						}
// // 					}
// // 				}

// // 				Z2->sample[s].weight = min_dist1 / (min_dist1 + min_dist2);

// // 				/*Gravar no arquivo*/
// // 				if (Z2->sample[t1].truelabel != Z2->sample[s].truelabel){
// // 					nerrors[Z2->sample[s].truelabel]++;
// // 					fprintf(fp,"%s;%f;\n",fs->files[s]->path,Z2->sample[s].weight);
// // 				}
// // 			}

// // 			fclose(fp);
// // 			iftDestroyDataSet(&Z2);
// // 			iftDestroyMatrix(&D);
// // 		}

// //       	iftDestroyMatrix(&P);
// //       	iftFree(prototype_sample);

// //       	printf("Accuracy per class: \n");
// //       	for (int c=1; c <= Z->nclasses; c++)	
// // 			printf("Class %d: %f\n",c,1.0 - nerrors[c]/nsamples_per_class[c]);

// //       	iftFree(nsamples_per_class);
// // 	}

// //     iftDestroyDataSet(&Z);
    
// //     printf("Done ... %s\n", iftFormattedTime(iftCompTime(tstart, iftToc())));

// //     return 0;
// // }
