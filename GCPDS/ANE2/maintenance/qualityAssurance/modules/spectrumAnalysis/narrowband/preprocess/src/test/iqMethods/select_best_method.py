#!/usr/bin/env python3
import json,sys,math

# Simple selector: maximize (accuracy_score / time_ms)
# accuracy_score derived from SNR or RMSE in metrics

if len(sys.argv) < 2:
    print('Usage: select_best_method.py metrics.json --out chosen.json')
    sys.exit(2)

metrics_file = sys.argv[1]
out_file = 'chosen_method.json'
if '--out' in sys.argv:
    out_file = sys.argv[sys.argv.index('--out')+1]

m = json.load(open(metrics_file))
# Expect m to be list of method dicts with keys: name, mean_time_ms, snr_db or rmse
best = None
best_score = -1
for method in m:
    # derive accuracy: prefer higher snr, else lower rmse
    if 'snr_db' in method:
        accuracy = 10**(method['snr_db']/20.0)
    elif 'rmse' in method:
        accuracy = 1.0/(method['rmse']+1e-12)
    else:
        accuracy = 1.0
    time_ms = method.get('mean_time_ms', method.get('median_ms', 1.0))
    score = accuracy / (time_ms + 1e-9)
    if score > best_score:
        best_score = score
        best = method

if best is None:
    print('No methods found in metrics file')
    sys.exit(2)

json.dump({'chosen': best, 'score': best_score}, open(out_file,'w'), indent=2)
print('Chosen method:', best.get('name','(unknown)'), 'score=', best_score)
