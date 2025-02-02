#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// Функция генерации случайного числа в диапазоне [min, max]
double randfrom(double min, double max) {
    return min + (max - min) * ((double)rand() / RAND_MAX);
}

// Основная функция
int main(int argc, char** argv) {
    int rank, size;
    int total_points, local_points, hit_count = 0, global_hit_count = 0;

    // Инициализация MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Главный процесс получает количество точек от пользователя
    if (rank == 0) {
        if (argc != 2) {
            printf("Utilisation: %s <количество точек>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        total_points = atoll(argv[1]);  // Общее количество точек
    }

    // Рассылка общего числа точек всем процессам
    MPI_Bcast(&total_points, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    // Разбиение работы между процессами
    local_points = total_points / size;

    // Инициализация генератора случайных чисел
    srand(time(NULL) + rank);

    // Генерация случайных точек и подсчёт попаданий
    for (int i = 0; i < local_points; i++) {
        double x = randfrom(0.0, 1.0);
        double y = randfrom(0.0, 1.0);
        if (y <= x * x) {
            hit_count++;
        }
    }

    // Сбор данных в корневом процессе
    MPI_Reduce(&hit_count, &global_hit_count, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // Главный процесс вычисляет интеграл
    if (rank == 0) {
        double integral = (double)global_hit_count / total_points;
        printf("Approximate value of the integral: %.6f\n", integral);
    }

    // Завершение MPI
    MPI_Finalize();
    return 0;
}
