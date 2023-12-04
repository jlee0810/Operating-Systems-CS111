# Hey! I'm Filing Here

In this lab, I successfully implemented the following: 1MiB ext2 file system with 2 directories, 1 regular file, and 1 symbolic link.

## Building

Type the command "make".

## Running

Run the executable ./ext2-create to create the cs111-base.img

Then you can do any of the following:
1) dumpe2fs cs111 -base.img # dumps the filesystem information to help debug

2) fsck.ext2 cs111 -base.img # this will check that your filesystem is correct

3) Mount it to an empty directory
mkdir mnt # create a directory to mnt your filesystem to
sudo mount -o loop cs111 -base.img mnt # mount your filesystem , loop lets you use a file

Then you can run ls -ain mnt/ to see if we get the correct outputs

sudo umount mnt # unmount the filesystem when you 're done
rmdir mnt # delete the directory used for mounting when you 're done

## Cleaning up

Run make clean
