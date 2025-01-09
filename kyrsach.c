#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <mpi.h>
#include <time.h>

#define KEY_COUNT 1000000
#define MAX_KEY_LENGTH 32
#define MD5_DIGEST_LENGTH 16
#define MAX_RETRIES 250

void compute_md5(const char *input, unsigned char *hash) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create EVP_MD_CTX\n");
        exit(1);
    }

    if (EVP_DigestInit_ex(ctx, EVP_md5(), NULL) != 1) {
        fprintf(stderr, "EVP_DigestInit_ex failed\n");
        EVP_MD_CTX_free(ctx);
        exit(1);
    }

    if (EVP_DigestUpdate(ctx, input, strlen(input)) != 1) {
        fprintf(stderr, "EVP_DigestUpdate failed\n");
        EVP_MD_CTX_free(ctx);
        exit(1);
    }

    unsigned int length = 0;
    if (EVP_DigestFinal_ex(ctx, hash, &length) != 1) {
        fprintf(stderr, "EVP_DigestFinal_ex failed\n");
        EVP_MD_CTX_free(ctx);
        exit(1);
    }

    EVP_MD_CTX_free(ctx);
}

void generate_random_key(char *key, int length) {
    for (int i = 0; i < length; i++) {
        key[i] = 'a' + rand() % 32;  
    }
    key[length] = '\0';
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time = MPI_Wtime();

    srand(time(NULL) + rank);

    int keys_per_proc = KEY_COUNT / size;
    int start = rank * keys_per_proc;
    int end = start + keys_per_proc;

    unsigned char hash[MD5_DIGEST_LENGTH];
    char key[MAX_KEY_LENGTH + 1];
    char *hash_table[KEY_COUNT] = {0};

    printf("Process %d: Searching keys %d to %d\n", rank, start, end);

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        int found_collision = 0;

        for (int i = start; i < end; i++) {
            generate_random_key(key, MAX_KEY_LENGTH - 1);
            compute_md5(key, hash);

            char hash_str[MD5_DIGEST_LENGTH * 2 + 1];
            for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                sprintf(hash_str + j * 2, "%02x", hash[j]);
            }

            if (hash_table[i % KEY_COUNT] != NULL &&
                strcmp(hash_table[i % KEY_COUNT], hash_str) == 0) {
                printf("Process %d: Collision found for hash %s (key: %s)\n", rank, hash_str, key);
                found_collision = 1;
                break;
            } else {
                hash_table[i % KEY_COUNT] = strdup(hash_str);
            }
        }

        if (found_collision) {
            break;
        } else {
            printf("Process %d: No collision found, retrying (%d/%d)\n", rank, attempt + 1, MAX_RETRIES);
        }
    }

    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;

    double total_time;
    MPI_Reduce(&elapsed_time, &total_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total execution time: %f seconds\n", total_time);
    }

    MPI_Finalize();
    return 0;
}
