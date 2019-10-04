#define MAX_CLIENT_NUM 4
#define MAX_CLIENT_NAME_SIZE 20

#define PERROR_AND_EXIT(message)\
            perror(message);\
                exit(1);
#define BUFF_SIZE 256

char *string_concat(char *line1, char *line2) {
    size_t line_len_1 = strlen(line1);
    size_t line_len_2 = strlen(line2);

    char *totalLine = calloc(line_len_1 + line_len_2 + 1, sizeof(char));
    if (!totalLine) abort();

    memcpy(totalLine, line1, line_len_1);
    memcpy(totalLine + line_len_1, line2, line_len_2);
    totalLine[line_len_1 + line_len_2] = '\0';

    return totalLine;

}
