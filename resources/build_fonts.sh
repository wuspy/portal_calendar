#!/bin/sh

python build_font.py 'font/Univers 65 Bold Regular.ttf' -name medium -size 28 -ranges !,.-Z,À-Ö,Ø-ß,� -fg=0 -bg=255
python build_font.py 'font/Univers LT 49 Light Ultra Condensed.ttf' -name chamber_number -size 400 -ranges 0-9 -fg=0 -bg=255
python build_font.py 'font/Univers LT 59 Ultra Condensed Regular.otf' -name small -size 24 -ranges !-~,¡-ÿ,� -fg=0 -bg=255
python build_font.py 'font/Univers LT 59 Ultra Condensed Regular.otf' -name weather_frame -size 18 -ranges .-Z,À-Ö,Ø-ß,� -fg=255 -bg=0
