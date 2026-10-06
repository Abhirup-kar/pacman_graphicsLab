#pragma once

#include <QByteArray>
#include <QVector>
#include <QtGlobal>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
#endif

struct WavSound {
    QByteArray data;
    void play() const {
#ifdef _WIN32
        if (!data.isEmpty()) {
            PlaySoundA(reinterpret_cast<LPCSTR>(data.constData()), NULL, SND_MEMORY | SND_ASYNC);
        }
#endif
    }
};

class SoundManager {
public:
    static WavSound makeWav(const QVector<double>& freqs, const QVector<double>& durations, double volume = 0.4, bool square = true) {
        const uint32_t sampleRate = 22050;
        int totalSamples = 0;
        for (double d : durations) totalSamples += int(sampleRate * d);

        uint32_t dataSize = totalSamples;
        uint32_t chunkSize = 36 + dataSize;

        QByteArray buf;
        buf.reserve(44 + dataSize);

        buf.append("RIFF", 4);
        buf.append(reinterpret_cast<const char*>(&chunkSize), 4);
        buf.append("WAVEfmt ", 8);
        uint32_t subchunk1Size = 16;
        uint16_t audioFormat = 1, numChannels = 1;
        uint32_t sr = sampleRate, byteRate = sampleRate;
        uint16_t blockAlign = 1, bitsPerSample = 8;
        buf.append(reinterpret_cast<const char*>(&subchunk1Size), 4);
        buf.append(reinterpret_cast<const char*>(&audioFormat), 2);
        buf.append(reinterpret_cast<const char*>(&numChannels), 2);
        buf.append(reinterpret_cast<const char*>(&sr), 4);
        buf.append(reinterpret_cast<const char*>(&byteRate), 4);
        buf.append(reinterpret_cast<const char*>(&blockAlign), 2);
        buf.append(reinterpret_cast<const char*>(&bitsPerSample), 2);
        buf.append("data", 4);
        buf.append(reinterpret_cast<const char*>(&dataSize), 4);

        const double PI = 3.14159265358979323846;
        for (int note = 0; note < freqs.size(); ++note) {
            double freq = freqs[note];
            double duration = durations[note];
            int nSamples = int(sampleRate * duration);
            for (int i = 0; i < nSamples; ++i) {
                uint8_t sample = 128;
                if (freq > 0) {
                    double t = double(i) / sampleRate;
                    double phase = fmod(t * freq, 1.0);
                    double val = square ? ((phase < 0.5) ? 1.0 : -1.0) : sin(2.0 * PI * phase);
                    double env = 1.0;
                    if (i > nSamples - 100) env = double(nSamples - i) / 100.0;
                    sample = static_cast<uint8_t>(qBound(0, int(128 + val * 127.0 * volume * env), 255));
                }
                buf.append(static_cast<char>(sample));
            }
        }
        return {buf};
    }

    static WavSound makeSweep(double startFreq, double endFreq, double duration, double volume = 0.4, bool square = true) {
        const uint32_t sampleRate = 22050;
        int totalSamples = int(sampleRate * duration);
        uint32_t dataSize = totalSamples;
        uint32_t chunkSize = 36 + dataSize;

        QByteArray buf;
        buf.reserve(44 + dataSize);

        buf.append("RIFF", 4);
        buf.append(reinterpret_cast<const char*>(&chunkSize), 4);
        buf.append("WAVEfmt ", 8);
        uint32_t subchunk1Size = 16;
        uint16_t audioFormat = 1, numChannels = 1;
        uint32_t sr = sampleRate, byteRate = sampleRate;
        uint16_t blockAlign = 1, bitsPerSample = 8;
        buf.append(reinterpret_cast<const char*>(&subchunk1Size), 4);
        buf.append(reinterpret_cast<const char*>(&audioFormat), 2);
        buf.append(reinterpret_cast<const char*>(&numChannels), 2);
        buf.append(reinterpret_cast<const char*>(&sr), 4);
        buf.append(reinterpret_cast<const char*>(&byteRate), 4);
        buf.append(reinterpret_cast<const char*>(&blockAlign), 2);
        buf.append(reinterpret_cast<const char*>(&bitsPerSample), 2);
        buf.append("data", 4);
        buf.append(reinterpret_cast<const char*>(&dataSize), 4);

        const double PI = 3.14159265358979323846;
        double phaseAcc = 0.0;
        for (int i = 0; i < totalSamples; ++i) {
            double progress = double(i) / totalSamples;
            double freq = startFreq + (endFreq - startFreq) * progress;
            phaseAcc += freq / sampleRate;
            double phase = fmod(phaseAcc, 1.0);
            double val = square ? ((phase < 0.5) ? 1.0 : -1.0) : sin(2.0 * PI * phase);
            double env = (i > totalSamples - 200) ? double(totalSamples - i) / 200.0 : 1.0;
            uint8_t sample = static_cast<uint8_t>(qBound(0, int(128 + val * 127.0 * volume * env), 255));
            buf.append(static_cast<char>(sample));
        }
        return {buf};
    }
};
