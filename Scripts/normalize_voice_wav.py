"""Trim a generated voice take and give it a readable WAV header.

The OpenAI speech CLI streams its WAV output, so the RIFF and data sizes are
written as 0xFFFFFFFF and the clip cannot be measured without reading it to the
end. Every take also arrives with about half a second of digital silence in
front of the first syllable and a tail behind the last one, which matters here
because these are combat barks that have to land the instant a hit does.

This rewrites the clip as canonical 16-bit PCM with the silence trimmed to a
short breath on each side, so the importer can read a real duration and the
sound fires immediately.

    python3 Scripts/normalize_voice_wav.py in.wav out.wav [lead_ms] [tail_ms]
"""
import struct
import sys
import wave


def read_take(path):
    raw = open(path, "rb").read()
    if raw[:4] != b"RIFF" or raw[8:12] != b"WAVE":
        raise SystemExit(f"{path}: not a RIFF/WAVE file")
    offset = 12
    params = None
    data = None
    while offset + 8 <= len(raw):
        chunk = raw[offset:offset + 4]
        size = struct.unpack("<I", raw[offset + 4:offset + 8])[0]
        body = raw[offset + 8:offset + 8 + size]
        if chunk == b"fmt ":
            params = struct.unpack("<HHIIHH", body[:16])
        elif chunk == b"data":
            # A streamed take declares 0xFFFFFFFF; in that case read to the end.
            data = body if size <= len(raw) - offset - 8 else raw[offset + 8:]
            break
        offset += 8 + size + (size & 1)
    if params is None or data is None:
        raise SystemExit(f"{path}: no fmt or data chunk")
    return params, data


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    source, target = sys.argv[1], sys.argv[2]
    lead_ms = int(sys.argv[3]) if len(sys.argv) > 3 else 30
    tail_ms = int(sys.argv[4]) if len(sys.argv) > 4 else 70
    (format_tag, channels, rate, _byte_rate, _align, bits), data = read_take(source)
    if format_tag != 1 or bits != 16:
        raise SystemExit(f"{source}: expected 16-bit PCM")
    frame = channels * 2
    frames = len(data) // frame
    samples = struct.unpack(f"<{frames * channels}h", data[:frames * frame])
    peak = max(abs(s) for s in samples) if samples else 0
    threshold = max(1, int(peak * 0.03))
    first = next((i for i, s in enumerate(samples) if abs(s) > threshold), 0)
    last = next((i for i, s in enumerate(reversed(samples)) if abs(s) > threshold), 0)
    first = max(0, first - int(rate * lead_ms / 1000) * channels)
    last = min(frames - 1, frames - 1 - last + int(rate * tail_ms / 1000) * channels)
    trimmed = samples[first:last + 1]
    with wave.open(target, "wb") as out:
        out.setnchannels(channels)
        out.setsampwidth(2)
        out.setframerate(rate)
        out.writeframes(struct.pack(f"<{len(trimmed)}h", *trimmed))
    print(f"{source}: {frames / rate:.2f}s -> {target}: {len(trimmed) / channels / rate:.2f}s "
          f"(peak {peak}, threshold {threshold})")


if __name__ == "__main__":
    main()
