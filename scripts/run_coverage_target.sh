#!/bin/bash

echo "RUNING TARGETS ..."

find bin -type f -executable | /usr/bin/grep -vE ".*(asio|opencv|benchmark).*" | xargs -P 10 -n 1 -I{} sh -c './"$0" || true' {} 2>/dev/null &>1

echo "ALL COVERAGE TARGETS RUN DONE!"
