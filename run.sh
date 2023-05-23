ORICUTRON_PATH="/mnt/c/Users/plifp/OneDrive/oric/oricutron_wsl/oricutron"

cl65 -ttelestrat src/gunzip.c -o gunzip

cp gunzip $ORICUTRON_PATH/sdcard/bin

cd $ORICUTRON_PATH
./oricutron
cd -

