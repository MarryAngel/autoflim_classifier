#include "ift.h"
#include "iftFPModel.h"
#include <omp.h>

#define BIFURC_BUFFER_SIZE 32
#define DEG_TO_RAD(angle) ((angle) * IFT_PI / 180.0)

bool fileExists(const char *filename)
{
  FILE *file = fopen(filename, "r"); // Try to open the file in read mode
  if (file)
  {
    fclose(file); // Close the file if it was successfully opened
    return true;  // File exists
  }
  else
  {
    return false; // File does not exist
  }
}

iftSet *GetSkelTermAndBifurcPoints(iftImage *skel, iftImage *orig,
                                   iftDict *param, iftImage *mask)
{
  iftSet *seeds = NULL;
  // iftImage *emask = ErodedInterpolatedMask(mask,2.0,2.0,param);

  float scale_factor = skel->xsize / orig->xsize;

  iftAdjRel *A = iftCircular(orig->xsize / 20);

  for (int p2 = 0; p2 < skel->n; p2++)
  {

    if ((skel->val[p2] == 128))
    {
      bool is_border = false;
      iftVoxel u2 = iftGetVoxelCoord(skel, p2);

      iftVoxel u1 = {iftRound(u2.x / scale_factor), iftRound(u2.y / scale_factor), 0};
      // int p1 = iftGetVoxelIndex(orig, u1);

      // ignore minutiae close to mask border

      for (int i = 0; i < A->n; i++)
      {
        iftVoxel v = iftGetAdjacentVoxel(A, u1, i);
        if (iftValidVoxel(mask, v))
        {
          int p3 = iftGetVoxelIndex(orig, v);

          if (mask != NULL)
          {
            if (mask->val[p3] == 0)
            {
              is_border = true;
              break;
            }
          }
        }
        else
        {
          is_border = true;
          break;
        }

        if (is_border)
        {
          skel->val[p2] = 255;
          continue;
        }
      }
      if (!is_border)
        iftUnionSetElem(&seeds, p2);
    }
  }

  iftDestroyAdjRel(&A);

  return (seeds);
}

void writeMntAngleFromMatrix(iftImage *bin, iftMatrix *M_ang, char *filename)
{

  FILE *file = fopen(filename, "w");

  if (file == NULL)
  {
    perror("Error opening file");
    exit(1);
  }

  int npts = 0;
  // count valid rows
  for (int i = 0; i < M_ang->nrows; i++)
  {
    if (iftMatrixElem(M_ang, 0, i) != -1)
    {
      npts++;
    }
  }

  // reading file header
  int xsize = bin->xsize;
  int ysize = bin->ysize;

  char *basename = iftBasename(filename);
  fprintf(file, "%s\n", basename);
  fprintf(file, "%d %d %d\n", npts, xsize, ysize);

  for (int i = 0; i < M_ang->nrows; i++)
  {
    int p = iftMatrixElem(M_ang, 0, i);
    if (p != -1)
    {
      float theta = iftMatrixElem(M_ang, 1, i);
      iftVoxel v = iftGetVoxelCoord(bin, p);
      fprintf(file, "%d %d %f %d\n", v.x, v.y, theta, 1);
    }
  }
  fclose(file);

  iftFree(basename);
}

// iftImage * MntMapFromSkel(iftImage* skel) {
//   iftImage * mnt_map = crossingNumberImg(skel);
//   int mnts_count = 0;
//   for (int p = 0; p < skel->n; p++)
//   {
//     if ((cn_img_new->val[p] >= 3) || (cn_img_new->val[p] == 1))
//     {
//       // skel->val[p] = 170;
//       skel->val[p] = 128;
//       mnts_count++;
//     }
//   }
// }

float GetTermAngle(iftImage *skel, int p)
{

  iftFIFO *queue = iftCreateFIFO(skel->n);
  iftAdjRel *A = iftCircular(sqrt(2));

  // Initialize for the source pixel
  iftInsertFIFO(queue, p);

  iftImage *distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
  for (int i = 0; i < distance->n; i++)
  {
    distance->val[i] = IFT_INFINITY_INT;
  }

  distance->val[p] = 0;

  int closest_term = -1;

  while (!iftEmptyFIFO(queue))
  {
    int curr = iftRemoveFIFO(queue);

    if (distance->val[curr] >= 10)
    {
      closest_term = curr;
      break;
    }

    // Explore neighbors
    iftVoxel u = iftGetVoxelCoord(skel, curr);
    for (int i = 1; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(skel, v))
      {
        int q = iftGetVoxelIndex(skel, v);

        // Check if neighbor is not visited
        if (queue->color[q] == IFT_WHITE && skel->val[q] > 0)
        {
          iftInsertFIFO(queue, q);
          distance->val[q] = distance->val[curr] + 1;
        }
      }
    }
  }

  iftVoxel v1 = iftGetVoxelCoord(skel, p);
  iftVoxel v2 = iftGetVoxelCoord(skel, closest_term);

  float theta = atan2(v2.y - v1.y, v2.x - v1.x);

  iftResetFIFO(queue);

  iftDestroyFIFO(&queue);
  iftDestroyAdjRel(&A);

  iftDestroyImage(&distance);

  return theta;
}

// Function to normalize a vector
void normalize(iftVector *v)
{
  double magnitude = sqrt(v->x * v->x + v->y * v->y);
  if (magnitude > 0)
  {
    v->x /= magnitude;
    v->y /= magnitude;
  }
}

// Returns angle in degrees
double angle_between_points(int x1, int y1, int x2, int y2)
{
  double dy = y2 - y1;
  double dx = x2 - x1;
  return 180 * atan2(dy, dx) / PI; // angle in degrees, range (-180, 180]
}

// Function to compute the angle of the bisector with respect to the horizontal axis (counter-clockwise)
double angle_bisector(iftVoxel p0, iftVoxel p1, iftVoxel p2)
{

  iftVector v1;
  v1.x = p1.x - p0.x;
  v1.y = p1.y - p0.y;

  iftVector v2;
  v2.x = p2.x - p0.x;
  v2.y = p2.y - p0.y;

  normalize(&v1);
  normalize(&v2);

  iftVector bisector;
  bisector.x = v1.x + v2.x,
  bisector.y = v1.y + v2.y;
  normalize(&bisector);

  // Counter-clockwise angle from horizontal
  double bisec = atan2(-bisector.y, bisector.x) * (180.0 / IFT_PI);

  // return only positive angles
  if (bisec < 0)
    return 360 + bisec;
  else
    return bisec;
}

// Function to find the angle closest to a given theta
double closest_angle(double theta, double a1, double a2, double a3)
{
  double angles[3] = {a1, a2, a3};
  double min_diff = 360.0;
  double closest = a1;

  double theta_180 = theta + 180;

  for (int i = 0; i < 3; i++)
  {
    // double diff = fmod(fabs(angles[i] - theta), 180.0);

    float diff = sqrtf(iftPowerOfTwo(sinf(angles[i] * PI / 180) - sinf(theta * PI / 180)) + iftPowerOfTwo(cosf(angles[i] * PI / 180) - cosf(theta * PI / 180)));
    if (fabs(diff) < fabs(min_diff))
    {
      min_diff = diff;
      closest = angles[i];
    }

    diff = sqrtf(iftPowerOfTwo(sinf(angles[i] * PI / 180) - sinf(theta_180 * PI / 180)) + iftPowerOfTwo(cosf(angles[i] * PI / 180) - cosf(theta_180 * PI / 180)));
    if (fabs(diff) < fabs(min_diff))
    {
      min_diff = diff;
      closest = angles[i];
    }
  }

  return closest;
}

// Iterative DFS to find the deepest node3 from p
int *findDeepestNodeIterative(iftImage *skel, int p, int *count)
{
  iftLIFO *stack = iftCreateLIFO(skel->n);

  iftInsertLIFO(stack, p);

  iftImage *depth = iftCreateImageFromImage(skel);

  iftAdjRel *A = iftCircular(sqrt(2));

  for (int i = 0; i < depth->n; i++)
  {
    depth->val[i] = -1;
  }
  // visited[startX][startY] = 1;

  int *deepest = malloc(BIFURC_BUFFER_SIZE * sizeof(int));
  *count = 0;

  while (!iftEmptyLIFO(stack))
  {
    int node = iftRemoveLIFO(stack);
    int d = depth->val[node];

    // reaches end of tracing
    if (d == BIFURC_R * RESCALE_FACTOR)
    {
      deepest[*count] = node;
      *count = *count + 1;
      continue;
    }

    // Explore neighbors
    iftVoxel u = iftGetVoxelCoord(skel, node);
    for (int i = 1; i < A->n; i++)
    {
      iftVoxel v = iftGetAdjacentVoxel(A, u, i);
      if (iftValidVoxel(skel, v))
      {
        int q = iftGetVoxelIndex(skel, v);

        // Check if neighbor is not visited
        if (stack->color[q] == IFT_WHITE && skel->val[q] > 0)
        {
          depth->val[q] = d + 1;
          iftInsertLIFO(stack, q);
          if (depth->val[q] == BIFURC_R * RESCALE_FACTOR)
            break;
        }
      }
    }
  }

  // iftVoxel p_coords = iftGetVoxelCoord(skel, p);
  // printf("deepest nodes from (%d, %d): ", p_coords.x, p_coords.y);
  // for (int i = 0; i < 5; i++) {
  //   iftVoxel i_coords = iftGetVoxelCoord(skel, deepest[i]);
  //   printf("(%d, %d) ", i_coords.x, i_coords.y);
  // }
  // printf("\n");

  // if (count == 3) {
  //   iftVoxel p_coords = iftGetVoxelCoord(skel, p);
  //   printf("deepest nodes from (%d, %d): ", p_coords.x, p_coords.y);
  //   for (int i = 0; i < 5; i++) {
  //     iftVoxel i_coords = iftGetVoxelCoord(skel, deepest[i]);
  //     printf("(%d, %d) ", i_coords.x, i_coords.y);
  //   }
  //   printf("\n");
  // }

  // assert(*count <= 3);

  iftDestroyImage(&depth);
  iftDestroyAdjRel(&A);
  iftResetLIFO(stack);
  iftDestroyLIFO(&stack);

  return deepest;
}

// iftFIFO *queue = iftCreateFIFO(skel->n);
// iftAdjRel *A = iftCircular(sqrt(2));

// iftAdjRel *B = iftCircular(bifurc_r);

// // Initialize for the source pixel
// iftInsertFIFO(queue, p);

// iftImage* distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
// for (int i = 0; i < distance->n; i++) {
//   distance->val[i] = IFT_INFINITY_INT;
// }

// distance->val[p] = 0;

// int closest_term = -1;
// // iftImage* tmp_img = iftCopyImage(skel);
// while (!iftEmptyFIFO(queue))
// {
//   int curr = iftRemoveFIFO(queue);

//   if (distance->val[curr] >= bifurc_r)
//   {
//     closest_term = curr;
//     break;
//   }

//   skel->val[curr] = iftRound(255.0 * ((float)(bifurc_r - distance->val[curr])/(float)bifurc_r));

//   iftVoxel u,v;

//   // Explore neighbors
//   u = iftGetVoxelCoord(skel, curr);
//   for (int i = 1; i < A->n; i++)
//   {
//     v = iftGetAdjacentVoxel(A, u, i);
//     if (iftValidVoxel(skel, v))
//     {
//       int q = iftGetVoxelIndex(skel, v);

//       // Check if neighbor is not visited
//       if (queue->color[q] == IFT_WHITE && skel->val[q] > 0)
//       {
//         iftInsertFIFO(queue, q);
//         distance->val[q] = distance->val[curr] + 1;
//       }
//     }
//   }
// }

// iftWriteImageByExt(tmp_img, "tmp.png");
// iftDestroyImage(&tmp_img);

// Find the 3 furthest nodes within distance d
// int maxNodes[3] = {-1, -1, -1};
// int maxDistances[3] = {-1, -1, -1};

// iftVoxel u = iftGetVoxelCoord(skel, p);

// for (int i = 0; i < B->n; i++) {
//   iftVoxel v = iftGetAdjacentVoxel(B, u, i);
//   if (iftValidVoxel(skel, v))
//   {
//     int q = iftGetVoxelIndex(skel, v);
//     if ((skel->val[q] != 0) && (q != p) && (distance->val[q] != IFT_INFINITY_INT)) {
//       for (int j = 0; j < 3; j++) {
//               if (distance->val[q] > maxDistances[j]) {
//                   for (int k = 2; k > j; k--) {
//                       maxDistances[k] = maxDistances[k - 1];
//                       maxNodes[k] = maxNodes[k - 1];
//                   }
//                   maxDistances[j] = distance->val[q];
//                   maxNodes[j] = q;
//                   break;
//               }
//       }
//     }
//   }
// }

// returns only positive angle values, -1 if invalid mnt
float GetBifurcAngle(iftImage *skel, iftImage *of_map, int p)
{
  int count = 0;
  int *deepest = findDeepestNodeIterative(skel, p, &count);

  if (count != 3)
  {
    iftFree(deepest);
    return -1;
  }

  iftVoxel u = iftGetVoxelCoord(skel, p);

  // printf("The 3 furthest nodes to (%d, %d) within distance %d are: ", u.x, u.y,BIFURC_R);
  // for (int i = 0; i < 3; i++) {

  //   iftVoxel coords = iftGetVoxelCoord(skel, deepest[i]);
  //   printf("(%d %d) (distance %f) ", coords.x, coords.y, sqrtf(iftPowerOfTwo(u.x - coords.x) + iftPowerOfTwo(u.y - coords.y)));

  // }
  // printf("\n");

  iftVoxel p1 = iftGetVoxelCoord(skel, deepest[0]);
  iftVoxel p2 = iftGetVoxelCoord(skel, deepest[1]);
  iftVoxel p3 = iftGetVoxelCoord(skel, deepest[2]);

  double angle1 = angle_bisector(u, p1, p2);
  double angle2 = angle_bisector(u, p2, p3);
  double angle3 = angle_bisector(u, p3, p1);

  // printf("bisectors %f, %f, %f\n", (float)angle1, (float)angle2, (float)angle3);
  int p_orig = SkelToOrigIndex(skel, of_map, p);
  double theta_of = 180 - 180 * ((float)of_map->val[p_orig] / 255.0);

  double closest = 360 - closest_angle(theta_of, angle1, angle2, angle3);

  // printf("closest bisector to %f: %f\n", theta_of, closest);

  iftFree(deepest);

  // iftVoxel u_orig = iftGetVoxelCoord(of_map, p_orig);

  return (float)closest * PI / 180;
}

iftMatrix *MntAngles(iftImage *skel, iftImage *of_map, iftSet **S)
{
  int n = iftSetSize(*S);
  iftMatrix *M = iftCreateMatrix(2, n);
  for (int i = 0; i < n; i++)
  {
    iftMatrixElem(M, 0, i) = -1;
    iftMatrixElem(M, 1, i) = -1;
  }

  int c = 0;
  while (*S != NULL)
  {
    int p = iftRemoveSet(S);

    float theta = -100;
    int cn = crossingNumber(skel, p);

    if (cn >= 3)
    {
      theta = GetBifurcAngle(skel, of_map, p);
      if (theta == -1)
      {
        continue;
      }
    }
    else if (cn == 1)
    {
      theta = getTermAngle(skel, p);
    }

    if (theta == -100)
      continue;

    int q = SkelToOrigIndex(skel, of_map, p);

    iftMatrixElem(M, 0, c) = q;
    iftMatrixElem(M, 1, c) = theta;

    // iftVoxel v = iftGetVoxelCoord(of_map, q);

    c++;
  }

  // iftWriteImageByExt(skel, "tmp_skel.png");

  return M;
}

void RemoveCloseByMnt(iftImage *orig, iftMatrix *M_mnt)
{
  int n_removed = 0;
  int n_total = 0;
  for (int i = 0; i < M_mnt->nrows; i++)
  {
    int mnt_i = iftMatrixElem(M_mnt, 0, i);
    if (mnt_i == -1)
      continue;

    n_total++;
    iftVoxel v = iftGetVoxelCoord(orig, mnt_i);

    bool removed = false;

    for (int j = i + 1; j < M_mnt->nrows; j++)
    {
      int mnt_j = iftMatrixElem(M_mnt, 0, j);
      iftVoxel u = iftGetVoxelCoord(orig, mnt_j);

      if ((mnt_j == -1) || (iftMatrixElem(M_mnt, 0, i) == -1))
        continue;

      // if i and j map to the same point, remove only j
      if ((v.x == u.x) && (v.y == u.y))
      {
        iftMatrixElem(M_mnt, 0, j) = -1;
        continue;
      }

      float d = sqrtf(iftPowerOfTwo((float)v.x - (float)u.x) + iftPowerOfTwo((float)v.y - (float)u.y));

      if (d < MIN_MNT_PROXIMITY)
      {

        iftMatrixElem(M_mnt, 0, j) = -1;
        iftMatrixElem(M_mnt, 0, i) = -1;

        removed = true;

        // printf("removing mnts (%d, %d) and (%d, %d) because %f < d\n", v.x,v.y, u.x, u.y, d);
      }
    }
  }

  int valid_mnt = 0;

  for (int i = 0; i < M_mnt->nrows; i++)
  {
    if (iftMatrixElem(M_mnt, 0, i) != -1)
    {
      valid_mnt += 1;
      int q = iftMatrixElem(M_mnt, 0, i);
      iftVoxel v = iftGetVoxelCoord(orig, q);
    }
  }

  // printf("removed %d/%d close by mnt\n", n_total-valid_mnt, n_total);
}

// Function to check if two vectors are within the given angle threshold
int areVectorsAligned(double ax, double ay, double bx, double by, double maxAngle)
{
  // Compute dot product
  double dotProduct = ax * bx + ay * by;

  // Compute magnitudes (norms) of the vectors
  double magnitudeA = sqrt(ax * ax + ay * ay);
  double magnitudeB = sqrt(bx * bx + by * by);

  // Avoid division by zero
  if (magnitudeA == 0 || magnitudeB == 0)
  {
    return 0; // A zero vector cannot be aligned with anything
  }

  // Compute cosine of the angle between the vectors
  double cosTheta = dotProduct / (magnitudeA * magnitudeB);

  // Compute cosine of the threshold angle
  double cosMaxAngle = cos(DEG_TO_RAD(maxAngle));

  // Check if the angle between the vectors is within the allowed range
  return cosTheta >= cosMaxAngle;
}

void RemoveShortBreaks(iftImage *orig, iftMatrix *M_mnt)
{
  int n_removed = 0;
  int n_total = 0;
  for (int i = 0; i < M_mnt->nrows; i++)
  {
    int mnt_i = iftMatrixElem(M_mnt, 0, i);
    if (mnt_i == -1)
      continue;

    n_total++;
    iftVoxel v = iftGetVoxelCoord(orig, mnt_i);

    bool removed = false;

    for (int j = i + 1; j < M_mnt->nrows; j++)
    {
      int mnt_j = iftMatrixElem(M_mnt, 0, j);
      iftVoxel u = iftGetVoxelCoord(orig, mnt_j);

      if ((mnt_j == -1) || (iftMatrixElem(M_mnt, 0, i) == -1))
        continue;

      // if i and j map to the same point, remove only j
      if ((v.x == u.x) && (v.y == u.y))
      {
        iftMatrixElem(M_mnt, 0, j) = -1;
        continue;
      }

      float d = sqrtf(iftPowerOfTwo((float)v.x - (float)u.x) + iftPowerOfTwo((float)v.y - (float)u.y));

      if (d < (int)(MIN_MNT_PROXIMITY * 2))
      {
        float angle_i = iftMatrixElem(M_mnt, 1, i);

        float angle_j = iftMatrixElem(M_mnt, 1, j);

        if (areVectorsAligned(cos(angle_i), sin(angle_i), -cos(angle_j), -sin(angle_j), 15))
        {
          iftMatrixElem(M_mnt, 0, j) = -1;
          iftMatrixElem(M_mnt, 0, i) = -1;

          removed = true;
        }

        // printf("removing mnts (%d, %d) and (%d, %d) because %f < d\n", v.x,v.y, u.x, u.y, d);
      }
    }
  }

  int valid_mnt = 0;

  for (int i = 0; i < M_mnt->nrows; i++)
  {
    if (iftMatrixElem(M_mnt, 0, i) != -1)
    {
      valid_mnt += 1;
      int q = iftMatrixElem(M_mnt, 0, i);
      iftVoxel v = iftGetVoxelCoord(orig, q);
    }
  }

  // printf("removed %d/%d close by mnt\n", n_total-valid_mnt, n_total);
}

iftImage *GetSkelMask(iftImage *skel)
{
  float asf_close_open = 25.0 * RESCALE_FACTOR / 2;
  iftImage *mask = iftAsfCOBin(skel, asf_close_open);
  iftImage *cbasins = iftCloseBasins(mask, NULL, NULL);
  iftDestroyImage(&mask);

  return (cbasins);
}

// Normalize angle to [0, 360)
double normalize_angle(double angle)
{
  angle = fmod(angle, 360.0);
  if (angle < 0)
    angle += 360.0;
  return angle;
}

// Direction difference between two angles
double direction_diff(double a1, double a2)
{
  a1 = normalize_angle(a1);
  a2 = normalize_angle(a2);

  // Reduce to direction modulo 180 (i.e., same line)
  double d1 = fmod(a1, 180.0);
  double d2 = fmod(a2, 180.0);

  double diff = fabs(d1 - d2);
  if (diff > 90)
    diff = 180 - diff; // smallest distance on circle

  return diff;
}

void RemoveBridges(iftImage *skel, iftImage *of_map)
{
  iftFIFO *queue = iftCreateFIFO(skel->n);
  iftAdjRel *A = iftCircular(sqrt(2));
  iftImage *distance = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);
  iftImage *parent = iftCreateImage(skel->xsize, skel->ysize, skel->zsize);

  int max_dist = MIN_MNT_PROXIMITY * RESCALE_FACTOR;

  iftSet *closeByBifurcs = NULL;

  for (int p = 0; p < skel->n; p++)
  {
    // for each bifurcation detected
    if (crossingNumber(skel, p) == 3)
    {
      // find all bifurcations in a radius d

      iftResetFIFO(queue);

      // Initialize for the source pixel
      iftInsertFIFO(queue, p);

      for (int i = 0; i < distance->n; i++)
      {
        distance->val[i] = IFT_INFINITY_INT;
        parent->val[i] = -1;
      }

      distance->val[p] = 0;

      while (!iftEmptyFIFO(queue))
      {
        int curr = iftRemoveFIFO(queue);

        if ((crossingNumber(skel, curr) == 3) && distance->val[curr] > 0)
        {
          iftUnionSetElem(&closeByBifurcs, curr);
          continue; // Do not explore neighbors of found bifurcation point
        }

        // Explore neighbors
        iftVoxel u = iftGetVoxelCoord(skel, curr);
        for (int i = 1; i < A->n; i++)
        {
          iftVoxel v = iftGetAdjacentVoxel(A, u, i);
          if (iftValidVoxel(skel, v))
          {
            int q = iftGetVoxelIndex(skel, v);

            // Check if neighbor is not visited
            if ((queue->color[q] == IFT_WHITE) && (skel->val[q] > 0))
            {
              distance->val[q] = distance->val[curr] + 1;
              parent->val[q] = curr;

              if (distance->val[q] < max_dist)
              {
                iftInsertFIFO(queue, q);
              }
            }
          }
        }
      }

      iftVoxel u = iftGetVoxelCoord(skel, p);

      int p_ = SkelToOrigIndex(skel, of_map, p);
      // iftVoxel u_ = iftGetVoxelCoord(of_map, p_);

      while (closeByBifurcs != NULL)
      {
        int q = iftRemoveSet(&closeByBifurcs);

        iftVoxel v = iftGetVoxelCoord(skel, q);

        float theta = angle_between_points(u.x, u.y, v.x, v.y);
        if (theta < 0)
          theta = 360 + theta;
        float theta_of = 180 * ((float)of_map->val[p_] / 255.0);
        if (theta_of < 0)
          theta_of = 360 + theta_of;

        int q_ = SkelToOrigIndex(skel, of_map, q);
        // iftVoxel v_ = iftGetVoxelCoord(of_map, q_);

        float diff = direction_diff(theta, theta_of + 90);

        if (diff < 25.0 && v.x >= u.x)
        {
          // TODO: delete line that connects p to q
          // printf("Detected bridge at (%d, %d) (%d %d): abs(%.1f - %.1f) = %.1f\n", u.x, u.y, v.x, v.y, theta, theta_of, diff);

          // delete path
          int cur = q;
          while (!(cur == p))
          {
            int prev = parent->val[cur];
            if ((cur != q) && (cur != p))
            {
              skel->val[cur] = 0; // Paint path black
            }
            cur = prev;
          }
        }
        else
        {
          // printf("Allowed bridge at (%d, %d) (%d %d): abs(%.1f - %.1f) = %.1f\n", u.x, u.y, v.x, v.y, theta, theta_of, diff);
        }
      }
    }
  }

  // iftWriteImageByExt(skel, "tmp_skel.png");

  iftDestroyFIFO(&queue);
  iftDestroyAdjRel(&A);
  iftDestroyImage(&distance);
  iftDestroyImage(&parent);
}

int main(int argc, char *argv[])
{
  size_t mem_start, mem_end;
  timer *t1, *t2;
  int no_replace = false; // Flag for checking existing files

  mem_start = iftMemoryUsed();

  // Check if user provided flags
  if (argc < 5 || argc > 7)
  {
    printf("Usage: iftMntFromSkel <P1> <P2> <P3> [<P4>] [--no-replace]\n");
    printf("P1: input folder with skeletons\n");
    printf("P2: input folder with orientation map images\n");
    printf("P3: output folder with minutiae points\n");
    printf("P4: (optional) input folder with mask images\n");
    printf("--no-replace (optional): skip processing files that have been processed before\n");
    exit(-1);
  }

  // Parse the "--no-replace" flag if present
  if (argc == 6 && strcmp(argv[5], "--no-replace") == 0)
  {
    no_replace = true;
  }

  t1 = iftTic();

  /* Read input parameters */

  iftFileSet *fs = iftLoadFileSetFromDirBySuffix(argv[1], ".png", 1);
  char *of_map_dir = argv[2];
  char *mnt_dir = argv[3];

  char *mask_dir = argv[4];

  printf("Selected skeleton dir with %d files.\n", fs->n);

  iftMakeDir(mnt_dir);

  int progress = 0;

  int total = fs->n;

  // int num_threads = omp_get_num_procs();  // Get number of available cores
  // omp_set_num_threads(4);       // Use all cores

  /* Select minutiae from skeleton */
  #pragma omp parallel for schedule(static, 50)
  for (int i = 0; i < fs->n; i++)
  {
    char filename[200];
    char *basename = iftFilename(fs->files[i]->path, ".png");

    // printf("aqui1\n");

    // Generate the output file path
    sprintf(filename, "%s/%s.mnt", mnt_dir, basename);
    // Check if the output file exists and skip if --no-replace flag is set
    if (no_replace && fileExists(filename))
    {
      // printf("Skipping already processed file: %s\n", filename);
      progress++;
      iftFree(basename);
      continue;
    }

    // printf("reading %s\n", basename);

    // printf("aqui2\n");
    iftImage *skel, *of_map, *mask;

    skel = iftReadImageByExt(fs->files[i]->path);


    sprintf(filename, "%s/%s.png", of_map_dir, basename);

    
    of_map = iftReadImageByExt(filename);



    mask = NULL;

    // sprintf(filename, "%s/%s.png", mask_dir, basename);
    // mask = iftReadImageByExt(filename);

    iftImage *skel_mask = GetSkelMask(skel);

    removeBugPixels(skel);

    RemoveBridges(skel, of_map);

    iftSet *S = GetSkelPoints(skel, of_map, skel_mask);

    sprintf(filename, "%s/%s.mnt", mnt_dir, basename);

    // Process the minutiae as normal
    iftMatrix *M_ang = MntAngles(skel, of_map, &S);

    RemoveShortBreaks(of_map, M_ang);

    RemoveCloseByMnt(of_map, M_ang);

    writeMntAngleFromMatrix(of_map, M_ang, filename);

    // Clean up
    iftDestroyImage(&of_map);
    iftDestroyImage(&skel);
    if (mask != NULL)
      iftDestroyImage(&mask);
    iftDestroyImage(&skel_mask);
    iftDestroyMatrix(&M_ang);

    /* Progress tracking */
    #pragma omp atomic
    progress++;

    if (omp_get_thread_num() == 0 || progress % 10 == 0) { // reduce IO contention
      printf("\rProgress: %d/%d (%.1f%%)", progress, total, 100.0 * progress / total);
      fflush(stdout);
    } 

    iftFree(basename);
  }

  iftDestroyFileSet(&fs);

  mem_end = iftMemoryUsed();
  iftVerifyMemory(mem_start, mem_end);

  t2 = iftToc();
  puts(iftFormattedTime(iftCompTime(t1, t2)));

  return 0;
}
