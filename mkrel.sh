#!/usr/bin/env bash
# mkrel.sh - Generate release distributables for Xen

read -p "Version: " VERSION

if [ -z "$VERSION" ]; then
    echo "No version was provided"
    exit 1
fi

STARTUP_DIR=$(pwd)
START_TIME=$(date +%s)

echo "Creating distributables for Xen v$VERSION"

rm -rf build
./generate_build.sh
./build_all.sh

REL_DIR=/home/x/Code/Xen/releases/$VERSION

if [ -e "$REL_DIR" ]; then
    rm -rf $REL_DIR
fi

mkdir -p "$REL_DIR/linux"
mkdir -p "$REL_DIR/windows"
mkdir -p "$REL_DIR/macos"

make_linux_rpm() {
    RPMBUILD_ROOT="/home/x/rpmbuild"
    LINUX_RPM_TAR="$RPMBUILD_ROOT/SOURCES/Xen-$VERSION.tar.gz"

    tar czf $LINUX_RPM_TAR \
        --transform 's,^,Xen-0.5.2/,' \
        --exclude='.git*' \
        --exclude='build' \
        --exclude='obj*' \
        --exclude='bin*' \
        --exclude='*.tar.gz' \
        src/ examples/ generate_build.sh build.sh run.sh build_all.sh \
        LICENSE README.md

    rpmbuild -ba specs/xen.spec

    OUT="$REL_DIR/linux/Xen-$VERSION-linux-x64.rpm"
    find "$RPMBUILD_ROOT/RPMS/x86_64" -name "Xen-$VERSION-*.rpm" ! -name "*debug*" -exec cp {} $OUT \;

    gpg --detach-sign --armor --output "$OUT.sig" $OUT
}

make_linux_deb() {
    DEB_ROOT="$REL_DIR/linux/Xen-$VERSION-linux-x64"
    mkdir -p "$DEB_ROOT/DEBIAN"
    mkdir -p "$DEB_ROOT/usr/bin"
    mkdir -p "$DEB_ROOT/usr/share/xen/examples"
    mkdir -p "$DEB_ROOT/usr/share/doc/xen"

    cp build/linux-release/bin/xen "$DEB_ROOT/usr/bin/"
    cp -r examples "$DEB_ROOT/usr/share/xen/"
    cp README.md LICENSE "$DEB_ROOT/usr/share/doc/xen/"
    cp specs/xen.control "$DEB_ROOT/DEBIAN/control"

    dpkg-deb --build $DEB_ROOT
    rm -rf $DEB_ROOT

    OUT="$REL_DIR/linux/Xen-$VERSION-linux-x64.deb"
    gpg --detach-sign --armor --output "$OUT.sig" $OUT
}

make_linux_tarball() {
    LINUX_BIN_TAR="$REL_DIR/linux/Xen-$VERSION-linux-x64.tar.gz"
    mkdir bin
    cp build/linux-release/bin/xen bin/
    tar -czf $LINUX_BIN_TAR bin/ examples/ README.md LICENSE
    rm -rf bin

    gpg --detach-sign --armor --output "$LINUX_BIN_TAR.sig" $LINUX_BIN_TAR
}

make_windows_zip() {
    WINDOWS_BIN_ZIP="$REL_DIR/windows/Xen-$VERSION-windows-x64.zip"
    mkdir bin
    cp build/windows-release/bin/xen.exe bin/
    zip -r $WINDOWS_BIN_ZIP bin/ README.md LICENSE
    rm -rf bin

    gpg --detach-sign --armor --output "$WINDOWS_BIN_ZIP.sig" $WINDOWS_BIN_ZIP
}

make_windows_exe() {
    FILENAME="Xen-$VERSION-windows-x64.exe"
    WINDOWS_EXE="$REL_DIR/windows/$FILENAME"
    (cd specs && wine64 /home/x/.wine/drive_c/Program\ Files\ \(x86\)/Inno\ Setup\ 6/ISCC.exe xen.iss)
    mv specs/$FILENAME "$REL_DIR/windows/"

    gpg --detach-sign --armor --output "$WINDOWS_EXE.sig" $WINDOWS_EXE
}

make_source_tarball() {
    SOURCE_TAR="$REL_DIR/Xen-$VERSION.tar.gz"
    tar -czf $SOURCE_TAR src/ examples/ specs/ .clang-format README.md LICENSE generate_build.sh CHANGELOG.md mkrel.sh

    gpg --detach-sign --armor --output "$SOURCE_TAR.sig" $SOURCE_TAR
}

make_macos_tarball() {
    MACOS_BIN_TAR="$REL_DIR/macos/Xen-$VERSION-macos-x64.tar.gz"
    mkdir bin
    cp build/macos-release/bin/xen bin/
    tar -czf $MACOS_BIN_TAR bin/ examples/ README.md LICENSE
    rm -rf bin

    gpg --detach-sign --armor --output "$MACOS_BIN_TAR.sig" $MACOS_BIN_TAR
}

make_macos_arm_tarball() {
    MACOS_BIN_TAR="$REL_DIR/macos/Xen-$VERSION-macos-arm64.tar.gz"
    mkdir bin
    cp build/macos-arm-release/bin/xen bin/
    tar -czf $MACOS_BIN_TAR bin/ examples/ README.md LICENSE
    rm -rf bin

    gpg --detach-sign --armor --output "$MACOS_BIN_TAR.sig" $MACOS_BIN_TAR
}

# Linux
make_linux_rpm
make_linux_deb
make_linux_tarball
# Windows
make_windows_zip
make_windows_exe
# macOS
make_macos_tarball
make_macos_arm_tarball
# Source Code
make_source_tarball

# Copy verification key
cp /home/x/public-key.asc "$REL_DIR/public_key.asc"

END_TIME=$(date +%s)

echo ""
echo "=== DONE ==="
echo "Created Xen v$VERSION"
echo "Took $(echo "$END_TIME - $START_TIME" | bc) seconds"
