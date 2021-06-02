# Copyright © 2021 Vladimir Erofeev. All rights reserved.

from parser import *
from plotter import plot_result
import sys


def main():
    argc = len(sys.argv)

    if argc >= 2:
        filepath = sys.argv[1]
    else:
        filepath = input("Result file: ")
    
    f = open(filepath, 'r')
    result = parse_result(f)
    plot_result(result)


if __name__ == "__main__":
    main()
