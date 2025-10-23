import os
import sys

      
if (len(sys.argv) != 3):
    print("iftFingerprint <folder> <0: latent/1: reference>")
    exit()
    
os.system("ls -v {} > temp.txt".format(sys.argv[1]))
f = open("temp.txt","r")
for line in f:
    filename = line.strip()
    os.system("iftFingerprint {}/{} {}".format(sys.argv[1],filename,sys.argv[2]))
              
f.close()
