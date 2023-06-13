
typedef struct Clip
{
    float *samples;
    int sample_count;
} Clip;

typedef struct LoadedFile
{
    uint8_t* data;
    size_t size;
} LoadedFile;

LoadedFile LoadFile(const char *fileName)
{
    LoadedFile loadedFile;
    loadedFile.data = NULL;
    loadedFile.size = 0;

    FILE *f = fopen(fileName, "rb");
    if (f == NULL)
    {
        printf("Failed to open file: %s\n", fileName);
        return loadedFile;
    }

    fseek(f, 0, SEEK_END);
    loadedFile.size = ftell(f);
    fseek(f, 0, SEEK_SET);

    loadedFile.data = (uint8_t *)malloc(loadedFile.size);
    if (loadedFile.data == NULL)
    {
        fclose(f);
        printf("Failed to allocate memory for file: %s\n", fileName);
        return loadedFile;
    }

    fread(loadedFile.data, loadedFile.size, 1, f);
    fclose(f);
    return loadedFile;
}

Clip LoadSoundClip(const char *fileName)
{
    LoadedFile loadedFile = LoadFile(fileName);
    Clip clip;
    clip.samples = NULL;
    clip.sample_count = 0;

    if (loadedFile.data != NULL)
    {
        uint8_t *file = loadedFile.data;
        size_t size = loadedFile.size;

        // Check if the file starts with the WAV file header
        if (file[0] == 'R' && file[1] == 'I' && file[2] == 'F' && file[3] == 'F' &&
            file[8] == 'W' && file[9] == 'A' && file[10] == 'V' && file[11] == 'E')
        {
            // Find the start of the audio data
            int dataStart = -1;
            for (int i = 12; i < 100; i++)
            {
                if (file[i] == 'd' && file[i + 1] == 'a' && file[i + 2] == 't' && file[i + 3] == 'a')
                {
                    dataStart = i + 8;
                    break;
                }
            }

            if (dataStart != -1)
            {
                // Calculate the number of samples
                size_t dataSize = size - dataStart;
                int sampleCount = dataSize / sizeof(float);

                // Allocate memory for the samples
                float *samples = (float *)malloc(dataSize);
                if (samples != NULL)
                {
                    // Copy the samples from the file
                    memcpy(samples, file + dataStart, dataSize);

                    // Set the values in the Clip struct
                    clip.samples = samples;
                    clip.sample_count = sampleCount;
                }
                else
                {
                    printf("Failed to allocate memory for samples.\n");
                }
            }
            else
            {
                printf("Failed to find audio data in the file: %s\n", fileName);
            }
        }
        else
        {
            printf("Invalid WAV file: %s\n", fileName);
        }

        free(file); // Free the memory allocated by LoadFile()
    }
    else
    {
        printf("Failed to load file: %s\n", fileName);
    }

    return clip;
}

class GSound
{
private:
    /* data */
public:
    double Time;
    float Amp = 0;
    float Freq;
    float Volume;
    bool AmpDecrease = false;

    Clip *clip;
    int curentClipFrame;

    GSound(float freq, float volume, bool ampDecrease);
};
GSound::GSound(float freq, float volume, bool ampDecrease)
{
    Freq = freq;
    Volume = volume;
    AmpDecrease = ampDecrease;
}
