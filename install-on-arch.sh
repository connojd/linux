#!/bin/bash

if [ $# -ne 3 ];
then
    echo "Please supply:"
    echo "    \$1 = major revision"
    echo "    \$2 = minor revision"
    echo "    \$3 = .config local version (without the dash)"
    exit
fi

make -j`grep -c ^processor /proc/cpuinfo`
sudo make modules_install

sudo cp -v arch/x86_64/boot/bzImage /boot/vmlinuz-linux$1$2-$3

# make initrd
sudo cp /etc/mkinitcpio.d/linux.preset /etc/mkinitcpio.d/linux$1$2-$3.preset
sudo sed -e "s/-linux/-linux$1$2-$3/" -i /etc/mkinitcpio.d/linux$1$2-$3.preset
sudo mkinitcpio -p linux$1$2-$3

#sudo mv /boot/initramfs-linux$1$2-$3.img /boot/efi/EFI/xen/
#sudo mv /boot/vmlinuz-linux$1$2-$3 /boot/efi/EFI/xen/

# copy system.map
#sudo cp System.map /boot/System.map-linux$1$2-$3
#sudo ln -sf /boot/System.map-linux$1$2-$3 /boot/System.map

# update grub.cfg
sudo grub-mkconfig -o /boot/grub/grub.cfg
