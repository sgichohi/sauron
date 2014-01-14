#!/bin/sh

echo "Initialising the Camera on port 5001"

cdir=$PWD

echo $cdir
dir=$cdir"/web-interface/build/output"

echo "Getting images from" $dir

cd camera/build

make

./camera_do_nothing 5 5001 -d $dir & 

cd ..

cd ..


cd web-interface

#cd build

#dir=`pwd`

#./camera_do_nothing 5 6001 -d '$dir /output' &

echo "getting the webserver up on port 5000"

#cd ..

python camera.py 5001 5000 &

cd ..