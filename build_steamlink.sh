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

export DESTDIR="${PWD}/steamlink/apps/spotify"

# Copy the files to the app directory
mkdir -p "${DESTDIR}"
cp -v testspriteminimal "${DESTDIR}"
cp -v spotify3.bmp "${DESTDIR}"
cp -v consolas.ttf "${DESTDIR}"
cp -v spotify4steamlink.sh "${DESTDIR}"
cp -v icon2.png "${DESTDIR}/icon.png" 

# Set spotify4steamlink.sh executable
chmod +x "${DESTDIR}/spotify4steamlink.sh"

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
