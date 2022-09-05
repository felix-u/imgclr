#!/usr/bin/env sh
imgclr="./imgclr"
xresources="$HOME/.Xresources"

get () {
    grep "$1:" "$xresources" | awk '{print $2}' | tr -d \#
}

"$imgclr" "$@" -p \
    "$(get background)" "$(get foreground)" "$(get color1)" "$(get color2)" \
    "$(get color3)" "$(get color4)" "$(get color5)" "$(get color6)" \
    "$(get color7)" "$(get color8)" "$(get color9)" "$(get color10)" \
    "$(get color11)" "$(get color12)" "$(get color13)" "$(get color14)" \
    "$(get color15)"
