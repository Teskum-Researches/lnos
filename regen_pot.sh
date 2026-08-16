#!/bin/bash
set -e

xgettext \
  --language=C++ \
  --keyword=_ \
  --from-code=UTF-8 \
  -o po/lnos.pot \
  lnoscommon/src/*.cpp \
  lnosctl/src/*.cpp \
  lnosd/src/*.cpp