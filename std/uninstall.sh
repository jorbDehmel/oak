echo "Uninstalling Oak..."

echo -n "Remove packages as well [y/N]? "
read response

if [ "$response" == y ] ; then
    echo "Uninstalling packages..."
    sudo rm -rf /usr/include/oak
else
    echo "NOT uninstalling packages."
    sudo rm -rf /usr/include/oak/std /usr/include/oak/packages_list.txt /usr/include/oak/std_oak_header.h
fi

sudo rm -rf /usr/bin/acorn

echo "Done uninstalling Oak."
