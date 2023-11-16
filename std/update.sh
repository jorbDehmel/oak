echo "Updating Oak..."

cd /tmp
if [ oak ] ; then
    rm -rf oak
fi

git clone https://github.com/jorbDehmel/oak
cd oak
make install
cd ..
rm -rf oak

echo "Done updating Oak."
