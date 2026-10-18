This is an attempt to shrink the SAM (speech synthesizer) code to fit embedded system with limited resources.

Currently the code needs a large output buffer (hundreds of kB) to place there a PCM signal.

I'll try to use a smaller buffer and generate the output in chunks

------------------------------------------------------------------

To produce the sample WAV output, do

SAM_shrink -wav i_am_sam.wav I am Sam
