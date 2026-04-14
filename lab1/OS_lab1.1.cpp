#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <limits>
#include <iomanip>

void listOfDrives();
void driveInfo();
void createDir();
void removeDir();
void createFile();
void copyMoveFile();
void showFileAttributes();
void showFileInfo();
void changeFileAttributes();
void changeFileTime();
void moveFileExFunc();
void printAttributes(DWORD attrs);
int user_input(int max_choice);
bool parseDate(const char* str, SYSTEMTIME& st);
bool parseTime(const char* str, SYSTEMTIME& st);

using namespace std;

int menu() {
    system("cls");
    int choice = -1;
    while (choice < 0 || choice > 11) {
        cout << "Что вы хотите сделать?\n";
        cout << "1. Получить список дисков.\n";
        cout << "2. Получить информацию о диске.\n";
        cout << "3. Создать каталог.\n";
        cout << "4. Удалить каталог.\n";
        cout << "5. Создать новый файл.\n";
        cout << "6. Скопировать и переместить файл.\n";
        cout << "7. MoveFileEx\n";
        cout << "8. Показать информацию о файле.\n";
        cout << "9. Показать атрибуты файла.\n";
        cout << "10. Изменить атрибуты файла.\n";
        cout << "11. Изменить время файла.\n";
        cout << "0. Выход.\n";
        cout << "Ваш выбор: ";
        choice = user_input(12);
        cout << endl;
    }
    return choice;
}

int user_input(int max_choice) {
    int choice;
    char str[50];
    cin >> str;
    while (sscanf_s(str, "%d", &choice) != 1 || choice < 0 || choice > max_choice) {
        cout << "Неверный ввод. Попробуйте снова: ";
        cin >> str;
    }
    return choice;
}

void listOfDrives() {
    DWORD mask = GetLogicalDrives();
    cout << "Логические диски (битовая маска десятичная): " << mask << "\n";

    cout << "Логические диски (битовая маска двоичная): ";

    int startBit = 0;
    for (int i = 31; i >= 0; i--) {
        if (mask & (1 << i)) {
            startBit = i;
            break;
        }
    }

    for (int i = startBit; i >= 0; i--) {
        cout << ((mask & (1 << i)) ? "1" : "0");
    }
    cout << "\n";

    char buffer[256];
    DWORD len = GetLogicalDriveStringsA(sizeof(buffer), buffer);
    if (len > 0) {
        cout << "Диски:\n";
        for (char* drive = buffer; *drive; drive += strlen(drive) + 1) {
            cout << "  " << drive << "\n";
        }
    }
    system("pause");
}

void driveInfo() {
    char str[50];
    int choice;
    cout << "Введите диск (например, C:\\): \n";
    cin >> str;

    char diskPath[4];
    strcpy_s(diskPath, str);

    choice = GetDriveTypeA(str);
    switch (choice) {
    case DRIVE_REMOVABLE:
        cout << "Съемный диск\n";
        break;
    case DRIVE_FIXED:
        cout << "Фиксированный диск (жесткий диск)\n";
        break;
    case DRIVE_REMOTE:
        cout << "Сетевой диск\n";
        break;
    case DRIVE_CDROM:
        cout << "Компакт-диск (CD/DVD)\n";
        break;
    case DRIVE_RAMDISK:
        cout << "RAM диск\n";
        break;
    default:
        cout << "Невозможно определить тип диска\n";
        break;
    }

    char volumeName[256] = { 0 };
    char fileSystemName[256] = { 0 };
    DWORD serialNumber = 0;
    DWORD maxComponentLength = 0;
    DWORD fileSystemFlags = 0;

    if (GetVolumeInformationA(
        diskPath,
        volumeName,
        sizeof(volumeName),
        &serialNumber,
        &maxComponentLength,
        &fileSystemFlags,
        fileSystemName,
        sizeof(fileSystemName)
    )) {
        cout << "Метка тома: " << volumeName << "\n";
        cout << "Серийный номер: " << hex << showbase << serialNumber << dec << "\n";
        cout << "Файловая система: " << fileSystemName << "\n";
        cout << "Максимальная длина имени файла: " << maxComponentLength << " символов\n";

        cout << "Флаги файловой системы: " << hex << showbase << fileSystemFlags << dec << "\n";

        cout << "\nВозможности файловой системы:\n";

        cout << "Регистрозависимый поиск: " << ((fileSystemFlags & FILE_CASE_SENSITIVE_SEARCH) ? "Да" : "Нет") << "\n";
        cout << "Сохранение регистра имен: " << ((fileSystemFlags & FILE_CASE_PRESERVED_NAMES) ? "Да" : "Нет") << "\n";
        cout << "Поддержка Unicode: " << ((fileSystemFlags & FILE_UNICODE_ON_DISK) ? "Да" : "Нет") << "\n";
        cout << "Поддержка ACL: " << ((fileSystemFlags & FILE_PERSISTENT_ACLS) ? "Да" : "Нет") << "\n";
        cout << "Сжатие файлов: " << ((fileSystemFlags & FILE_FILE_COMPRESSION) ? "Да" : "Нет") << "\n";
        cout << "Квоты тома: " << ((fileSystemFlags & FILE_VOLUME_QUOTAS) ? "Да" : "Нет") << "\n";
        cout << "Разреженные файлы: " << ((fileSystemFlags & FILE_SUPPORTS_SPARSE_FILES) ? "Да" : "Нет") << "\n";
        cout << "Точки повторной обработки: " << ((fileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS) ? "Да" : "Нет") << "\n";
        cout << "Шифрование файлов: " << ((fileSystemFlags & FILE_SUPPORTS_ENCRYPTION) ? "Да" : "Нет") << "\n";
        cout << "Именованные потоки: " << ((fileSystemFlags & FILE_NAMED_STREAMS) ? "Да" : "Нет") << "\n";
        cout << "Том только для чтения: " << ((fileSystemFlags & FILE_READ_ONLY_VOLUME) ? "Да" : "Нет") << "\n";
        cout << "Транзакции: " << ((fileSystemFlags & FILE_SUPPORTS_TRANSACTIONS) ? "Да" : "Нет") << "\n";
        cout << "Жесткие ссылки: " << ((fileSystemFlags & FILE_SUPPORTS_HARD_LINKS) ? "Да" : "Нет") << "\n";
        cout << "Расширенные атрибуты: " << ((fileSystemFlags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES) ? "Да" : "Нет") << "\n";
        cout << "Журнал USN: " << ((fileSystemFlags & FILE_SUPPORTS_USN_JOURNAL) ? "Да" : "Нет") << "\n";
    }
    else {
        cout << "Не удалось получить информацию о томе. Ошибка: " << GetLastError() << "\n";
    }

    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalFreeBytes;

    if (GetDiskFreeSpaceExA(
        diskPath,
        &freeBytesAvailable,
        &totalNumberOfBytes,
        &totalFreeBytes
    )) {
        double totalGB = (double)totalNumberOfBytes.QuadPart / (1024 * 1024 * 1024);
        double freeGB = (double)totalFreeBytes.QuadPart / (1024 * 1024 * 1024);
        double usedGB = totalGB - freeGB;
        cout << "Всего: " << totalGB << " ГБ\n";
        cout << "Свободно: " << freeGB << " ГБ\n";
        cout << "Занято: " << usedGB << " ГБ\n";
    }

    system("pause");
}

void createDir() {
    char str[50];

    cout << "Введите имя нового каталога: \n";
    cin >> str;

    if (CreateDirectoryA(str, NULL)) {
        cout << "Каталог " << str << " создан.\n";
    }
    else {
        DWORD error = GetLastError();
        if (error == ERROR_ALREADY_EXISTS) {
            cout << "Каталог " << str << " уже существует.\n";
        }
        else {
            cout << "Не удалось создать каталог. Ошибка: " << error << "\n";
        }
    }
    system("pause");
}

void removeDir() {
    char str[50];
    int choice;

    cout << "Введите имя каталога для удаления: \n";
    cin >> str;

    do {
        cout << "Это действие нельзя отменить. Вы уверены, что хотите удалить каталог?\n";
        cout << "1. Да.\n";
        cout << "2. Нет.\n";
        cout << "Ваш выбор: ";
        cin >> choice;

        if (choice == 1) {
            if (RemoveDirectoryA(str)) {
                cout << "Каталог " << str << " удален.\n";
            }
            else {
                DWORD error = GetLastError();
                if (error == ERROR_FILE_NOT_FOUND) {
                    cout << "Каталог не найден.\n";
                }
                else {
                    cout << "Не удалось удалить каталог. Ошибка: " << error << "\n";
                }
            }
            break;
        }
        else if (choice == 2) {
            cout << "Операция отменена.\n";
            break;
        }
        else {
            cout << "Неверный ввод!\n";
        }
    } while (true);

    system("pause");
}

void createFile() {
    char file_path[256];
    char file_name[256];
    char full_path[512];

    cout << "Введите путь для создания файла (например, C:\\folder\\): ";
    cin >> file_path;

    cout << "Введите имя нового файла (например, test.txt): ";
    cin >> file_name;

    strcpy_s(full_path, file_path);
    strcat_s(full_path, file_name);

    HANDLE handle = CreateFileA(
        full_path,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();

        if (error == ERROR_FILE_NOT_FOUND) {
            cout << "Путь не найден. Файл не может быть создан.\n";
        }
        else if (error == ERROR_ACCESS_DENIED) {
            cout << "Доступ запрещен. Невозможно создать файл.\n";
        }
        else if (error == ERROR_FILE_EXISTS) {
            cout << "Не удалось создать файл. Файл уже существует.\n";
        }
        else {
            cout << "Не удалось создать файл. Код ошибки: " << error << "\n";
        }
    }
    else {
        cout << "Файл создан по пути: " << full_path << "\n";
        CloseHandle(handle);
    }
    system("pause");
}

void copyMoveFile() {
    char source[MAX_PATH];
    char dest[MAX_PATH];
    int choice;
    do {
        cout << "Выберите операцию:\n";
        cout << "1. Скопировать файл.\n";
        cout << "2. Переместить файл.\n";
        cout << "Введите ваш выбор: ";
        cin >> choice;

        if (choice == 1) {
            cout << "Введите путь к исходному файлу (например, C:\\folder\\file.txt): ";
            cin >> source;

            cout << "Введите путь назначения (например, C:\\newfolder\\file.txt): ";
            cin >> dest;

            DWORD sourceAtt = GetFileAttributesA(source);
            if (sourceAtt == INVALID_FILE_ATTRIBUTES) {
                cout << "Исходный файл не найден.\n";
                system("pause");
                return;
            }

            DWORD destAtt = GetFileAttributesA(dest);
            if (destAtt != INVALID_FILE_ATTRIBUTES) {
                cout << "Файл уже существует в месте назначения. Перезапись не разрешена.\n";
                system("pause");
                return;
            }

            BOOL result = CopyFileA(source, dest, FALSE);
            if (result) {
                cout << "Файл успешно скопирован.\n";
            }
            else {
                DWORD error = GetLastError();
                cout << "Копирование не удалось. Код ошибки: " << error << "\n";
            }

            break;
        }
        else if (choice == 2) {
            cout << "Введите путь к исходному файлу (например, C:\\folder\\file.txt): ";
            cin >> source;

            cout << "Введите путь назначения (например, C:\\newfolder\\file.txt): ";
            cin >> dest;

            DWORD sourceAtt = GetFileAttributesA(source);
            if (sourceAtt == INVALID_FILE_ATTRIBUTES) {
                cout << "Исходный файл не найден.\n";
                system("pause");
                return;
            }

            DWORD destAtt = GetFileAttributesA(dest);
            if (destAtt != INVALID_FILE_ATTRIBUTES) {
                cout << "Файл уже существует в месте назначения. Перезапись не разрешена.\n";
                system("pause");
                return;
            }

            BOOL result = MoveFileA(source, dest);
            if (result) {
                cout << "Файл успешно перемещен!\n";
            }
            else {
                DWORD error = GetLastError();
                cout << "Перемещение не удалось. Код ошибки: " << error << "\n";
            }

            break;
        }
        else {
            cout << "Неверный ввод.\n";
        }
    } while (true);

    system("pause");
}

void moveFileExFunc() {
    char source[MAX_PATH];
    char dest[MAX_PATH];
    int choice;

    cout << "Введите путь к исходному файлу (например, C:\\folder\\file.txt): ";
    cin >> source;

    cout << "Введите путь назначения (например, C:\\newfolder\\file.txt): ";
    cin >> dest;

    DWORD sourceAtt = GetFileAttributesA(source);
    if (sourceAtt == INVALID_FILE_ATTRIBUTES) {
        cout << "Исходный файл не найден.\n";
        system("pause");
        return;
    }

    DWORD destAtt = GetFileAttributesA(dest);
    bool destExists = (destAtt != INVALID_FILE_ATTRIBUTES &&
        !(destAtt & FILE_ATTRIBUTE_DIRECTORY));

    cout << "Файл с таким именем уже существует в месте назначения.\n";

    cout << "Что вы хотите сделать?\n";
    cout << "1. Переместить без флагов.\n";
    cout << "2. Перезаписать.\n";
    cout << "3. Переместить между томами (копировать + удалить исходный)\n";
    cout << "4. Переместить между томами + перезаписать\n";
    cout << "Ваш выбор: ";
    cin >> choice;

    DWORD flags = 0;

    switch (choice) {
    case 1:
        flags = 0;
        break;
    case 2:
        flags = MOVEFILE_REPLACE_EXISTING;
        break;
    case 3:
        flags = MOVEFILE_COPY_ALLOWED;
        break;
    case 4:
        flags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;
        break;
    }

    BOOL result = MoveFileExA(source, dest, flags);

    if (result)
        cout << "Операция MoveFileEx успешно завершена.\n";
    else
        cout << "MoveFileEx не удалась. Код ошибки: " << GetLastError() << "\n";

    system("pause");
}

void showFileAttributes() {
    char file_name[256];
    DWORD attrs;

    do {
        cout << "Введите путь к файлу (например, C:\\some_dir\\file_name): ";
        cin >> file_name;

        attrs = GetFileAttributesA(file_name);
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            cout << "Файл не найден. Попробуйте снова.\n";
        }
        else {
            break;
        }
    } while (true);

    printAttributes(attrs);
    system("pause");
}

void printAttributes(DWORD attrs) {

    cout << "Код атрибута: 0x" << hex << attrs << dec << "\n";

    if (attrs & FILE_ATTRIBUTE_ARCHIVE) cout << "- Архивный\n";
    if (attrs & FILE_ATTRIBUTE_HIDDEN) cout << "- Скрытый\n";
    if (attrs & FILE_ATTRIBUTE_READONLY) cout << "- Только чтение\n";
    if (attrs & FILE_ATTRIBUTE_SYSTEM) cout << "- Системный\n";
    if (attrs & FILE_ATTRIBUTE_TEMPORARY) cout << "- Временный\n";
}

void showFileInfo() {
    char file_name[256];

    do {
        cout << "Введите путь к файлу: ";
        cin >> file_name;
    } while (GetFileAttributesA(file_name) == INVALID_FILE_ATTRIBUTES && cout << "Файл не найден. Попробуйте снова.\n");

    HANDLE hFile = CreateFileA(
        file_name,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        cout << "Не удается открыть файл\n";
        system("pause");
        return;
    }

    BY_HANDLE_FILE_INFORMATION info;
    if (GetFileInformationByHandle(hFile, &info)) {

        cout << "\nРазмер файла: " << info.nFileSizeLow << " байт\n";
        cout << "Количество ссылок: " << info.nNumberOfLinks << "\n";

        FILETIME create, access, write;
        GetFileTime(hFile, &create, &access, &write);

        SYSTEMTIME stUTC, stLocal;
        cout << "\nВременные метки файла:\n";
        FileTimeToSystemTime(&create, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

        printf("Создан: %02d.%02d.%04d %02d:%02d:%02d\n",
            stLocal.wDay, stLocal.wMonth, stLocal.wYear,
            stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

        FileTimeToSystemTime(&access, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

        printf("Последний доступ: %02d.%02d.%04d %02d:%02d:%02d\n",
            stLocal.wDay, stLocal.wMonth, stLocal.wYear,
            stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

        FileTimeToSystemTime(&write, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

        printf("Изменен: %02d.%02d.%04d %02d:%02d:%02d\n",
            stLocal.wDay, stLocal.wMonth, stLocal.wYear,
            stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
    }

    CloseHandle(hFile);

    system("pause");
}

void changeFileAttributes() {
    char file_name[256];
    int choice;

    do {
        cout << "Введите путь к файлу: ";
        cin >> file_name;
    } while (GetFileAttributesA(file_name) == INVALID_FILE_ATTRIBUTES &&
        cout << "Файл не найден. Попробуйте снова.\n");

    DWORD attrs = GetFileAttributesA(file_name);
    DWORD newAttrs = attrs;

    cout << "Выберите атрибут:\n";
    cout << "1. Только чтение\n";
    cout << "2. Скрытый\n";
    cout << "3. Архивный\n";
    cout << "4. Системный\n";
    cout << "Выбор: ";

    choice = user_input(4);

    switch (choice) {
    case 1: newAttrs ^= FILE_ATTRIBUTE_READONLY; break;
    case 2: newAttrs ^= FILE_ATTRIBUTE_HIDDEN; break;
    case 3: newAttrs ^= FILE_ATTRIBUTE_ARCHIVE; break;
    case 4: newAttrs ^= FILE_ATTRIBUTE_SYSTEM; break;
    }

    if (SetFileAttributesA(file_name, newAttrs))
        cout << "Атрибут изменен\n";
    else
        cout << "Ошибка: " << GetLastError() << "\n";

    system("pause");
}

bool parseDate(const char* str, SYSTEMTIME& st) {
    int d, m, y;

    if (sscanf_s(str, "%d.%d.%d", &d, &m, &y) != 3)
        return false;

    if (d < 1 || d > 31) return false;
    if (m < 1 || m > 12) return false;

    st.wDay = d;
    st.wMonth = m;
    st.wYear = y;

    return true;
}

bool parseTime(const char* str, SYSTEMTIME& st) {
    int h, min, s;

    if (sscanf_s(str, "%d:%d:%d", &h, &min, &s) != 3)
        return false;

    if (h < 0 || h > 23) return false;
    if (min < 0 || min > 59) return false;
    if (s < 0 || s > 59) return false;

    st.wHour = h;
    st.wMinute = min;
    st.wSecond = s;

    return true;
}

void changeFileTime() {
    char file_name[256];
    int choice;
    int dateDay, dateMonth, dateYear;
    int timeHour, timeMin, timeSec;

    do {
        cout << "Введите путь к файлу: ";
        cin >> file_name;
        if (GetFileAttributesA(file_name) == INVALID_FILE_ATTRIBUTES)
            cout << "Файл не найден. Попробуйте снова.\n";
        else
            break;
    } while (true);

    HANDLE hFile = CreateFileA(
        file_name,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        cout << "Не удается открыть файл\n";
        system("pause");
        return;
    }

    FILETIME createTime, accessTime, writeTime;
    GetFileTime(hFile, &createTime, &accessTime, &writeTime);

    SYSTEMTIME st;
    FileTimeToSystemTime(&writeTime, &st);

    cout << "\nВыберите время для изменения:\n";
    cout << "1. Время создания\n";
    cout << "2. Время последнего доступа\n";
    cout << "3. Время последней записи\n";
    cout << "Выбор: ";
    cin >> choice;

    FILETIME* targetTime;

    if (choice == 1)
        targetTime = &createTime;
    else if (choice == 2)
        targetTime = &accessTime;
    else
        targetTime = &writeTime;

    SYSTEMTIME stUTC, stLocal;

    FileTimeToSystemTime(targetTime, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    cout << "Введите дату (ДД.ММ.ГГГГ) или 0 чтобы пропустить: ";
    if (scanf_s("%d.%d.%d", &dateDay, &dateMonth, &dateYear) == 3 && dateDay != 0) {
        stLocal.wDay = dateDay;
        stLocal.wMonth = dateMonth;
        stLocal.wYear = dateYear;
    }
    else {
        cout << "Изменение даты пропущено.\n";
        char ch; while ((ch = getchar()) != '\n' && ch != EOF);
    }

    cout << "Введите время (ЧЧ:ММ:СС) или 0 чтобы пропустить: ";
    if (scanf_s("%d:%d:%d", &timeHour, &timeMin, &timeSec) == 3 && timeHour != 0) {
        stLocal.wHour = timeHour;
        stLocal.wMinute = timeMin;
        stLocal.wSecond = timeSec;
    }
    else {
        cout << "Изменение времени пропущено.\n";
        char ch; while ((ch = getchar()) != '\n' && ch != EOF);
    }

    if (!TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC)) {
        cout << "Ошибка преобразования локального времени в UTC. Код: " << GetLastError() << "\n";
        CloseHandle(hFile);
        system("pause");
        return;
    }

    FILETIME ft;

    if (!SystemTimeToFileTime(&stUTC, &ft)) {
        cout << "Ошибка преобразования SYSTEMTIME в FILETIME. Код: " << GetLastError() << "\n";
        CloseHandle(hFile);
        system("pause");
        return;
    }

    if (choice == 1)
        createTime = ft;
    else if (choice == 2)
        accessTime = ft;
    else
        writeTime = ft;

    if (SetFileTime(hFile, &createTime, &accessTime, &writeTime))
        cout << "Время успешно обновлено\n";
    else
        cout << "Ошибка: " << GetLastError() << "\n";

    CloseHandle(hFile);

    system("pause");
}

int main()
{
    int choice;
    do {
        choice = menu();
        switch (choice) {
        case 0:
            cout << "Выход из программы." << endl;
            break;
        case 1:
            listOfDrives();
            break;
        case 2:
            driveInfo();
            break;
        case 3:
            createDir();
            break;
        case 4:
            removeDir();
            break;
        case 5:
            createFile();
            break;
        case 6:
            copyMoveFile();
            break;
        case 7:
            moveFileExFunc();
            break;
        case 8:
            showFileInfo();
            break;
        case 9:
            showFileAttributes();
            break;
        case 10:
            changeFileAttributes();
            break;
        case 11:
            changeFileTime();
            break;
        }
    } while (choice != 0);
    return 0;
}