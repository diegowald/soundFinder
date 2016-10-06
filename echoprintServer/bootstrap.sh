#!/usr/bin/env bash

apt update
apt install -y zlib1g-dev 
apt install -y libbz2-dev 
apt install -y gcc
apt install -y make
apt install -y python-setuptools
easy_install web.py
easy_install pyechonest
apt install -y openjdk-8-jre-headless 

wget http://sourceforge.net/projects/tokyocabinet/files/tokyocabinet/1.4.32/tokyocabinet-1.4.32.tar.gz
tar xzvf tokyocabinet-1.4.32.tar.gz 
cd tokyocabinet-1.4.32/

mkdir /usr/local/tokyocabinet-1.4.21/
./configure --prefix=/usr/local/tokyocabinet-1.4.21/
./configure --prefix=/usr/local/tokyocabinet-1.4.21/
make
make install

cd
wget http://sourceforge.net/projects/tokyocabinet/files/tokyotyrant/1.1.33/tokyotyrant-1.1.33.tar.gz
tar xzvf tokyotyrant-1.1.33.tar.gz 
cd tokyotyrant-1.1.33/
sudo mkdir /usr/local/tokyotirant-1.1.33
./configure --prefix=/usr/local/tokyotirant-1.1.33/ --with-tc=/usr/local/tokyocabinet-1.4.21/
make
make install
cd /usr/local/tokyotirant-1.1.33/
ln -s /usr/local/tokyocabinet-1.4.21/lib/libtokyocabinet.so.8 lib/
/usr/local/tokyotirant-1.1.33/bin/ttserver &

cd
mkdir echoprint
cd echoprint/
git init
git pull git://github.com/echonest/echoprint-server.git
cd solr/solr
java -Dsolr.solr.home=~/echoprint/solr/solr/solr/ -Djava.awt.headless=true -jar start.jar &

