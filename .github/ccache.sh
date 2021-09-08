OS=$(uname)
CCACHE_VER=4.4

case $OS in
  Darwin )
    brew install ccache
    ln -s $(which ccache) ./ccache
    ;;
  Linux )
    sudo apt-get install -y ccache
    ln -s $(which ccache) ./ccache
    ;;
  * )
    curl -OL https://github.com/ccache/ccache/releases/download/v${CCACHE_VER}/ccache-${CCACHE_VER}-windows-64.zip
    unzip -j ccache-*-windows-64.zip '*/ccache.exe'
    ;;
esac
mkdir ./.ccache
./ccache -o compiler_check='%compiler% -dumpmachine; %compiler% -dumpversion'
