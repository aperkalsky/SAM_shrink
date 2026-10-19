#include <stdio.h>
#include <string.h>
#include "sam.h"

#define DEFAULT_OUTPUT_FILE "sam_stream.wav"
#define PCM_CHUNK_CAPACITY 512
#define TEXT_CAPACITY 254

extern int debug;

static void WriteLe16(FILE *file, unsigned int value)
{
    fputc((int)(value & 0xff), file);
    fputc((int)((value >> 8) & 0xff), file);
}

static void WriteLe32(FILE *file, unsigned long value)
{
    WriteLe16(file, (unsigned int)(value & 0xffff));
    WriteLe16(file, (unsigned int)((value >> 16) & 0xffff));
}

static void WriteWavHeader(FILE *file, unsigned long sampleCount)
{
    fwrite("RIFF", 1, 4, file);
    WriteLe32(file, 36 + sampleCount);
    fwrite("WAVEfmt ", 1, 8, file);
    WriteLe32(file, 16);
    WriteLe16(file, 1);       /* PCM */
    WriteLe16(file, 1);       /* mono */
    WriteLe32(file, 22050);   /* sample rate */
    WriteLe32(file, 22050);   /* byte rate: 8-bit mono */
    WriteLe16(file, 1);       /* block alignment */
    WriteLe16(file, 8);       /* bits per sample */
    fwrite("data", 1, 4, file);
    WriteLe32(file, sampleCount);
}

int main(int argc, char **argv)
{
    char text[TEXT_CAPACITY];
    unsigned char pcm[PCM_CHUNK_CAPACITY];
    size_t textLength = 0;
    unsigned long totalSamples = 0;
    FILE *wav;
    const char *outputFile = DEFAULT_OUTPUT_FILE;
    int arg;
    SAM_Status status;

    text[0] = '\0';
    if (argc <= 1)
    {
        strcpy(text, "I am Sam");
    }
    else
    {
        for (arg = 1; arg < argc; arg++)
        {
            if (strcmp(argv[arg], "-wav") == 0)
            {
                if (arg + 1 >= argc)
                {
                    fprintf(stderr, "Missing filename after -wav.\n");
                    return 1;
                }
                outputFile = argv[++arg];
                continue;
            }
            if (strcmp(argv[arg], "-debug") == 0)
            {
                debug = 1;
                continue;
            }
            size_t wordLength = strlen(argv[arg]);
            size_t separatorLength = textLength > 0 ? 1 : 0;
            if (textLength + separatorLength + wordLength >= sizeof(text))
            {
                fprintf(stderr, "Input text is too long (maximum %u characters).\n",
                    (unsigned int)(sizeof(text) - 1));
                return 1;
            }
            if (separatorLength != 0) text[textLength++] = ' ';
            memcpy(text + textLength, argv[arg], wordLength);
            textLength += wordLength;
            text[textLength] = '\0';
        }
    }

    if (!SAM_Begin(text))
    {
        fprintf(stderr, "Unable to begin SAM synthesis.\n");
        return 1;
    }

    wav = fopen(outputFile, "wb+");
    if (wav == NULL)
    {
        perror("Unable to open WAV output");
        SAM_Cancel();
        return 1;
    }
    WriteWavHeader(wav, 0);

    do
    {
        size_t produced = 0;
        status = SAM_GenerateChunk(pcm, sizeof(pcm), &produced);
        printf("SAM_GenerateChunk() called. res = %d, produced = %zu\n", status, produced);
        if (produced != 0)
        {
            if (fwrite(pcm, 1, produced, wav) != produced)
            {
                fprintf(stderr, "Unable to write WAV output.\n");
                fclose(wav);
                SAM_Cancel();
                return 1;
            }
            totalSamples += (unsigned long)produced;
        }
    } while (status == SAM_STATUS_MORE);

    if (status == SAM_STATUS_ERROR)
    {
        fprintf(stderr, "SAM synthesis failed.\n");
        fclose(wav);
        SAM_Cancel();
        return 1;
    }

    if (fseek(wav, 0, SEEK_SET) != 0)
    {
        fclose(wav);
        return 1;
    }
    WriteWavHeader(wav, totalSamples);
    fclose(wav);
    printf("Wrote %lu PCM samples to %s\n", totalSamples, outputFile);
    return 0;
}
