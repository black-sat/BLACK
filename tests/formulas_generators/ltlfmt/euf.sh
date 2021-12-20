#!/bin/bash

#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2021 Luca Geatti
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

help() {
 cat <<HELP
Usage ./$(basename "$0") <startindex> <endindex>

Generates formulas for EUF theory for N ∈ {<startindex> .. <endindex>}
HELP
}

START="$1"
END="$2"
BIN="../../../build/tests/euf"
OUTDIR="../../../benchmarks/formulas/ltlf-modulo-theory/EUF/scalable_1"

for (( i=$START; i<=$END; i=i+1 ))
do
  $BIN "1" $i > "$OUTDIR/scalable-1-$i.ltlfmt"
  echo "Created benchmark #$i for Category 1 and Theory EUF"
done

