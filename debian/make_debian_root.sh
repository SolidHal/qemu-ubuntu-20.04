#! /bin/sh
# Script to make a debian root image.
set -e

KEEPFS=0
if [ "$1" = "-k" ]; then
    KEEPFS=1
    shift
fi

if [ $# -lt 4 ]; then
    echo Usage: "%s [-k] size-in-MB distrib deburl image [files_to_copy_in_/root]" >&2
    echo "eg %s 150 sid http://proxy:10000/debian qemu" >&2
    exit 1
fi

SIZE=$1
DISTRO=$2
URL=$3
IMAGE=$4
shift 4

# now files to copy are in "$@".  We don't put them in a variable
# because that would coufuse spaces-in-filenames with
# whitespace-separation.


if [ $SIZE -lt 130 ]; then
    echo 'Size must be at least 130 megabytes (Debian unstable takes 100)' >&2
    exit 1
fi

cleanup()
{
    echo Cleaning up... >&2
    umount -d /tmp/mount.$$ || true
    rm -f $IMAGE.ext2 $IMAGE
}

trap cleanup EXIT

HEADS=16
SECTORS=63
# 512 bytes in a sector: cancel the 512 with one of the 1024s...
CYLINDERS=$(( $SIZE * 1024 * 2 / ($HEADS * $SECTORS) ))

# Create a filesystem: one track for partition table.
dd bs=$(($SECTORS * 512)) if=/dev/zero of=$IMAGE.ext2 count=$(($CYLINDERS * $HEADS - 1))
mke2fs -q -m1 -F $IMAGE.ext2

# Mount it.
mkdir /tmp/mount.$$
mount -o loop $IMAGE.ext2 /tmp/mount.$$

# Do debian install on it.
#debootstrap --exclude=syslinux,at,exim,mailx,libstdc++2.10-glibc2.2,mbr,setserial,fdutils,info,ipchains,lilo,pcmcia-cs,ppp,pppoe,pppoeconf,pppconfig $DISTRO /tmp/mount.$$ $URL
debootstrap --exclude=syslinux,at,exim,mailx,libstdc++2.10-glibc2.2,mbr,setserial,fdutils,info,ipchains,iptables,lilo,pcmcia-cs,ppp,pppoe,pppoeconf,pppconfig,wget,telnet,cron,logrotate,exim4,exim4-base,exim4-config,exim4-daemon-light,pciutils,modconf,tasksel --include=aptitude,libsigc++-1.2-5c102 $DISTRO /tmp/mount.$$ $URL

# Final configuration.
cat > /tmp/mount.$$/etc/fstab <<EOF
/dev/hda1 / ext2 errors=remount-ro 0 1
proc /proc proc defaults 0 0
EOF

# Console on ttyS0, not tty1, and no other gettys.
sed 's,1:2345:respawn:/sbin/getty 38400 tty1,1:2345:respawn:/sbin/getty 38400 ttyS0,' < /tmp/mount.$$/etc/inittab | sed 's,^.:23:respawn.*,,' > /tmp/mount.$$/etc/inittab.new
mv /tmp/mount.$$/etc/inittab.new /tmp/mount.$$/etc/inittab

# Set hostname to base of image name.
basename $IMAGE > /tmp/mount.$$/etc/hostname

# Create /etc/shadow
chroot /tmp/mount.$$ pwconv

# Set root password to "root"
sed 's/^root:[^:]*/root:$1$aybpiIGf$cB7iFDNZvViQtQjEZ5HFQ0/' < /tmp/mount.$$/etc/shadow > /tmp/mount.$$/etc/shadow.new
mv /tmp/mount.$$/etc/shadow.new /tmp/mount.$$/etc/shadow

# Remove packages we don't need
chroot /tmp/mount.$$ /usr/bin/dpkg --remove console-common aptitude console-tools console-data base-config man-db manpages
# Try to remove all libraries: some won't be removable.
chroot /tmp/mount.$$ dpkg --remove `chroot /tmp/mount.$$ dpkg --get-selections | sed -n 's/^\(lib[^ \t]*\)[\t ]*install/\1/p'` 2>/dev/null || true


# Copy wanted files to /root if asked to
if [ $# -gt 0 ]; then
    cp -a "$@" /tmp/mount.$$/root/
fi
umount -d /tmp/mount.$$

# Create file with partition table.
uudecode -o- << "EOF" | gunzip > $IMAGE
begin 664 partition-table.gz
M'XL("*_<##\"`W!A<G1I=&EO;BUT86)L90#LT#$-`"`0!,&']D6A`D6XP1T&
M"%B@))FIMKGF(OA9C;%;EENYZO.Z3P\"````!P``__\:!0````#__QH%````
M`/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%
M`````/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0````#_
M_QH%`````/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0``
M``#__QH%`````/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:
M!0````#__QH%`````/__&@4`````__\:!0````#__QH%`````/__&@4`````
M__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%`````/__&@4`
M````__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%`````/__
M&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%````
M`/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%
M`````/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0````#_
M_QH%`````/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0``
M``#__QH%`````/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:
M!0````#__QH%`````/__&@4`````__\:!0````#__QH%`````/__&@4`````
M__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%`````/__&@4`
M````__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%`````/__
M&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%````
M`/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0````#__QH%
M`````/__&@4`````__\:!0````#__QH%`````/__&@4`````__\:!0````#_
M_QH%`````/__&@4`````__\:!0````#__QH%`````/__0@``````__\#`%&_
&<90`?@``
`
end
EOF
cat $IMAGE.ext2 >> $IMAGE
rm $IMAGE.ext2

# Repartition so one partition covers entire disk.
echo '63,' | sfdisk -uS -H$HEADS -S$SECTORS -C$CYLINDERS $IMAGE

trap "" EXIT

echo Done.
