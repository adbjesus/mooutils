#!/usr/bin/env python3

import os
import subprocess
import sys
import numpy as np
import argparse
import pathlib

def filter_nondom(points):
    ps = []
    for i in range(len(points)):
        nondom = True
        for j in range(i):
            if np.all(points[j] >= points[i]):
                nondom = False
                break
        if nondom:
            for j in range(i+1, len(points)):
                if np.all(points[j] >= points[i]) and not np.all(points[j] == points[i]):
                    nondom = False
                    break
        if nondom:
            ps.append(points[i])
    return ps

def generate_ndpoints(n, m, low, high):
    ps = np.random.normal(size = n * m).reshape((n, m))
    ps = np.apply_along_axis(lambda p : np.abs(p / np.sqrt(np.sum(p**2))), 1, ps)
    ps = np.round(ps * (high - low) + low)
    ps = filter_nondom(ps)
    ps = [list(map(int, p)) for p in ps]
    return ps

def generate_refp(m, low, high):
    return list(map(int, np.random.randint(low, high, size = m)))

def get_hv(points, refp, hvbin):
    # This assumes minimization, so we need to get the negative for ref and points
    points_stdin = '\n'.join(map(lambda p: ' '.join(map(str, p)), np.negative(points)))
    refp_arg = ' '.join(map(str, np.negative(refp)))
    args = [hvbin, "-r", refp_arg]
    p = subprocess.run(args, input=points_stdin.encode(), capture_output=True)
    return int(float(p.stdout.decode().strip()))

# Generate non-dominated points, compute its hypervolume, and save data
# to a file
def generate_to_file(n, m, seed, plow, phigh, rlow, rhigh, filename, hvbin):
    np.random.seed(seed)
    points = generate_ndpoints(n, m, plow, phigh)
    refp = generate_refp(m, rlow, rhigh)
    hv = get_hv(points, refp, hvbin)

    ptostring = lambda p: ' '.join(map(str, p))
    with open(filename, 'w') as f:
        f.write(f"{len(points)} {m}\n")
        f.write(f"{hv}\n")
        f.write(ptostring(refp))
        f.write("\n")
        f.write('\n'.join(map(ptostring, points)))
        f.write("\n")

    print(f"Data written to {fname}.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('hvbin', type=pathlib.Path, help='path to hv executable')
    args = parser.parse_args()

    rlow = -10
    rhigh = 0
    plow = 1
    # Note: (phigh-rlow)^m should fit inside a double without
    # approximation issues since the hv library uses double
    phighs = [10000, 1000, 100, 50]
    ms = [2, 3, 5, 7]
    ns = [10, 50, 100, 200]

    datadir = os.path.join(os.getcwd(), "data")
    try:
        os.mkdir(datadir)
    except FileExistsError:
        pass

    for n in ns:
        for (m, phigh) in zip(ms, phighs):
            print(n, m, phigh)
            for seed in range(10):
                fname = os.path.join(datadir, f"hv_{n}_{m}_{seed}.dat")
                generate_to_file(n, m, seed, plow, phigh, rlow, rhigh, fname, args.hvbin)
