#!/usr/bin/env ruby

require 'pathname'

# Константы для путей
SOURCE_DIR = 'src'                   # Исходная директория
OUTPUT_FILE = 'combined_output.txt'  # Имя выходного файла

# Проверка существования исходной директории
unless Dir.exist?(SOURCE_DIR)
  puts "Директория '#{SOURCE_DIR}' не найдена."
  exit 1
end

# Открытие выходного файла для записи (перезаписывает существующий файл)
begin
  File.open(OUTPUT_FILE, 'w') do |outfile|
    # Рекурсивно обходим все файлы в SOURCE_DIR
    Dir.glob(File.join(SOURCE_DIR, '**', '*')).each do |filepath|
      next if File.directory?(filepath)  # Пропускаем директории

      # Получаем относительный путь файла
      relative_path = Pathname.new(filepath).relative_path_from(Pathname.new(SOURCE_DIR)).to_s

      # Читаем содержимое файла
      begin
        content = File.read(filepath)
      rescue => e
        puts "Не удалось прочитать файл #{filepath}: #{e.message}"
        next
      end

      # Записываем относительный путь и содержимое в выходной файл
      outfile.puts "---- #{relative_path} ----"
      outfile.puts content
      outfile.puts "\n"  # Добавляем разделитель между файлами
    end
  end
rescue => e
  puts "Не удалось открыть файл для записи: #{e.message}"
  exit 1
end

puts "Объединение файлов завершено. Результат сохранён в #{OUTPUT_FILE}"