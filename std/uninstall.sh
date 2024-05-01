echo "Uninstalling Oak..."

echo "Removing packages..."
sudo rm -rf /usr/include/oak

echo "Removing executables..."
sudo rm -rf /usr/bin/acorn /usr/bin/oak2c \
    /usr/bin/acorn-db /usr/bin/oak2c-db

echo "Done uninstalling Oak. To re-install Oak, run:"
echo "\`git clone https://github.com/jorbDehmel/oak && \\"
echo "  make -C oak install packages ; rm -rf oak\`"
