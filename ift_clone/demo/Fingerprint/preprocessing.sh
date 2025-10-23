if [ $# -ne 8 ];
then
  echo "Usage $0: Missing arguments"
  echo "1: Folder with fingerprint images"
  echo "2: Folder with gabor images"
  echo "3: Input FPModel parameters"
  echo "4: output folder with gabor masks"
  echo "5: output folder with binarized gabor"
  echo "6: output folder with skeleton"
  echo "7: output folder with minuciae points"
  echo "8: output folder with drawn minuciae"
  exit 1
fi
echo "##############################################"
echo "Extracting Gabor Mask..."
iftGaborMask $2 $3 $4

echo "##############################################"
echo "Binarizing Gabor..."
iftGaborBin $2 $4 $3 $5

echo "##############################################"
echo "Multi-scale skeletonization..."
iftSkeleton $5 $6

echo "##############################################"
echo "Detecting minucia points..."
iftMntFromSkel $6 $4 $3 tmp_seeds

echo "##############################################"
echo "Detecting minuciae angles..."
iftMntAngle tmp_seeds $6 $7

echo "##############################################"
echo "Drawing minuciaes..."
iftDrawMinuciae $5 $7 $8 0

rm -rf tmp_seeds
