#!/bin/bash

rm -rf soil.sock run.sh
make clean
make

directory=$(pwd)

cat > "run.sh" <<EOF
cd $directory
./main 1>out.log 2>err.log&
python3 pread.py
EOF
chmod +x run.sh

# place the script into the /opt directory
mkdir -p /opt/soil
cp run.sh /opt/soil/run.sh

# place the service into the systemd directory
cp soil.service /etc/systemd/system/soil.service

# enable and start the service
systemctl enable soil.service
systemctl start soil.service
