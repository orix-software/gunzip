ORICUTRON_PATH="/mnt/c/Users/plifp/OneDrive/oric/oricutron_wsl/oricutron"

./make.sh

cp build/bin/gunzip $ORICUTRON_PATH/sdcard/bin

cp run.sh test.sh &&  gzip -c test.sh > test.gz
cp test.gz $ORICUTRON_PATH/sdcard/

cd $ORICUTRON_PATH
./oricutron
cd -

