int cp_main(int argc, char *argv[]) {
     if (argc != 3) {
        fprintf(stderr, "Usage: cp source dest\n");
        return 1;
    }

    FILE *source = fopen(argv[1], "rb");
    if (source == NULL) {
        perror("fopen (source)");
        return 1;
    }

    FILE *dest = fopen(argv[2], "wb");
    if (dest == NULL) {
        perror("fopen (destination)");
        fclose(source);
        return 1;
    }

    int ch;
    while ((ch = fgetc(source)) != EOF) {
        if (fputc(ch, dest) == EOF) {
            perror("fputc");
            fclose(source);
            fclose(dest);
            return 1;
        }
    }

    if (fclose(source) == EOF) {
        perror("fclose (source)");
        fclose(dest);
        return 1;
    }

    if (fclose(dest) == EOF) {
        perror("fclose (destination)");
        return 1;
    }

    return 0;

}
