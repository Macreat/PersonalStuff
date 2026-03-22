#!/usr/bin/env python3
import subprocess, json, os, sys, time

# Run C binary with /usr/bin/time and optionally perf to collect metrics
def run_c_binary(bin_path, args, out_json):
    # use /usr/bin/time -v to collect max RSS and elapsed
    time_cmd = ['/usr/bin/time','-v', bin_path] + args
    print('Running:', ' '.join(time_cmd))
    p = subprocess.run(time_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout = p.stdout.decode()
    stderr = p.stderr.decode()
    # parse stderr for 'Elapsed (wall clock) time' or 'Elapsed (wall clock) time' different on systems
    metrics = {'stdout': stdout, 'stderr': stderr}
    # save raw
    with open(out_json,'w') as f: json.dump(metrics,f)
    return metrics

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: metrics_collector.py <bin> <out_json> [-- args]')
        sys.exit(2)
    bin_path = sys.argv[1]
    out_json = sys.argv[2]
    args = sys.argv[3:]
    run_c_binary(bin_path, args, out_json)
