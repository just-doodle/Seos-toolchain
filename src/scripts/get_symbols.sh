#!/bin/bash                                                                                                                             

cat <<EOF
#include "symbols.h"

symbol_t* get_symDB()
{
    static const symbol_t symbols[] = {
EOF

readelf -s $1 | awk '/FUNC/ {printf "\t\t\"%s\", 0x%s, %s,\n",$8, $2, $3;}'
readelf -s $1 | awk '/NOTYPE/ {printf "\t\t\"%s\", 0x%s, %s,\n",$8, $2, 32;}'

cat <<EOF
        "EOS", 0x0, 0x00
    };
    return symbols;
}
EOF

cat << EOF

symbol_t* get_objectSymDB()
{
    static const symbol_t objSymbols[] = {
EOF

readelf -s $1 | awk '/OBJECT/ {printf "\t\t\"%s\", 0x%s, %s,\n",$8, $2, $3;}'

cat <<EOF
        "EOS", 0x0, 0x00
    };
    return objSymbols;
}
EOF