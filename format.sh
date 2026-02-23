#!/bin/bash
clang-format -i $(find src include -name "*.h" -o -name "*.c")
