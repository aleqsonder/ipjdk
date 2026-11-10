import ipj.memory.SharedPointer;

import java.nio.charset.StandardCharsets;
import java.util.Scanner;

public final class SharedPointerIpcDemo {
    private static final long REGION_SIZE = 4096;

    // Разметка области:
    // [0..3]    — длина строки в байтах;
    // [4..4095] — строка в UTF-8.
    private static final int HEADER_SIZE = Integer.BYTES;
    private static final int MAX_MESSAGE =
            (int) REGION_SIZE - HEADER_SIZE;

    public static void main(String[] args) {
        if (args.length != 3 ||
                !(args[0].equals("server") || args[0].equals("client"))) {
            System.err.println(
                    "Использование: java SharedPointerIpcDemo "
                    + "<server|client> <region-id> <message>");
            return;
        }

        try (SharedPointer memory =
                     new SharedPointer(args[1], REGION_SIZE)) {
            System.out.printf(
                    "PID=%d, область=%s, создана=%s%n",
                    ProcessHandle.current().pid(),
                    args[1],
                    memory.isNew());

            if (args[0].equals("server")) {
                runServer(memory, args[2]);
            } else {
                runClient(memory);
            }
        }
    }

    private static void runServer(SharedPointer memory, String message) {
        writeMessage(memory, message);

        System.out.println("Сообщение записано.");
        System.out.println(
                "Запустите client с тем же region-id в другом терминале.");
        System.out.println(
                "После завершения client нажмите Enter здесь.");

        try (final var scanner = new Scanner(System.in)) {
			scanner.next();
		}

        System.out.println("Ответ: " + readMessage(memory));
    }

    private static void runClient(SharedPointer memory) {
        if (memory.isNew()) {
            throw new IllegalStateException(
                    "Область не существовала. Сначала запустите server "
                    + "и используйте одинаковый region-id.");
        }

        System.out.println("Получено: " + readMessage(memory));

        writeMessage(memory, "Ответ от второго процесса");
        System.out.println(
                "Ответ записан. Теперь нажмите Enter в терминале server.");
    }

    private static void writeMessage(SharedPointer memory, String text) {
        byte[] bytes = text.getBytes(StandardCharsets.UTF_8);
        if (bytes.length > MAX_MESSAGE) {
            throw new IllegalArgumentException("Сообщение слишком длинное");
        }

        SharedPointer cursor = memory.skip(HEADER_SIZE);
        for (byte value : bytes) {
            cursor = cursor.put8(value);
        }

        memory.set32(bytes.length);
    }

    private static String readMessage(SharedPointer memory) {
        int length = memory.get32();
        if (length < 0 || length > MAX_MESSAGE) {
            throw new IllegalStateException(
                    "Некорректная длина сообщения: " + length);
        }

        byte[] bytes = new byte[length];
        SharedPointer cursor = memory.skip(HEADER_SIZE);

        for (int i = 0; i < length; i++) {
            bytes[i] = cursor.get8();
            cursor = cursor.skip(Byte.BYTES);
        }

        return new String(bytes, StandardCharsets.UTF_8);
    }
}
