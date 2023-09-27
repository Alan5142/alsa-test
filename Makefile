CC ?= gcc

ALSA_RECORD = alsa_record
ALSA_RECORD_OBJ_FILES = alsa_record.o

ALSA_PLAY = alsa_play
ALSA_PLAY_OBJ_FILES = alsa_play.o

LIBS = .

FLAGS ?= -Wpedantic -Os -lasound

all: $(ALSA_RECORD) $(ALSA_PLAY)

$(ALSA_RECORD): $(ALSA_RECORD_OBJ_FILES)
	$(CC) -o $@ $^ $(FLAGS)

$(ALSA_PLAY): $(ALSA_PLAY_OBJ_FILES)
	$(CC) -o $@ $^ $(FLAGS)

%.o: %.c
	$(CC) -o $@ -c $^ -g $(FLAGS)

clean:
	rm *.o $(APP_NAME)