#include <gtest/gtest.h>
#include "patchfile/FileList.h"

TEST(FileListTest, FileList_sortAsc) {
    // Given
    File file1, file2, file3;
    file1.name = "B";
    file2.name = "C";
    file3.name = "A";
    FileList fileList;
    fileList.files.push_back(file1);
    fileList.files.push_back(file2);
    fileList.files.push_back(file3);

    // When
    fileList.sortAsc();

    // Then
    std::string expectedOrder[] = { "A", "B", "C" };
    int index = 0;
    for (auto& file : fileList.files) {
        EXPECT_EQ(file.name, expectedOrder[index++]);
    }
}

TEST(FileListTest, File_encodeFlags) {
    // Given
    File file;
    file.isDirectory = true;
    file.isAdd = true;
    file.isRemove = true;
    file.isModify = true;

    // When
    uint16_t actual = file.encodeFlags();

    // Then
    EXPECT_EQ(actual, 0x0f);
}

TEST(FileListTest, File_decodeFlags) {
    // Given
    File file;

    // When
    file.decodeFlags(0x0f);

    // Then
    EXPECT_EQ(file.isDirectory, true);
    EXPECT_EQ(file.isAdd, true);
    EXPECT_EQ(file.isRemove, true);
    EXPECT_EQ(file.isModify, true);
}
