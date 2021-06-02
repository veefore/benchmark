# Copyright © 2021 Vladimir Erofeev. All rights reserved.


class Pattern:
    def __init__(self):
        self.is_consecutive = 0 
        self.is_read = 0 

class Throughput:
    def __init__(self):
        self.mean = 0 
        self.std = 0 

class Measurement:
    def __init__(self):
        self.throughput = Throughput()
        self.factors = dict()

class Result:
    def __init__(self):
        self.pattern = Pattern()
        self.measurements = []

def parse_pattern(f):
    pattern = Pattern()
    pattern.is_consecutive = int(f.readline())
    pattern.is_read = int(f.readline())
    return pattern


def parse_factor(f):
    factor = f.readline().rstrip("\n")
    level = int(f.readline())
    return factor, level


def parse_throughput(f):
    throughput = Throughput()
    throughput.mean = int(f.readline())
    throughput.std = int(f.readline())
    return throughput


def parse_measurement(f, factorsCnt):
    measurement = Measurement()
    for i in range(factorsCnt):
        factor, level = parse_factor(f)
        measurement.factors[factor] = level
    measurement.throughput = parse_throughput(f)
    return measurement


def parse_result(f):
    result = Result()
    result.pattern = parse_pattern(f)
    measurementsCnt = int(f.readline())
    factorsCnt = int(f.readline())
    for i in range(measurementsCnt):
        result.measurements.append(parse_measurement(f, factorsCnt))
    return result
