make PLAT=win32 LUAV=5.2 LUAINC_win32='c:\cygwin\home\diego\build\include' LUALIB_win32='c:\cygwin\home\diego\build\bin\release'

#!/bin/sh
for p in Release Debug x64/Release x64/Debug; do
    for el in mime socket; do
        for e in dll lib; do
            cp $p/$el/core.$e ../bin/$p/$el/
        done;
    done;
    cp src/ltn12-lanes.lua src/socket-lanes.lua src/mime-lanes.lua ../bin/$p/
    cp src/http-lanes.lua src/url-lanes.lua src/tp-lanes.lua src/ftp-lanes.lua src/headers-lanes.lua src/smtp-lanes.lua ../bin/$p/socket/
done;
