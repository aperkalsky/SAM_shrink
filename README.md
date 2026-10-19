SAM_shrink is a compact SAM speech synthesizer intended for systems with limited memory.

Audio is generated incrementally. Call `SAM_Begin(text)`, then repeatedly call
`SAM_GenerateChunk(pcm, capacity, &produced)` and consume the samples returned
until it reports `SAM_STATUS_DONE`. Call `SAM_Cancel()` to abandon a session.

The desktop test program accepts text arguments and writes the generated audio
to `sam_stream.wav` in the current directory. With no arguments, it synthesizes
“I am Sam”. The WAV is 8-bit, mono PCM at 22050 Hz.
