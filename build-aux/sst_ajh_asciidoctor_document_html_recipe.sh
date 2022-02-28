#! /bin/sh -
#
# Copyright (C) Stealth Software Technologies, Inc.
#
# For the complete copyright information, please see the
# associated README file.
#

eval ASCIIDOCTOR_FLAGS='${ASCIIDOCTOR_FLAGS?}'
eval MAKE='${MAKE:?}'
eval SED='${SED:?}'
eval TSUF=${TSUF:?}
eval dst=${dst:?}
eval imagesdir=${imagesdir:?}
eval prefix=${prefix:?}
eval slug=${slug:?}
eval srcdir=${srcdir:?}

readonly ASCIIDOCTOR_FLAGS
readonly MAKE
readonly SED
readonly TSUF
readonly dst
readonly imagesdir
readonly prefix
readonly slug
readonly srcdir

x=
x="${x?} -a imagesdir=${imagesdir?}"
x="${x?} ${ASCIIDOCTOR_FLAGS?}"
eval " ${MAKE?}"' \
  ${slug?}_disable_wrapper_recipe=/x \
  ASCIIDOCTOR_FLAGS="${x?}" \
  ${dst?} \
' || exit $?

#---------------------------------------------------------------
# KaTeX installation
#---------------------------------------------------------------

if test -d ${prefix?}katex; then

  mv -f ${dst?} ${dst?}${TSUF?}1 || exit $?

  x='
    /<script.*[Mm]ath[Jj]ax.*\.js/ d
  '
  eval " ${SED?}"' \
    "${x?}" \
    <${dst?}${TSUF?}1 \
    >${dst?}${TSUF?}2 \
  ' || exit $?

  mv -f ${dst?}${TSUF?}2 ${dst?} || exit $?

fi

#---------------------------------------------------------------
# Fonts installation
#---------------------------------------------------------------

if test -d ${prefix?}fonts; then

  mv -f ${dst?} ${dst?}${TSUF?}1 || exit $?

  x='
    /<link.*fonts\.googleapis\.com/ d
  '
  eval " ${SED?}"' \
    "${x?}" \
    <${dst?}${TSUF?}1 \
    >${dst?}${TSUF?}2 \
  ' || exit $?

  mv -f ${dst?}${TSUF?}2 ${dst?} || exit $?

fi

#---------------------------------------------------------------
