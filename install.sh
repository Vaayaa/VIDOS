#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

echo "WELCOME TO VIDOS."
echo "INSTALLING..."

git submodule update --init --recursive
sudo apt-get install libsoil-dev libasound2-dev
make

echo "SETTING VIDOS TO RUN ON STARTUP."
echo "$DIR/main.out"  | sudo tee -a filename /home/pi/.bashrc
./main.out