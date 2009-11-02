
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>, Manuel Odendahl <wesen@ruinwesen.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <string.h>
#include <SDCard.h>

#ifndef NULL
#define NULL 0
#endif

SDCardClass::SDCardClass() {
  partition = NULL;
  fs = NULL;
  root_dir = NULL;
  current_dir = NULL;
}

/* find name in directory dd, fill dir_entry and return 1 on success */
uint8_t find_file_in_dir(struct fat_fs_struct *fs,
			 struct fat_dir_struct *dd,
			 const char *name,
			 struct fat_dir_entry_struct *dir_entry) {
  while (fat_read_dir(dd, dir_entry)) {
    if (strcmp(dir_entry->long_name, name) == 0) {
      fat_reset_dir(dd);
      return 1;
    }
  }

  return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
{
    struct fat_dir_entry_struct file_entry;
    if(!find_file_in_dir(fs, dd, name, &file_entry))
        return 0;

    return fat_open_file(fs, &file_entry);
}

uint8_t SDCardClass::init() {
  if (current_dir != NULL) {
    fat_close_dir(current_dir);
    current_dir = NULL;
  }
  if (root_dir != NULL) {
    fat_close_dir(root_dir);
    root_dir = NULL;
  }

  if (fs != NULL) {
    fat_close(fs);
    fs = NULL;
  }

  if (partition != NULL) {
    partition_close(partition);
    partition = NULL;
  }
  
  if (!sd_raw_init()) {
    return 1;
  }

  partition = partition_open(sd_raw_read, sd_raw_read_interval,
			     sd_raw_write, sd_raw_write_interval,
			     0);

  if (!partition) {
    return 2;
  }

  fs = fat_open(partition);
  if (!fs) {
    return 3;
  }

  fat_get_dir_entry_of_path(fs, "/", &root_dir_entry);
  root_dir = fat_open_dir(fs, &root_dir_entry);
  if (!root_dir) {
    return 4;
  }

  openDirectory((char *)"/");

  return 0;
}

bool SDCardClass::openDirectory(char *path) {
  if (current_dir != NULL) {
    fat_close_dir(current_dir);
    current_dir = NULL;
  }
  fat_get_dir_entry_of_path(fs, path, &current_dir_entry);
  current_dir = fat_open_dir(fs, &current_dir_entry);
  if (current_dir == NULL)
    return createDirectory(path);
  else
    return true;
}

bool SDCardClass::createDirectory(char *dir) {
  if (current_dir == NULL)
    return false;

  if (dir[0] == '/') {
    if (!openDirectory((char *)"/"))
      return false;
    dir++;
  }

  char subDir[64];
  while (1) {
    char *pos = strchr(dir, '/');
    if (pos == NULL) {
      strncpy(subDir, dir, sizeof(subDir) - 1);
    } else {
      int len = pos - dir;
      memcpy(subDir, dir, len);
      subDir[len] = '\0';
      dir = pos + 1;
    }
    
    struct fat_dir_entry_struct new_dir_entry;
    
    int result = fat_create_dir(current_dir, subDir, &new_dir_entry);

    if (result == 0 && strcmp(subDir, new_dir_entry.long_name)) {
      return false;
    } else {
      memcpy(&current_dir_entry, &new_dir_entry, sizeof(current_dir_entry));
      fat_close_dir(current_dir);
      current_dir = fat_open_dir(fs, &new_dir_entry);
      if (current_dir == NULL)
	return false;

      if (pos == NULL) {
	sd_raw_sync();
	return (current_dir != NULL);
      }
    }
  }
}

bool SDCardClass::writeFile(char *path, uint8_t *buf, uint8_t len) {
  SDCardFile file;
  if (!file.create(path)) {
    return false;
  }
  uint8_t l2 = file.write(buf, len);
  if (l2 != len) {
    file.close();
    return false;
  }
  file.close();
  return true;
}

int SDCardClass::readFile(char *path, uint8_t *buf, uint8_t len) {
  SDCardFile file;
  if (!file.open(path)) {
    return -1;
  }
  uint8_t l2 = file.read(buf, len);
  file.close();
  return l2;
}

offset_t SDCardClass::getSize() {
  if (fs)
    return fat_get_fs_size(fs);
  else
    return 0;
}

offset_t SDCardClass::getFree() {
  if (fs)
    return fat_get_fs_free(fs);
  else
    return 0;
}



SDCardFile::SDCardFile() {
  fd = NULL;
}

bool SDCardFile::open(char *path) {
  if (SDCard.current_dir == NULL)
    return false;

  close();

  char *pos = strrchr(path, '/');
  if (pos != NULL) {
    int len = pos - path;
    char subPath[len+1];
    memcpy(subPath, path, len);
    subPath[len] = 0;
    SDCard.createDirectory(subPath);
    path = pos + 1;
  }

  if (!find_file_in_dir(SDCard.fs, SDCard.current_dir, path, &file_dir_entry)) {
    return false;
  }
  
  fd = fat_open_file(SDCard.fs, &file_dir_entry);
  if (fd != NULL)
    return true;
  else
    return false;
}

bool SDCardFile::create(char *path) {
  if (SDCard.current_dir == NULL)
    return false;

  close();

  char *pos = strrchr(path, '/');
  if (pos != NULL) {
    int len = pos - path;
    char subPath[len+1];
    memcpy(subPath, path, len);
    subPath[len] = 0;
    SDCard.createDirectory(subPath);
    path = pos + 1;
  }
  
  if (!fat_create_file(SDCard.current_dir, path, &file_dir_entry) && strcmp(path, file_dir_entry.long_name))
    return false;

  fd = fat_open_file(SDCard.fs, &file_dir_entry);
  if (!fd) {
    return false;
  } else {
    return true;
  }
}

void SDCardFile::close() {
  if (fd != NULL) {
    fat_close_file(fd);
    sd_raw_sync();
    fd = NULL;
  }
}

intptr_t SDCardFile::read(uint8_t *buf, uint8_t len) {
  if (fd == NULL)
    return -1;

  return fat_read_file(fd, buf, len);
}

intptr_t SDCardFile::write(uint8_t *buf, uint8_t len) {
  if (fd == NULL)
    return -1;

  return fat_write_file(fd, buf, len);
}

bool SDCardFile::seek(int32_t *offset, uint8_t whence) {
  if (fd == NULL)
    return -1;

  if (fat_seek_file(fd, offset, whence))
    return true;
  else
    return false;
}


SDCardClass SDCard;