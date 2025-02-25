
#include <furi.h>
#include <storage/storage.h>
#include <toolbox/dir_walk.h>
#include <toolbox/path.h>

#include "quac.h"
#include "item.h"
#include <m-array.h>

ARRAY_DEF(FileArray, FuriString*, FURI_STRING_OPLIST);

ItemsView* item_get_items_view_from_path(void* context, const FuriString* input_path) {
    App* app = context;

    // Handle the app start condition
    FuriString* in_path;
    if(input_path == NULL) {
        in_path = furi_string_alloc_set_str(APP_DATA_PATH(""));
    } else {
        in_path = furi_string_alloc_set(input_path);
    }
    if(furi_string_get_char(in_path, furi_string_size(in_path) - 1) == '/') {
        furi_string_left(in_path, furi_string_size(in_path) - 1);
    }
    const char* cpath = furi_string_get_cstr(in_path);

    FURI_LOG_I(TAG, "Reading items from path: %s", cpath);
    ItemsView* iview = malloc(sizeof(ItemsView));
    iview->path = furi_string_alloc_set(in_path);

    iview->name = furi_string_alloc();
    if(app->depth == 0) {
        furi_string_set_str(iview->name, QUAC_NAME);
    } else {
        path_extract_basename(cpath, iview->name);
        item_prettify_name(iview->name);
    }

    DirWalk* dir_walk = dir_walk_alloc(app->storage);
    dir_walk_set_recursive(dir_walk, false);

    FuriString* path = furi_string_alloc();
    FileArray_t flist;
    FileArray_init(flist);

    FuriString* filename_tmp;
    filename_tmp = furi_string_alloc();

    // Walk the directory and store all file names in sorted order
    if(dir_walk_open(dir_walk, cpath)) {
        while(dir_walk_read(dir_walk, path, NULL) == DirWalkOK) {
            // FURI_LOG_I(TAG, "> dir_walk: %s", furi_string_get_cstr(path));
            const char* cpath = furi_string_get_cstr(path);

            path_extract_filename(path, filename_tmp, false);
            // Always skip our .quac.conf file!
            if(!furi_string_cmp_str(filename_tmp, QUAC_SETTINGS_FILENAME)) {
                continue;
            }

            // Skip "hidden" files
            char first_char = furi_string_get_char(filename_tmp, 0);
            if(first_char == '.' && !app->settings.show_hidden) {
                // FURI_LOG_I(TAG, ">> skipping hidden file: %s", furi_string_get_cstr(filename_tmp));
                continue;
            }

            // Insert the new file path in sorted order to flist
            uint32_t i = 0;
            FileArray_it_t it;
            for(FileArray_it(it, flist); !FileArray_end_p(it); FileArray_next(it), ++i) {
                if(strcmp(cpath, furi_string_get_cstr(*FileArray_ref(it))) > 0) {
                    continue;
                }
                // FURI_LOG_I(TAG, ">> Inserting at %lu", i);
                FileArray_push_at(flist, i, path);
                break;
            }
            if(i == FileArray_size(flist)) {
                // FURI_LOG_I(TAG, "Couldn't insert, so adding at the end!");
                FileArray_push_back(flist, path);
            }
        }
    }

    furi_string_free(filename_tmp);
    furi_string_free(path);

    // Generate our Item list
    FileArray_it_t iter;
    ItemArray_init(iview->items);
    for(FileArray_it(iter, flist); !FileArray_end_p(iter); FileArray_next(iter)) {
        path = *FileArray_ref(iter);

        Item* item = ItemArray_push_new(iview->items);
        item->is_link = false;
        item->name = furi_string_alloc();

        FileInfo fileinfo;
        if(storage_common_stat(app->storage, furi_string_get_cstr(path), &fileinfo) == FSE_OK &&
           file_info_is_dir(&fileinfo)) {
            item->type = Item_Group;
            path_extract_filename(path, item->name, false);
        } else {
            // Action files have extensions, which determine their type
            item->ext[0] = 0;
            item_path_extract_filename(path, item->name, &(item->ext), &(item->is_link));
            item->type = item_get_item_type_from_extension(item->ext);
        }

        // FURI_LOG_I(TAG, "Basename: %s", furi_string_get_cstr(item->name));
        item_prettify_name(item->name);

        item->path = furi_string_alloc_set(path);
        // FURI_LOG_I(TAG, "Path: %s", furi_string_get_cstr(item->path));
    }

    furi_string_free(in_path);
    FileArray_clear(flist);
    dir_walk_free(dir_walk);

    return iview;
}

void item_items_view_free(ItemsView* items_view) {
    furi_string_free(items_view->name);
    furi_string_free(items_view->path);
    ItemArray_it_t iter;
    for(ItemArray_it(iter, items_view->items); !ItemArray_end_p(iter); ItemArray_next(iter)) {
        furi_string_free(ItemArray_ref(iter)->name);
        furi_string_free(ItemArray_ref(iter)->path);
    }
    ItemArray_clear(items_view->items);
    free(items_view);
}

void item_prettify_name(FuriString* name) {
    // FURI_LOG_I(TAG, "Converting %s to...", furi_string_get_cstr(name));
    if(furi_string_size(name) > 3) {
        char c = furi_string_get_char(name, 2);
        if(c == '_') {
            char a = furi_string_get_char(name, 0);
            char b = furi_string_get_char(name, 1);
            if(a >= '0' && a <= '9' && b >= '0' && b <= '9') {
                furi_string_right(name, 3);
            }
        }
    }
    furi_string_replace_all_str(name, "_", " ");
    // FURI_LOG_I(TAG, "... %s", furi_string_get_cstr(name));
}

ItemType item_get_item_type_from_extension(const char* ext) {
    ItemType type = Item_Unknown;

    if(!strcmp(ext, ".sub")) {
        type = Item_SubGhz;
    } else if(!strcmp(ext, ".rfid")) {
        type = Item_RFID;
    } else if(!strcmp(ext, ".ir")) {
        type = Item_IR;
    } else if(!strcmp(ext, ".nfc")) {
        type = Item_NFC;
    } else if(!strcmp(ext, ".ibtn")) {
        type = Item_iButton;
    } else if(!strcmp(ext, ".qpl")) {
        type = Item_Playlist;
    }
    return type;
}

void item_path_extract_filename(
    FuriString* path,
    FuriString* name,
    char (*ext)[MAX_EXT_LEN],
    bool* is_link) {
    furi_check(ext);
    furi_check(is_link);

    FuriString* temp = furi_string_alloc_set(path);
    *is_link = furi_string_end_withi_str(temp, ".ql");
    if(*is_link) {
        furi_string_left(temp, furi_string_size(temp) - 3);
    }

    path_extract_extension(temp, *ext, MAX_EXT_LEN);
    path_extract_filename(temp, name, true);

    furi_string_free(temp);
}
