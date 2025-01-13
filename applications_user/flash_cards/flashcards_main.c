#include <stdbool.h>
#include <input/input.h>
#include <gui/canvas.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <furi.h>
#include <storage/storage.h>
#include "singly_linked_list.h"

#define TAG                   "flashcards"
#define GENERATE_TEST_DATA    0
#define FILE_READ_BUFFER_SIZE (const uint32_t)1024
typedef struct {
    char const* key;
    char const* value;
} flashcard_record_t;

#if GENERATE_TEST_DATA
static const flashcard_record_t s_flash_cards[] = {
    {"links", "left"},
    {"rechts", "right"},
    {"boven", "up"},
    {"beneden", "down"},
};
#endif

static singly_linked_list_head_t s_list;

static int parse_flash_cards_data(File* file) {
    int ret = -1;
    furi_assert(storage_file_is_open(file));
    char* buffer = (char*)malloc(FILE_READ_BUFFER_SIZE);
    if(buffer) {
        uint64_t const size = storage_file_size(file);
        if(size < FILE_READ_BUFFER_SIZE) {
            storage_file_read(file, buffer, size);
            FURI_LOG_I(TAG, "read file %s", buffer);
        } else {
            FURI_LOG_W(TAG, "TODO: read files and truncate them");
        }
        bool key = true;
        bool val = false;
        singly_linked_list_init(&s_list);
        flashcard_record_t* rec = malloc(sizeof(flashcard_record_t));
        singly_linked_list_t* cur = malloc(sizeof(singly_linked_list_t));
        if(!rec || !cur) {
            return -1;
        }
        FURI_LOG_I(TAG, "starting loop");

        for(size_t index = 0u; index < size; index++) {
            FURI_LOG_I(TAG, "loop iteration %u, buffer %c", index, buffer[index]);
            if(key) {
                rec->key = &buffer[index];
                key = false;
            }
            if(val) {
                rec->value = &buffer[index];
                val = false;
            }
            if(buffer[index] == ',') {
                buffer[index] = '\0';
                key = false;
                val = true;
            }
            if(buffer[index] == '\n') {
                buffer[index] = '\0';
                key = true;
                val = false;
                cur->data = (void*)rec; /* rec ownership to list item */
                singly_linked_list_add(&s_list, cur); /* list iten ownership to list */
                if(index < (size - 1)) {
                    rec = malloc(sizeof(flashcard_record_t));
                    cur = malloc(sizeof(singly_linked_list_t));
                    if((rec == NULL) || (cur == NULL)) {
                        /*TODO:free list*/
                        return -1;
                    }
                }
            }
        }
        ret = 0;
    }
    return ret;
}

int32_t flashcards_main(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, "starting");
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
#if GENERATE_TEST_DATA
    if(!storage_file_open(file, APP_DATA_PATH("flash_cards.csv"), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Failed to open file");
    }
    for(size_t index = 0u; index < sizeof(s_flash_cards) / sizeof(*s_flash_cards); index++) {
        char const* key = s_flash_cards[index].key;
        char const* val = s_flash_cards[index].value;
        storage_file_write(file, key, strlen(key));
        storage_file_write(file, ",", 1);
        storage_file_write(file, val, strlen(val));
        storage_file_write(file, "\n", 1);
    }
#endif
    if(!storage_file_open(file, APP_DATA_PATH("flash_cards.csv"), FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open file");
        return -1;
    }
    FURI_LOG_I(TAG, "parsing flash cards");
    int ret = parse_flash_cards_data(file);
    if(ret == 0) {
        FURI_LOG_I(TAG, "getting records");
        for(size_t index = 0u; index < s_list.count; ++index) {
            flashcard_record_t* current = signly_linked_list_get_data_at_index(&s_list, index);
            FURI_LOG_I(TAG, "key %s value is %s\n", current->key, current->value);
        }
    }
    storage_file_close(file);

    // Deallocate file
    storage_file_free(file);

    // Close storage
    furi_record_close(RECORD_STORAGE);

    return 0;
}
