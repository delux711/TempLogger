import struct
import csv

# Definovanie konštánt
HEADER_PATTERN_START = b'\x12\x34\x56\x70'
HEADER_PATTERN_END = b'\x12\x34\x56\x7F'
BLOCK_SIZE = 0x1000  # Vzdialenosť medzi vzormi
RECORD_SIZE = 4  # Veľkosť záznamu (2 bajty pre teplotu, 2 bajty pre poradové číslo)

def parse_binary_file(input_file, output_file):
    with open(input_file, 'rb') as file:
        data = file.read()

    # Vytvorenie výstupného CSV súboru
    with open(output_file, mode='w', newline='', encoding='utf-8') as csv_file:
        writer = csv.writer(csv_file, delimiter=';')  # Použitie bodkočiarky ako oddeľovača
        writer.writerow(['Time (s)', 'Sequence Number', 'Temperature (C)'])

        # Iterácia cez všetky bloky
        for block_start in range(0, len(data), BLOCK_SIZE):
            # Hľadanie hlavičky v požadovanom rozsahu
            for header_byte in range(0x70, 0x80):
                header_pattern = b'\x12\x34\x56' + bytes([header_byte])
                if data[block_start:block_start + 4] == header_pattern:
                    block_offset = block_start + 4  # Preskočiť hlavičku

                    for record_start in range(block_offset, block_start + BLOCK_SIZE, RECORD_SIZE):
                        record = data[record_start:record_start + RECORD_SIZE]

                        # Skontrolovať, či má záznam správnu veľkosť
                        if len(record) < RECORD_SIZE:
                            continue

                        # Rozbaliť záznam
                        raw_temperature, sequence_number = struct.unpack('<HH', record)

                        # Konverzia teploty
                        temperature = raw_temperature / 16.0

                        # Výpočet času na základe poradového čísla (predpoklad 30s intervaly)
                        time_seconds = sequence_number * 30

                        # Zápis do CSV
                        writer.writerow([time_seconds, sequence_number, f"{temperature:.2f}"])

# Hlavná funkcia na spustenie skriptu
if __name__ == "__main__":
    input_file = "empty.bin"  # Názov vstupného súboru
    output_file = "output.csv"  # Názov výstupného súboru
    parse_binary_file(input_file, output_file)
    print(f"Dáta boli extrahované a uložené do súboru {output_file}")
