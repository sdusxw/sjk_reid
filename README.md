# sjk_reid

## prepair list

### libyuv

git clone https://github.com/lemenkov/libyuv.git

### libiconv

    wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz

    tar xfz libiconv-1.14.tar.gz

    cd libiconv-1.14

    ./configure --prefix=/usr/local/libiconv

    make

    make install

    cd ..

reference
    http://www.mamicode.com/info-detail-1368777.html

## compile

  1. mkdir build
  2. cd build && cmake ../src
  3. make
