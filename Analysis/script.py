import csv

def process_binary_file(file_path, output_csv_path):
    try:
        with open(file_path, 'rb') as file:
            data = file.read()
            pattern = bytes([0x12, 0x34, 0x56])
            extracted_data = []

            for i in range(0, len(data), 0x1000):
                if data[i:i+3] == pattern and 0x70 <= data[i+3] <= 0x7F:
                    offset = i + 4
                    while offset < i + 0x1000:
                        if offset + 3 >= len(data):
                            break
                        lsb = data[offset]
                        msb = data[offset + 1]
                        raw_temperature = (msb << 8) | lsb
                        temperature = raw_temperature / 16.0
                        sequence_number = (data[offset + 2] << 8) | data[offset + 3]
                        if sequence_number != 0:
                            time_seconds = sequence_number * 30
                            hours = time_seconds // 3600
                            minutes = (time_seconds % 3600) // 60
                            seconds = time_seconds % 60
                            time_formatted = f"{hours}:{minutes:02}:{seconds:02}"
                        else:
                            time_formatted = "0:00:00"
                        extracted_data.append((sequence_number, temperature, time_formatted))
                        offset += 4

            # Vytvorenie CSV súboru
            with open(output_csv_path, 'w', newline='') as csvfile:
                csvwriter = csv.writer(csvfile)
                # Napíšeme hlavičku
                csvwriter.writerow(['Sequence Number', 'Time (seconds)', 'Temperature (°C)'])
                # Napíšeme dáta
                for sequence_number, temperature, time_seconds in extracted_data:
                    csvwriter.writerow([sequence_number, time_seconds, temperature])
                    
            print(f"CSV file created: {output_csv_path}")
    except FileNotFoundError:
        print(f"File {file_path} not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    process_binary_file("empty.bin", "output.csv")
