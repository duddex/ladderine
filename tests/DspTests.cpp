// Copyright (C) 2026 duddex
// SPDX-License-Identifier: AGPL-3.0-or-later

/*
    Ladderine - offline DSP checks.

    Renders the voice without JUCE and checks the fundamentals: tuning,
    ladder self-oscillation, stability at extreme settings, envelope release
    and filter brightness. Also writes WAV files so the sound can be listened
    to outside a DAW.

    Build (MSVC, from the project root):
        cl /std:c++20 /O2 /EHsc /I src tests\DspTests.cpp src\engine\SynthEngine.cpp
    or with any C++17 compiler.
*/
#include "engine/SynthEngine.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
constexpr double kSampleRate = 48000.0;
int failures = 0;

void check (bool condition, const char* what, const std::string& detail = {})
{
    std::printf ("%s  %s%s%s\n",
                 condition ? "PASS" : "FAIL",
                 what,
                 detail.empty() ? "" : "  (",
                 detail.empty() ? "" : (detail + ")").c_str());

    if (! condition)
        ++failures;
}

void writeWav (const std::string& path, const std::vector<float>& samples)
{
    FILE* f = std::fopen (path.c_str(), "wb");

    if (f == nullptr)
        return;

    const uint32_t dataBytes = (uint32_t) samples.size() * 2u;
    const uint32_t rate = (uint32_t) kSampleRate;

    auto write32 = [f] (uint32_t v)
    {
        const unsigned char b[4] { (unsigned char) (v & 255u), (unsigned char) ((v >> 8) & 255u),
                                   (unsigned char) ((v >> 16) & 255u), (unsigned char) ((v >> 24) & 255u) };
        std::fwrite (b, 1, 4, f);
    };
    auto write16 = [f] (uint16_t v)
    {
        const unsigned char b[2] { (unsigned char) (v & 255u), (unsigned char) ((v >> 8) & 255u) };
        std::fwrite (b, 1, 2, f);
    };

    std::fwrite ("RIFF", 1, 4, f);
    write32 (36u + dataBytes);
    std::fwrite ("WAVEfmt ", 1, 8, f);
    write32 (16u);
    write16 (1u);
    write16 (1u);
    write32 (rate);
    write32 (rate * 2u);
    write16 (2u);
    write16 (16u);
    std::fwrite ("data", 1, 4, f);
    write32 (dataBytes);

    for (float s : samples)
    {
        const int v = (int) std::lround (std::tanh (s) * 32767.0f);
        write16 ((uint16_t) (int16_t) std::max (-32768, std::min (32767, v)));
    }

    std::fclose (f);
}

struct Run
{
    std::vector<float> samples;
    float peak = 0.0f;
    float rms = 0.0f;
    bool finite = true;
};

Run render (ladderine::SynthEngine& engine, ladderine::EngineParams params, int note, double seconds,
            double noteOffAt = -1.0)
{
    engine.setParams (params);

    const int total = (int) (seconds * kSampleRate);
    const int offAt = noteOffAt >= 0.0 ? (int) (noteOffAt * kSampleRate) : total;

    Run run;
    run.samples.resize ((size_t) total);

    engine.noteOn (note, 1.0f);

    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < total; i += 32)
    {
        const int n = std::min (32, total - i);

        if (i >= offAt)
            engine.noteOff (note);

        engine.render (run.samples.data() + i, n);

        for (int k = 0; k < n; ++k)
        {
            const float s = run.samples[(size_t) i + (size_t) k];

            if (! std::isfinite (s))
                run.finite = false;

            run.peak = std::max (run.peak, std::abs (s));
            sum += (double) s * s;
            ++count;
        }
    }

    run.rms = count > 0 ? (float) std::sqrt (sum / count) : 0.0f;
    return run;
}

double estimateFrequency (const std::vector<float>& x, double from, double to)
{
    const int a = (int) (from * kSampleRate);
    const int b = std::min ((int) x.size(), (int) (to * kSampleRate));

    int crossings = 0;
    float prev = x[(size_t) a];

    for (int i = a + 1; i < b; ++i)
    {
        const float cur = x[(size_t) i];

        if (prev <= 0.0f && cur > 0.0f)
            ++crossings;

        prev = cur;
    }

    return (double) crossings / ((double) (b - a) / kSampleRate);
}

ladderine::EngineParams baseParams()
{
    ladderine::EngineParams p;
    p.oscOn[0] = true;
    p.oscOn[1] = false;
    p.oscOn[2] = false;
    p.oscWave[0] = ladderine::WaveSaw;
    p.oscRange[0] = ladderine::Range8;
    p.oscLevel[0] = 0.8f;
    p.noiseLevel = 0.0f;
    p.drive = 0.0f;
    p.cutoffHz = 12000.0f;
    p.emphasis = 0.0f;
    p.contour = 0.0f;
    p.kbdTrack = ladderine::KbdTrackOff;
    p.ampEnv = { 0.002f, 0.2f, 1.0f, 0.1f };
    p.filterEnv = { 0.002f, 0.2f, 0.0f, 0.1f };
    p.glideMode = ladderine::GlideOff;
    p.glideSeconds = 0.0f;
    p.modWheel = 0.0f;
    p.volumeDb = 0.0f;
    p.tuneSemis = 0.0f;
    p.drift = 0.0f;
    return p;
}
} // namespace

int main (int argc, char** argv)
{
    const std::string outDir = argc > 1 ? argv[1] : ".";

    // ---------------------------------------------------------------- tuning
    {
        ladderine::SynthEngine engine;
        engine.prepare (kSampleRate);

        auto run = render (engine, baseParams(), 69, 1.5);
        const double hz = estimateFrequency (run.samples, 0.3, 1.3);

        check (run.finite, "tuning: output is finite");
        check (run.peak > 0.05f, "tuning: output is audible", "peak " + std::to_string (run.peak));
        check (std::abs (hz - 440.0) < 4.0, "tuning: A4 = 440 Hz",
               std::to_string (hz) + " Hz");
        writeWav (outDir + "/01_tuning_a4.wav", run.samples);
    }

    // ------------------------------------------------------------- one octave
    {
        ladderine::SynthEngine engine;
        engine.prepare (kSampleRate);

        auto run = render (engine, baseParams(), 81, 1.5);
        const double hz = estimateFrequency (run.samples, 0.3, 1.3);

        check (std::abs (hz - 880.0) < 8.0, "tuning: A5 = 880 Hz", std::to_string (hz) + " Hz");
    }

    // ---------------------------------------------------- filter self-oscillation
    {
        ladderine::SynthEngine engine;
        engine.prepare (kSampleRate);

        auto p = baseParams();
        p.oscOn[0] = false;
        p.cutoffHz = 800.0f;
        p.emphasis = 1.0f;
        p.ampEnv = { 0.002f, 0.2f, 1.0f, 0.2f };
        p.filterEnv = { 0.002f, 0.2f, 0.0f, 0.2f };

        auto run = render (engine, p, 60, 2.0);
        const double hz = estimateFrequency (run.samples, 0.8, 1.8);

        check (run.finite, "self-oscillation: output is finite");
        check (run.peak > 0.02f, "self-oscillation: the ladder sings", "peak " + std::to_string (run.peak));
        check (hz > 640.0 && hz < 960.0, "self-oscillation: near 800 Hz", std::to_string (hz) + " Hz");
        writeWav (outDir + "/02_self_oscillation.wav", run.samples);
    }

    // ------------------------------------------------------- filter brightness
    {
        ladderine::SynthEngine engine;
        engine.prepare (kSampleRate);

        auto dark = baseParams();
        dark.cutoffHz = 300.0f;
        dark.ampEnv = { 0.002f, 0.2f, 1.0f, 0.1f };
        dark.filterEnv = { 0.002f, 0.2f, 0.0f, 0.1f };

        auto bright = dark;
        bright.cutoffHz = 8000.0f;

        const auto d = render (engine, dark, 48, 1.0);
        const auto b = render (engine, bright, 48, 1.0);

        check (b.rms > d.rms * 1.15f, "filter: opening the cutoff adds energy",
               "dark " + std::to_string (d.rms) + ", bright " + std::to_string (b.rms));

        writeWav (outDir + "/03_dark.wav", d.samples);
        writeWav (outDir + "/04_bright.wav", b.samples);
    }

    // ------------------------------------------------------------- release tail
    {
        ladderine::SynthEngine engine;
        engine.prepare (kSampleRate);

        auto p = baseParams();
        p.ampEnv = { 0.002f, 0.2f, 0.8f, 0.15f };

        auto run = render (engine, p, 55, 1.5, 0.5);

        const int tailStart = (int) (1.3 * kSampleRate);
        float tailPeak = 0.0f;

        for (int i = tailStart; i < (int) run.samples.size(); ++i)
            tailPeak = std::max (tailPeak, std::abs (run.samples[(size_t) i]));

        check (run.finite, "release: output is finite");
        check (tailPeak < 1.0e-4f, "release: the tail decays to silence",
               "tail peak " + std::to_string (tailPeak));
        writeWav (outDir + "/05_release.wav", run.samples);
    }

    // ----------------------------------------------------------- stress testing
    {
        const float cutoffs[] { 20.0f, 250.0f, 1200.0f, 8000.0f, 20000.0f };
        const float emphases[] { 0.0f, 0.6f, 1.0f };
        int cases = 0;

        for (float cut : cutoffs)
        {
            for (float emph : emphases)
            {
                ladderine::SynthEngine engine;
                engine.prepare (kSampleRate);

                auto p = baseParams();
                p.cutoffHz = cut;
                p.emphasis = emph;
                p.contour = 1.0f;
                p.drive = 1.0f;
                p.oscLevel[0] = 1.0f;
                p.oscLevel[1] = 1.0f;
                p.noiseLevel = 1.0f;
                p.oscOn[0] = p.oscOn[1] = true;
                p.oscWave[0] = ladderine::WavePulse;
                p.oscWave[1] = ladderine::WaveSaw;
                p.oscRange[1] = ladderine::Range16;
                p.oscTune[1] = 7.0f;

                auto run = render (engine, p, 96, 0.8);

                if (! run.finite || run.peak > 4.0f)
                {
                    check (false, "stress: stable at extreme settings",
                           "cutoff " + std::to_string (cut) + ", emphasis " + std::to_string (emph)
                               + ", peak " + std::to_string (run.peak));
                }

                ++cases;
            }
        }

        check (true, "stress: extreme renders stay finite",
               std::to_string (cases) + " cases");
    }

    // --------------------------------------------------------------- glide check
    {
        ladderine::SynthEngine engine;
        engine.prepare (kSampleRate);

        auto p = baseParams();
        p.glideMode = ladderine::GlideOn;
        p.glideSeconds = 0.2f;
        p.ampEnv = { 0.002f, 0.2f, 1.0f, 0.1f };

        // start on A3 (57), then slide to A3 (69); estimate the pitch halfway
        std::vector<float> buffer ((size_t) (1.2 * kSampleRate));
        engine.setParams (p);
        engine.noteOn (57, 1.0f);

        for (int i = 0; i < (int) buffer.size(); i += 32)
        {
            const int n = std::min (32, (int) buffer.size() - i);

            if (i == (int) (0.3 * kSampleRate))
                engine.noteOn (69, 1.0f);

            engine.render (buffer.data() + i, n);
        }

        const double early = estimateFrequency (buffer, 0.05, 0.25);
        const double late = estimateFrequency (buffer, 0.9, 1.1);

        check (std::abs (early - 220.0) < 6.0, "glide: starts on the old note",
               std::to_string (early) + " Hz");
        check (std::abs (late - 440.0) < 8.0, "glide: arrives on the new note",
               std::to_string (late) + " Hz");
        writeWav (outDir + "/06_glide.wav", buffer);
    }

    // ------------------------------------------------------------- musical demo
    {
        ladderine::SynthEngine engine;
        engine.prepare (kSampleRate);

        auto p = baseParams();
        p.oscOn[1] = true;
        p.oscLevel[1] = 0.6f;
        p.oscTune[1] = 0.08f;
        p.cutoffHz = 900.0f;
        p.emphasis = 0.45f;
        p.contour = 0.6f;
        p.ampEnv = { 0.004f, 0.5f, 0.75f, 0.2f };
        p.filterEnv = { 0.01f, 0.45f, 0.15f, 0.2f };
        p.glideMode = ladderine::GlideOff;

        engine.setParams (p);

        const int notes[] { 40, 43, 45, 47, 45, 43, -1, 40, 47, 50, 52 };
        std::vector<float> buffer ((size_t) (4.0 * kSampleRate));
        int written = 0;
        int noteIndex = 0;
        double noteTime = 0.0;

        while (written < (int) buffer.size())
        {
            const int n = std::min (32, (int) buffer.size() - written);
            const int note = notes[noteIndex % (int) (sizeof (notes) / sizeof (notes[0]))];

            if (noteTime <= 0.0)
            {
                if (note < 0)
                    engine.noteOff (40);
                else
                    engine.noteOn (note, 0.9f);

                noteTime = 0.22;
                ++noteIndex;
            }

            engine.render (buffer.data() + written, n);
            written += n;
            noteTime -= (double) n / kSampleRate;
        }

        writeWav (outDir + "/07_demo.wav", buffer);
        check (true, "demo rendered to 07_demo.wav");
    }

    std::printf ("\n%s\n", failures == 0 ? "All DSP checks passed."
                                        : (std::to_string (failures) + " DSP check(s) failed.").c_str());
    return failures == 0 ? 0 : 1;
}


