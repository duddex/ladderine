# Copyright (C) 2026 duddex
# SPDX-License-Identifier: AGPL-3.0-or-later

import wave, numpy as np, glob, os

def load(p):
    with wave.open(p, 'rb') as w:
        n = w.getnframes()
        sr = w.getframerate()
        x = np.frombuffer(w.readframes(n), dtype='<i2').astype(np.float64) / 32768
    return x, sr

for p in sorted(glob.glob('tests/out/*.wav')):
    x, sr = load(p)
    peak = np.max(np.abs(x))
    rms = np.sqrt(np.mean(x * x))
    dc = np.mean(x)
    print(f"{os.path.basename(p):24s} peak {peak:.3f} rms {rms:.4f} dc {dc:+.5f}")

x, sr = load('tests/out/01_tuning_a4.wav')
seg = x[int(0.5 * sr):int(1.4 * sr)]
w = np.hanning(len(seg))
X = np.abs(np.fft.rfft(seg * w))
f = np.fft.rfftfreq(len(seg), 1 / sr)

def band(a, b):
    m = (f >= a) & (f < b)
    return np.sqrt(np.sum(X[m] ** 2))

sel = (f > 400) & (f < 480)
fund = f[sel][np.argmax(X[sel])]
print("A4 peak bin", fund)

harm = sum(band(440 * h - 20, 440 * h + 20) ** 2 for h in range(1, 11))
total = band(20, 20000) ** 2
print("harmonic share 1..10: %.4f" % (harm / total))

mask = (f > 2000) & (f < 8000)
for h in range(1, 120):
    mask &= np.abs(f - 440 * h) > 40
print("inharmonic 2-8 kHz rel total: %.6f" % (np.sum(X[mask] ** 2) / total))
