#SSA2DVD : SSA/ASS to DVD subtitle converter
##Installation
steps :

```bash
$ cd ssa2dvd
$ make
$ su -c "cp build/ssa2dvd /bin" (or /usr/bin)
```

##What does it do
ssa2dvd reads a SSA/ASS subtitle file and produces a list of png files with an xml file. You can then use the xml file with the `spumux` command  (from the `dvdauthor` package) to add the png files as a subtitle stream to a mpeg file.

Note that when issuing the `spumux` command, you have to be in the same directory as the xml file.

##How to use it
```bash
$ ssa2dvd -s subtitle_file -w video_width -h video height -o output_directory
```

* subtitle_file : Path to a valid SSA/ASS subtitle file
* video_width : Width of the MPEG file.
* video_height : Height of the MPEG file.
* output_directory : Where to put the pngs and the xml file.

##Example
Suppose we have a MPEG file `test.mpg` and a SSA/ASS subtitle `sub.ssa`.

```bash
$ mkdir ~/out
$ ssa2dvd -s sub.ssa -w 720 -h 480 -o ~/out/
$ cd ~/out
$ spumux -m dvd dvd_sub.xml < /path/to/test.mpg > test_with_sub.mpg
```

You can use then `test_with_sub.mpg` with `dvdauthor` to produce a DVD that you can watch on TV.



