#!/bin/bash
#

TOP=$(cd `dirname "${BASH_SOURCE[0]}"` && pwd)
if [ "${MARVELL_SDK_PATH}" = "" ]; then
	MARVELL_SDK_PATH="$(cd "${TOP}/../.." && pwd)"
fi
if [ "${MARVELL_ROOTFS}" = "" ]; then
	source "${MARVELL_SDK_PATH}/setenv.sh" || exit 1
fi
cd "${TOP}"

make $MAKE_J || exit 2

export DESTDIR="${PWD}/steamlink/apps/spotify4steamlink"

# Check that the librespot binaries are provided
if [ ! -f ${PWD}/librespot-org-build/arm-unknown-linux-gnueabihf/release/librespot ]; then
	echo "librespot binary is missing!"
	echo "Please provide it in the 'librespot-org-build' folder"
	echo "Exiting"
	exit 10
fi

# Copy the files to the app directory
mkdir -p "${DESTDIR}"
cp -v main "${DESTDIR}"
cp -v spotify3.bmp "${DESTDIR}"
cp -v consolas.ttf "${DESTDIR}"
cp -v spotify4steamlink.sh "${DESTDIR}"
cp -v icon2.png "${DESTDIR}/icon.png" 
cp -rv ${PWD}/librespot-org-build "${DESTDIR}/librespot-org-build"

# Set spotify4steamlink.sh executable
chmod +x "${DESTDIR}/spotify4steamlink.sh"

# Set the librespot binaries executable
chmod +x "${DESTDIR}/librespot-org-build/arm-unknown-linux-gnueabihf/release/librespot"

# Create the table of contents and icon
cat >"${DESTDIR}/toc.txt" <<__EOF__
name=Spotify
icon=icon.png
run=spotify4steamlink.sh
__EOF__

# Pack it up
name=$(basename ${DESTDIR})
pushd "$(dirname ${DESTDIR})"
tar zcvf $name.tgz $name || exit 3
rm -rf $name
popd

# All done!
echo "Build complete!"
echo
echo "Put the steamlink folder onto a USB drive, insert it into your Steam Link, and cycle the power to install."
