#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Использование: $0 <исходный_файл> <конечный_файл>"
    echo "Пример: $0 test.bin result.bin"
    exit 1
fi

SOURCE=$1
DEST=$2

if [ ! -f "$SOURCE" ]; then
    echo "Ошибка: файл $SOURCE не найден"
    exit 1
fi

FILE_SIZE_BYTES=$(stat -c%s "$SOURCE")
FILE_SIZE_MB=$(echo "scale=2; $FILE_SIZE_BYTES/1024/1024" | bc 2>/dev/null || echo "unknown")

echo ""
echo "Тестирование файла: $SOURCE (размер: $FILE_SIZE_MB МБ)"
echo ""

echo "Тест с размером блоков"
echo "Фиксированное количество операций - 4"
BLOCK_SIZES=(4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304)

for BLOCK_SIZE in "${BLOCK_SIZES[@]}"; do
    echo -n "Размер блока: $BLOCK_SIZE байт (фикс. операции: 4) - "
    OUTPUT=$(./lab1_aio "$SOURCE" "${DEST}_block${BLOCK_SIZE}.bin" $BLOCK_SIZE 4)
    TIME=$(echo "$OUTPUT" | grep -o '[0-9]\+\.[0-9]\+' | head -1)
    
    if [ ! -z "$TIME" ]; then
        SPEED=$(echo "scale=2; $FILE_SIZE_MB / $TIME" | bc 2>/dev/null || echo "?")
        echo "$TIME сек, $SPEED МБ/с"
    else
        echo "$OUTPUT"
    fi
done

echo ""
echo ""

echo "Тест с количеством операций"
echo "Фиксированный размер блока: 1048576 байт"
OP_COUNTS=(1 2 4 8 12 16)

for OP_COUNT in "${OP_COUNTS[@]}"; do
    echo -n "Количество операций: $OP_COUNT (фикс. блок: 1048576 байт) - "
    OUTPUT=$(./lab1_aio "$SOURCE" "${DEST}_ops${OP_COUNT}.bin" 1048576 $OP_COUNT)
    TIME=$(echo "$OUTPUT" | grep -o '[0-9]\+\.[0-9]\+' | head -1)
    
    if [ ! -z "$TIME" ]; then
        SPEED=$(echo "scale=2; $FILE_SIZE_MB / $TIME" | bc 2>/dev/null || echo "?")
        echo "$TIME сек, $SPEED МБ/с"
    else
        echo "$OUTPUT"
    fi
done

echo ""
