import matplotlib.pyplot as plt

# Данные
processes = [1, 2, 4, 8]
execution_time = [15.34, 12.34, 6.78, 3.45]
collisions = [1, 1, 1, 1]  # Найденные коллизии

# Построение графика
plt.figure(figsize=(8, 6))

# График времени выполнения
plt.plot(processes, execution_time, marker='o', label="Время выполнения (сек.)")

# Подписи осей
plt.xlabel("Количество процессов")
plt.ylabel("Время выполнения (сек.)")
plt.title("Зависимость времени выполнения от количества процессов")

# Добавление аннотаций для найденных коллизий
for i, (x, y) in enumerate(zip(processes, execution_time)):
    plt.text(x, y, f"Коллизии: {collisions[i]}", fontsize=10, ha='right')

# Легенда
plt.legend()

# Отображение сетки
plt.grid(True)

# Сохранение графика в файл
plt.savefig("execution_time_vs_processes.png")  # Здесь укажите нужное имя файла и формат

# Показ графика
plt.show()
