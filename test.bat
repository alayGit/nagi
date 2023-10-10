make
OSFMount -a -t file -f "C:\Commander\sdcard.img" -o rw -m E:
copy sbe.cx16* "E:"
copy sbe.cx16* "C:\Commander"
timeout 3
OSFMount -D -m E:
cd C:\Commander\ 
x16emu.exe -sdcard "sdcard.img" -prg "sbe.cx16" -run -debug
cd C:\meka\ForumDemoC\

