#!/usr/bin/env python3
import json,sys
py_metrics = json.load(open(sys.argv[1]))
c_metrics = json.load(open(sys.argv[2]))
print('Python metrics summary:')
print(py_metrics)
print('C metrics summary:')
print(c_metrics)
# User should implement specific comparisons (time per sample, RSS, accuracy vs golden outputs)
